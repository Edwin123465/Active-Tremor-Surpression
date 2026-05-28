"""
wizard.py  –  Guided startup for the Tremor Orthosis
=====================================================
Run this INSTEAD of dashboard.py to walk through calibration,
EMG setup, and threshold tuning before a session.

Usage:
    python wizard.py

It will optionally launch dashboard.py at the end.
"""

import sys
import time
import threading
import queue
import subprocess
import os
import serial
import serial.tools.list_ports

PORT = "COM6"
BAUD = 115200

# ── Serial setup ──────────────────────────────────────────────────
event_queue = queue.Queue()
ser = None

def _serial_thread():
    while True:
        try:
            line = ser.readline().decode("ascii", errors="replace").strip()
            if line:
                event_queue.put(line)
        except Exception:
            break

def connect():
    global ser
    print(f"  Connecting to {PORT} at {BAUD} baud ...", end=" ", flush=True)
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print("OK")
        threading.Thread(target=_serial_thread, daemon=True).start()
        return True
    except serial.SerialException as e:
        print(f"FAILED\n  {e}")
        print("\n  Available ports:")
        for p in serial.tools.list_ports.comports():
            print(f"    {p.device}  –  {p.description}")
        print(f"\n  Edit PORT at the top of wizard.py and try again.")
        return False

def send(cmd):
    ser.write((cmd + "\n").encode())

def drain(seconds=0.5):
    """Silently consume buffered serial lines for a short period."""
    deadline = time.time() + seconds
    while time.time() < deadline:
        try:
            event_queue.get(timeout=0.05)
        except queue.Empty:
            pass

def wait_for(keyword, timeout=15, show_lines=False):
    """Block until a line containing `keyword` arrives, or timeout."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            line = event_queue.get(timeout=0.2)
            if show_lines and not line.startswith("T,"):
                print(f"    {line}")
            if keyword in line:
                return True
        except queue.Empty:
            pass
    return False

def yn(prompt):
    while True:
        r = input(f"  {prompt} [y/n]: ").strip().lower()
        if r in ("y", "n", "yes", "no"):
            return r in ("y", "yes")
        print("  Please type y or n.")

def ask(prompt, default):
    val = input(f"  {prompt} [{default}]: ").strip()
    return val if val else str(default)

# ── Wizard steps ──────────────────────────────────────────────────
def step_connect():
    print("\n" + "━"*54)
    print("  TREMOR ORTHOSIS  –  Startup Wizard")
    print("━"*54)
    print()
    if not connect():
        sys.exit(1)
    print("  Waiting for Arduino to boot", end="", flush=True)
    for _ in range(30):
        try:
            line = event_queue.get(timeout=0.5)
            if not line.startswith("T,"):
                print(".", end="", flush=True)
            if "ready" in line:
                print(" ready!\n")
                drain(0.3)
                return
        except queue.Empty:
            print(".", end="", flush=True)
    print("\n  [!] Timed out — is the Arduino plugged in and flashed?")
    sys.exit(1)

def step_emg():
    print("─── Step 1 / EMG sensor " + "─"*30)
    print("""
  The MyoWare EMG sensor detects voluntary muscle contractions
  so the system does NOT suppress intentional movements.
  If you are not wearing it, disable EMG gating — the classifier
  will then use the gyro signal only (REST vs TREMOR).
""")
    fitted = yn("Is the EMG sensor fitted and placed on skin?")
    if fitted:
        send("C,EMG_ON")
        print("  EMG gating ON  (C,EMG_OFF anytime to disable)\n")
    else:
        send("C,EMG_OFF")
        print("  EMG gating OFF (C,EMG_ON anytime to re-enable)\n")
    drain(0.3)

def step_calibrate():
    print("─── Step 2 / Calibration " + "─"*29)
    print("""
  Calibration measures gyro offsets for both IMUs so tremor
  detection is accurate.  Run once per session (values are
  saved to EEPROM and reloaded at next boot).

  You will need to:
    a) Hold the arm COMPLETELY STILL for 3 seconds
    b) Then move the forearm through its FULL range (flex/extend)
""")
    if not yn("Run calibration now?"):
        print("  Skipping — using stored EEPROM values (or defaults).\n")
        return

    print("  Starting calibration ...")
    send("C,CALIB_START")

    if not wait_for("calib_still", timeout=6, show_lines=True):
        print("  [!] Did not receive calibration start. Aborting.\n")
        send("C,CALIB_ABORT")
        return

    print()
    print("  ★  HOLD STILL — do not move the arm ...")
    for i in range(3, 0, -1):
        print(f"     {i} ...", flush=True)
        time.sleep(1)

    print("  Waiting for still phase to finish ...", end="", flush=True)
    if not wait_for("calib_rom", timeout=10):
        print(" timed out — aborting.\n")
        send("C,CALIB_ABORT")
        return
    print(" done!")

    print()
    print("  ★  Now MOVE YOUR FOREARM through its full range:")
    print("     Flex and extend the elbow/wrist several times slowly.")
    input("\n  Press Enter when done: ")

    send("C,CALIB_SAVE")
    drain(1.0)
    print("  Calibration saved to EEPROM.\n")

def step_threshold():
    print("─── Step 3 / Tremor threshold " + "─"*24)
    print("""
  The threshold is the minimum filtered gyro amplitude (deg/s)
  that counts as tremor.

    Higher value → less sensitive → fewer false triggers
    Lower  value → more sensitive → catches weaker tremor

  Current default: 25.0 deg/s
  Tip: watch the dashboard "Tremor Amplitude" plot while moving
  slowly — keep the threshold above the noise floor.
""")
    raw = ask("Threshold in deg/s (Enter to keep 25.0)", "25.0")
    try:
        t = float(raw)
        t = max(1.0, min(200.0, t))
        send(f"C,SET_THRESH,{t:.1f}")
        drain(0.3)
        print(f"  Threshold set to {t:.1f} deg/s\n")
    except ValueError:
        print("  Invalid — keeping current threshold.\n")

def step_summary():
    print("─── Ready! " + "─"*43)
    print("""
  The orthosis is in STREAM mode — sensing and sending telemetry.
  Suppression is OFF.  Use the potentiometer to control it:

  ┌──────────────────────────────────────────────────────┐
  │  Turn pot to the RIGHT THIRD  (>80 %)                │
  │       → Suppression ON  (servo engages)              │
  │                                                      │
  │  Leave pot in the MIDDLE (20–80 %)                   │
  │       → Suppression stays ON; dial sets strength     │
  │                                                      │
  │  Turn pot to the LEFT THIRD   (<20 %)                │
  │       → Suppression OFF (servo releases)             │
  └──────────────────────────────────────────────────────┘

  Serial commands (via Serial Monitor at 115200 baud, Newline):
    C,SUPPRESS_ON / C,SUPPRESS_OFF  — software on/off
    C,ESTOP                         — emergency stop → FAULT
    C,RESET                         — clear FAULT
    C,EMG_OFF / C,EMG_ON            — toggle EMG gating
    C,SET_THRESH,<deg_s>            — adjust tremor sensitivity
    C,PING                          — check connection (replies E,INFO,pong)
""")

def step_launch():
    if not yn("Launch dashboard now?"):
        ser.close()
        print("  Done.  Run dashboard.py when ready.\n")
        return
    ser.close()
    dashboard = os.path.join(os.path.dirname(os.path.abspath(__file__)), "dashboard.py")
    print(f"  Launching dashboard ...\n")
    subprocess.Popen([sys.executable, dashboard])

# ── Main ──────────────────────────────────────────────────────────
if __name__ == "__main__":
    step_connect()
    step_emg()
    step_calibrate()
    step_threshold()
    step_summary()
    step_launch()
