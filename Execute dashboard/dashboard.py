"""
Tremor Orthosis — Live Dashboard
=================================
Requirements (install once):
    pip install pyserial matplotlib

Run:
    python dashboard.py

Keys:
    Q  = quit
    O  = send C,SUPPRESS_ON
    F  = send C,SUPPRESS_OFF
    P  = send C,PING
    E  = send C,ESTOP
"""

import sys
import threading
import collections
import csv
import os
import time
import datetime
import serial
import serial.tools.list_ports
import matplotlib
matplotlib.use("TkAgg")          # explicit backend — avoids Qt/WX conflicts
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

# ── Config ─────────────────────────────────────────────────────────
PORT      = "COM6"
BAUD      = 115200
HISTORY   = 300          # samples shown (3 s at 100 Hz)
INTERVAL  = 100          # plot refresh ms (~10 fps) — stable on Windows

CLASS_NAMES  = {0: "REST", 1: "VOLUNTARY", 2: "TREMOR"}
CLASS_COLORS = {0: "#999999", 1: "#64ff8c", 2: "#ff5050"}
STATE_NAMES  = {0:"BOOT",1:"SELFTEST",2:"IDLE",3:"CALIB",
                4:"STREAM",5:"SUPPRESS",6:"FAULT"}

# ── Shared data (written by serial thread, read by plot thread) ────
lock      = threading.Lock()
buf_tremor = collections.deque([0.0] * HISTORY, maxlen=HISTORY)
buf_emg    = collections.deque([0.0] * HISTORY, maxlen=HISTORY)
buf_fa_gx  = collections.deque([0.0] * HISTORY, maxlen=HISTORY)
buf_fa_gy  = collections.deque([0.0] * HISTORY, maxlen=HISTORY)
buf_fa_gz  = collections.deque([0.0] * HISTORY, maxlen=HISTORY)
buf_servo  = collections.deque([90.0]* HISTORY, maxlen=HISTORY)

latest = {
    "t_ms": 0, "pot": 0, "emg": 0, "servo": 0.0,
    "tremor": 0.0, "state": 0, "classifier": 0,
    "fa_gx": 0, "fa_gy": 0, "fa_gz": 0,
    "pkt_ok": 0, "pkt_err": 0,
    "events": collections.deque(maxlen=6),
}

ser = None   # serial port handle (set in connect())

# ── CSV recording ──────────────────────────────────────────────────
CSV_DIR       = os.path.join(os.path.dirname(__file__), "CSV_Logs")
_csv_file     = None
_csv_writer   = None
_session_start = None   # wall-clock time.time() at first connect

CSV_HEADER = [
    "wall_time_s",
    "t_ms", "pot", "emg", "servo_deg",
    "fa_ax", "fa_ay", "fa_az", "fa_gx", "fa_gy", "fa_gz",
    "ua_ax", "ua_ay", "ua_az", "ua_gx", "ua_gy", "ua_gz",
    "tremor_amp", "state", "classifier",
]

def _open_csv():
    """Create a new timestamped CSV file and write the header."""
    global _csv_file, _csv_writer, _session_start
    os.makedirs(CSV_DIR, exist_ok=True)
    stamp   = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    path    = os.path.join(CSV_DIR, f"session_{stamp}.csv")
    _csv_file   = open(path, "w", newline="", buffering=1)   # line-buffered
    _csv_writer = csv.writer(_csv_file)
    _csv_writer.writerow(CSV_HEADER)
    _session_start = time.time()
    print(f"[CSV] Recording to {path}")

def _close_csv():
    global _csv_file, _csv_writer
    if _csv_file:
        _csv_file.flush()
        _csv_file.close()
        _csv_file = _csv_writer = None
        print("[CSV] Session saved.")

# ── Serial thread ──────────────────────────────────────────────────
def parse_line(raw):
    raw = raw.strip()
    if not raw or raw.startswith("#"):
        return
    if raw[0] == "T":
        parse_telemetry(raw)
    elif raw[0] == "E":
        with lock:
            latest["events"].append(raw)

def parse_telemetry(line):
    f = line.split(",")
    if len(f) < 19:
        with lock:
            latest["pkt_err"] += 1
        return
    try:
        t_ms  = int(f[1]);   pot   = int(f[2]);   emg   = int(f[3])
        servo = float(f[4])
        fa_ax = int(f[5]);   fa_ay = int(f[6]);   fa_az = int(f[7])
        fa_gx = int(f[8]);   fa_gy = int(f[9]);   fa_gz = int(f[10])
        ua_ax = int(f[11]);  ua_ay = int(f[12]);  ua_az = int(f[13])
        ua_gx = int(f[14]);  ua_gy = int(f[15]);  ua_gz = int(f[16])
        tremor     = float(f[17])
        state      = int(f[18])
        classifier = int(f[19]) if len(f) > 19 else 0

        with lock:
            latest["t_ms"]       = t_ms
            latest["pot"]        = pot
            latest["emg"]        = emg
            latest["servo"]      = servo
            latest["fa_gx"]      = fa_gx
            latest["fa_gy"]      = fa_gy
            latest["fa_gz"]      = fa_gz
            latest["tremor"]     = tremor
            latest["state"]      = state
            latest["classifier"] = classifier
            latest["pkt_ok"]    += 1
            buf_tremor.append(tremor)
            buf_emg.append(emg)
            buf_fa_gx.append(fa_gx * 0.1)
            buf_fa_gy.append(fa_gy * 0.1)
            buf_fa_gz.append(fa_gz * 0.1)
            buf_servo.append(servo)
            if latest["pkt_ok"] % 100 == 0:
                print(f"[pkt {latest['pkt_ok']:6d}]  tremor={tremor:.2f}  "
                      f"emg={emg}  state={state}  cls={classifier}")

        # ── CSV row (written outside the lock — no shared state touched) ──
        if _csv_writer is not None:
            wall = round(time.time() - _session_start, 3)
            _csv_writer.writerow([
                wall,
                t_ms, pot, emg, servo,
                fa_ax, fa_ay, fa_az, fa_gx, fa_gy, fa_gz,
                ua_ax, ua_ay, ua_az, ua_gx, ua_gy, ua_gz,
                tremor, state, classifier,
            ])

    except Exception:
        with lock:
            latest["pkt_err"] += 1

def serial_thread():
    global ser
    while True:
        try:
            line = ser.readline().decode("ascii", errors="replace")
            parse_line(line)
        except Exception:
            break

def connect():
    global ser
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Opened {PORT} at {BAUD} baud.")
        _open_csv()
        t = threading.Thread(target=serial_thread, daemon=True)
        t.start()
        return True
    except serial.SerialException as e:
        print(f"Could not open {PORT}: {e}")
        print("Available ports:")
        for p in serial.tools.list_ports.comports():
            print(f"  {p.device} — {p.description}")
        return False

def send_cmd(cmd):
    if ser and ser.is_open:
        ser.write((cmd + "\n").encode())
        print(f"Sent: {cmd}")

# ── Plot setup ─────────────────────────────────────────────────────
fig = plt.figure(figsize=(13, 8), facecolor="#111")
fig.canvas.manager.set_window_title("Tremor Orthosis Dashboard")
gs  = GridSpec(4, 2, figure=fig, hspace=0.55, wspace=0.35,
               left=0.07, right=0.97, top=0.93, bottom=0.07)

ax_tremor = fig.add_subplot(gs[0, :])   # full width
ax_emg    = fig.add_subplot(gs[1, :])
ax_gyro   = fig.add_subplot(gs[2, :])
ax_servo  = fig.add_subplot(gs[3, 0])
ax_info   = fig.add_subplot(gs[3, 1])

for ax in [ax_tremor, ax_emg, ax_gyro, ax_servo]:
    ax.set_facecolor("#1a1a1a")
    ax.tick_params(colors="#888", labelsize=8)
    for spine in ax.spines.values():
        spine.set_edgecolor("#333")

ax_info.set_facecolor("#1a1a1a")
ax_info.axis("off")

x = list(range(HISTORY))   # x-axis indices, matches deque maxlen

# Tremor
ln_tremor, = ax_tremor.plot(x, list(buf_tremor), color="#ffb450", lw=1.2)
ax_tremor.set_ylim(0, 80)
ax_tremor.set_ylabel("deg/s", color="#888", fontsize=8)
ax_tremor.set_title("Tremor Amplitude  (bandpass 2–8 Hz)", color="#ffb450",
                    fontsize=9, loc="left")
ax_tremor.axhline(10, color="#ff5050", lw=0.7, ls="--", label="threshold")

# EMG
ln_emg, = ax_emg.plot(x, list(buf_emg), color="#64ff8c", lw=1.0)
ax_emg.set_ylim(0, 1023)
ax_emg.set_ylabel("ADC", color="#888", fontsize=8)
ax_emg.set_title("EMG Envelope", color="#64ff8c", fontsize=9, loc="left")

# Gyro
ln_gx, = ax_gyro.plot(x, list(buf_fa_gx), color="#6ab4ff", lw=0.9, label="Gx")
ln_gy, = ax_gyro.plot(x, list(buf_fa_gy), color="#ff82c8", lw=0.9, label="Gy")
ln_gz, = ax_gyro.plot(x, list(buf_fa_gz), color="#c8ff82", lw=0.9, label="Gz")
ax_gyro.set_ylabel("deg/s", color="#888", fontsize=8)
ax_gyro.set_title("Forearm Gyro  (Gx/Gy/Gz)", color="#aaa", fontsize=9, loc="left")
ax_gyro.legend(loc="upper right", fontsize=7, facecolor="#222", labelcolor="#ccc",
               framealpha=0.6)

# Servo
ln_servo, = ax_servo.plot(x, list(buf_servo), color="#c8a0ff", lw=1.0)
ax_servo.set_ylim(0, 180)
ax_servo.set_ylabel("deg", color="#888", fontsize=8)
ax_servo.set_title("Servo Angle", color="#c8a0ff", fontsize=9, loc="left")

# Info panel text objects
ax_info.set_xlim(0, 1); ax_info.set_ylim(0, 1)
def mktxt(x, y, color="#ccc", size=9):
    return ax_info.text(x, y, "", color=color, fontsize=size,
                        transform=ax_info.transAxes, va="top",
                        fontfamily="monospace")

txt_state      = mktxt(0.0, 1.00, size=11)
txt_classifier = mktxt(0.0, 0.82, size=10)
txt_uptime     = mktxt(0.0, 0.65, "#888", size=8)
txt_pot        = mktxt(0.0, 0.55, "#888", size=8)
txt_pkts       = mktxt(0.0, 0.45, "#888", size=8)
txt_events     = mktxt(0.0, 0.32, "#6fa", size=7)
txt_keys       = mktxt(0.0, 0.10, "#555", size=7)
txt_keys.set_text("Keys: O=SuppressON  F=SuppressOFF  P=Ping  E=ESTOP  Q=Quit")

# ── Animation update ───────────────────────────────────────────────
def update(_frame):
    with lock:
        tr   = list(buf_tremor)
        em   = list(buf_emg)
        gx   = list(buf_fa_gx)
        gy   = list(buf_fa_gy)
        gz   = list(buf_fa_gz)
        sv   = list(buf_servo)
        snap = dict(latest)
        evts = list(latest["events"])

    ln_tremor.set_ydata(tr)
    ln_emg.set_ydata(em)
    ln_gx.set_ydata(gx)
    ln_gy.set_ydata(gy)
    ln_gz.set_ydata(gz)
    ln_servo.set_ydata(sv)

    # Auto-scale gyro
    all_g = gx + gy + gz
    mn, mx = min(all_g), max(all_g)
    pad = max(10, (mx - mn) * 0.1)
    ax_gyro.set_ylim(mn - pad, mx + pad)

    state_name = STATE_NAMES.get(snap["state"], "?")
    cl         = snap["classifier"]
    cl_name    = CLASS_NAMES.get(cl, "?")
    cl_col     = CLASS_COLORS.get(cl, "#ccc")

    txt_state.set_text(f"STATE: {state_name}")
    txt_state.set_color("#64ff8c" if snap["state"] == 5 else
                        "#ff5050" if snap["state"] == 6 else "#ccc")
    txt_classifier.set_text(f"● {cl_name}")
    txt_classifier.set_color(cl_col)
    txt_uptime.set_text(f"Uptime: {snap['t_ms']/1000:.1f}s   Pot: {snap['pot']}")
    txt_pot.set_text(f"EMG: {snap['emg']}   Servo: {snap['servo']:.1f}°")
    txt_pkts.set_text(f"Pkts OK: {snap['pkt_ok']}   ERR: {snap['pkt_err']}")

    ev_str = "\n".join(evts[-4:]) if evts else "(no events)"
    txt_events.set_text(f"Events:\n{ev_str}")


# ── Keyboard handler ───────────────────────────────────────────────
def on_key(event):
    k = event.key.lower() if event.key else ""
    if k == "q":
        _close_csv()
        plt.close("all")
        sys.exit(0)
    elif k == "o":
        send_cmd("C,SUPPRESS_ON")
    elif k == "f":
        send_cmd("C,SUPPRESS_OFF")
    elif k == "p":
        send_cmd("C,PING")
    elif k == "e":
        send_cmd("C,ESTOP")

fig.canvas.mpl_connect("key_press_event", on_key)

# ── Manual Tk timer loop ────────────────────────────────────────────
# FuncAnimation + blit=False on Windows TkAgg never flushes the paint
# event to the screen.  Driving the canvas directly with after() +
# draw() + flush_events() is reliable across all matplotlib/Tk versions.
def _tick():
    try:
        update(None)
        fig.canvas.draw()
        fig.canvas.flush_events()
    except Exception as e:
        print(f"[dashboard] render error: {e}", file=sys.stderr)
        return  # don't reschedule — window may be closing
    try:
        fig.canvas.manager.window.after(INTERVAL, _tick)
    except Exception:
        pass  # window is being destroyed

# ── Main ────────────────────────────────────────────────────────────
if __name__ == "__main__":
    if not connect():
        print("\nEdit PORT at the top of dashboard.py and try again.")
        sys.exit(1)

    # Schedule first tick, then start the Tk mainloop once.
    # plt.figure() already created the Tk window; the mainloop hasn't started yet.
    fig.canvas.manager.window.after(INTERVAL, _tick)
    plt.show()  # blocks here; Tk event loop fires _tick every INTERVAL ms
