"""
analysis.py  —  Tremor Orthosis Session Analysis
=================================================
Loads a CSV produced by dashboard.py and generates:
  1. Tremor amplitude timeline with suppression state overlay
  2. EMG and servo angle timeline
  3. Tremor amplitude distribution: STREAM vs SUPPRESS
  4. Classifier breakdown pie chart
  5. Printed summary statistics

Requirements:
    pip install pandas matplotlib scipy

Usage:
    python analysis.py                        # auto-loads latest CSV in CSV_Logs/
    python analysis.py CSV_Logs/session_X.csv # specific file
    python analysis.py --list                 # list available sessions
"""

import sys
import os
import glob
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

# ── Column names (must match CSV_HEADER in dashboard.py) ──────────────
COLS = [
    "wall_time_s",
    "t_ms", "pot", "emg", "servo_deg",
    "fa_ax", "fa_ay", "fa_az", "fa_gx", "fa_gy", "fa_gz",
    "ua_ax", "ua_ay", "ua_az", "ua_gx", "ua_gy", "ua_gz",
    "tremor_amp", "state", "classifier",
]

# ── Metadata maps ──────────────────────────────────────────────────────
STATE_NAMES = {0:"BOOT", 1:"SELFTEST", 2:"IDLE", 3:"CALIB",
               4:"STREAM", 5:"SUPPRESS", 6:"FAULT"}
CLASS_NAMES = {0:"REST", 1:"VOLUNTARY", 2:"TREMOR"}
CLASS_COLORS = {0:"#999999", 1:"#64c8ff", 2:"#ff5050"}


# ══════════════════════════════════════════════════════════════════════
def load(path: str) -> pd.DataFrame:
    df = pd.read_csv(path, header=0, names=COLS)
    # Convert protocol units to real units.
    for ax in ("fa_ax","fa_ay","fa_az","ua_ax","ua_ay","ua_az"):
        df[ax] = df[ax] * 0.001          # → g
    for gx in ("fa_gx","fa_gy","fa_gz","ua_gx","ua_gy","ua_gz"):
        df[gx] = df[gx] * 0.1            # → deg/s
    df["fa_gyro_l1"] = df[["fa_gx","fa_gy","fa_gz"]].abs().sum(axis=1)
    return df


def latest_csv() -> str:
    csv_dir = os.path.join(os.path.dirname(__file__), "CSV_Logs")
    files   = sorted(glob.glob(os.path.join(csv_dir, "session_*.csv")))
    if not files:
        raise FileNotFoundError(f"No session CSVs found in {csv_dir}")
    return files[-1]


# ══════════════════════════════════════════════════════════════════════
def print_summary(df: pd.DataFrame, path: str):
    duration = df["wall_time_s"].iloc[-1] - df["wall_time_s"].iloc[0]
    n        = len(df)
    rate     = n / duration if duration > 0 else 0

    print(f"\n{'='*60}")
    print(f"  Session:  {os.path.basename(path)}")
    print(f"  Duration: {duration:.1f} s  ({duration/60:.1f} min)")
    print(f"  Packets:  {n}  ({rate:.1f} Hz)")
    print(f"{'='*60}")

    # State time breakdown
    print("\n── State time breakdown ──────────────────────────────────")
    for sid, sname in STATE_NAMES.items():
        mask = df["state"] == sid
        secs = mask.sum() / rate if rate > 0 else 0
        pct  = 100 * mask.mean()
        if pct > 0.5:
            print(f"  {sname:<10}  {secs:6.1f} s  ({pct:.1f} %)")

    # Classifier breakdown
    print("\n── Classifier breakdown ──────────────────────────────────")
    for cid, cname in CLASS_NAMES.items():
        pct = 100 * (df["classifier"] == cid).mean()
        print(f"  {cname:<10}  {pct:.1f} %")

    # Tremor amplitude statistics
    print("\n── Tremor amplitude (deg/s) ──────────────────────────────")
    for sid, sname in [(4, "STREAM"), (5, "SUPPRESS")]:
        sub = df[df["state"] == sid]["tremor_amp"]
        if len(sub) < 10:
            continue
        print(f"  {sname:<10}  mean={sub.mean():.2f}  "
              f"std={sub.std():.2f}  max={sub.max():.2f}  "
              f"n={len(sub)}")

    # Efficacy estimate
    stream_mean   = df[df["state"] == 4]["tremor_amp"].mean()
    suppress_mean = df[df["state"] == 5]["tremor_amp"].mean()
    if pd.notna(stream_mean) and pd.notna(suppress_mean) and stream_mean > 0:
        reduction = 100 * (stream_mean - suppress_mean) / stream_mean
        print(f"\n  Tremor reduction estimate: "
              f"{reduction:+.1f} % (STREAM→SUPPRESS mean)")
        print("  Note: this comparison is only valid if tremor was present\n"
              "  in both states during the session.")
    print(f"{'='*60}\n")


# ══════════════════════════════════════════════════════════════════════
def plot(df: pd.DataFrame, path: str):
    t = df["wall_time_s"].values

    fig, axes = plt.subplots(
        4, 1,
        figsize=(14, 9),
        gridspec_kw={"height_ratios": [3, 2, 2, 1]},
        sharex=True,
        facecolor="#111",
    )
    fig.suptitle(f"Session: {os.path.basename(path)}",
                 color="#ccc", fontsize=11)

    for ax in axes:
        ax.set_facecolor("#1a1a1a")
        ax.tick_params(colors="#888", labelsize=8)
        for sp in ax.spines.values(): sp.set_edgecolor("#333")

    # ── Shade background by state ──────────────────────────────────
    state_colors = {4: "#003060", 5: "#003020", 6: "#400000"}
    for ax in axes:
        _shade_state(ax, df, t, state_colors)

    # ── 1. Tremor amplitude ────────────────────────────────────────
    ax = axes[0]
    ax.plot(t, df["tremor_amp"], color="#ffb450", lw=0.8, alpha=0.9)
    _add_threshold_line(ax, df)
    ax.set_ylabel("deg/s", color="#888", fontsize=8)
    ax.set_title("Tremor Amplitude  (bandpass 2–8 Hz)", color="#ffb450",
                 fontsize=9, loc="left")

    # Colour scatter by classifier on top.
    for cid, ccol in CLASS_COLORS.items():
        mask = df["classifier"] == cid
        ax.scatter(t[mask], df["tremor_amp"].values[mask],
                   c=ccol, s=2, alpha=0.5, zorder=3)

    # Legend patches.
    patches  = [mpatches.Patch(color=c, label=CLASS_NAMES[i])
                for i, c in CLASS_COLORS.items()]
    patches += [mpatches.Patch(color="#003060", label="STREAM"),
                mpatches.Patch(color="#003020", label="SUPPRESS")]
    ax.legend(handles=patches, loc="upper right", fontsize=7,
              facecolor="#222", labelcolor="#ccc", framealpha=0.7,
              ncol=2)

    # ── 2. EMG ────────────────────────────────────────────────────
    ax = axes[1]
    ax.plot(t, df["emg"], color="#64ff8c", lw=0.7, alpha=0.8)
    ax.set_ylabel("ADC", color="#888", fontsize=8)
    ax.set_title("EMG Envelope", color="#64ff8c", fontsize=9, loc="left")
    ax.set_ylim(0, 1023)

    # ── 3. Forearm gyro L1 ────────────────────────────────────────
    ax = axes[2]
    ax.plot(t, df["fa_gyro_l1"], color="#6ab4ff", lw=0.7, alpha=0.8)
    ax.set_ylabel("deg/s", color="#888", fontsize=8)
    ax.set_title("Forearm Gyro  |Gx|+|Gy|+|Gz|", color="#6ab4ff",
                 fontsize=9, loc="left")

    # ── 4. Servo angle ────────────────────────────────────────────
    ax = axes[3]
    ax.plot(t, df["servo_deg"], color="#c8a0ff", lw=0.8, alpha=0.9)
    ax.set_ylabel("deg", color="#888", fontsize=8)
    ax.set_title("Servo Angle", color="#c8a0ff", fontsize=9, loc="left")
    ax.set_ylim(0, 180)
    ax.set_xlabel("Time (s)", color="#888", fontsize=8)

    plt.tight_layout()

    # ── Second figure: distribution comparison ─────────────────────
    fig2, axes2 = plt.subplots(1, 2, figsize=(10, 4), facecolor="#111")
    fig2.suptitle("Tremor Distribution  —  STREAM vs SUPPRESS",
                  color="#ccc", fontsize=10)

    for ax2 in axes2:
        ax2.set_facecolor("#1a1a1a")
        ax2.tick_params(colors="#888", labelsize=8)
        for sp in ax2.spines.values(): sp.set_edgecolor("#333")

    # Box plot
    ax2 = axes2[0]
    data_s  = df[df["state"] == 4]["tremor_amp"].dropna().values
    data_su = df[df["state"] == 5]["tremor_amp"].dropna().values
    if len(data_s) > 0 and len(data_su) > 0:
        bp = ax2.boxplot([data_s, data_su], patch_artist=True,
                         labels=["STREAM", "SUPPRESS"],
                         medianprops=dict(color="white", lw=1.5))
        bp["boxes"][0].set_facecolor("#003060")
        bp["boxes"][1].set_facecolor("#003020")
        for whisker in bp["whiskers"]: whisker.set_color("#888")
        for cap in bp["caps"]:         cap.set_color("#888")
    ax2.set_ylabel("Tremor amplitude (deg/s)", color="#888", fontsize=8)
    ax2.set_title("Box plot by state", color="#ccc", fontsize=9)

    # Histogram overlay
    ax2 = axes2[1]
    bins = np.linspace(0, max(df["tremor_amp"].max(), 1), 40)
    if len(data_s)  > 0: ax2.hist(data_s,  bins=bins, alpha=0.55,
                                   color="#6ab4ff", label="STREAM",   density=True)
    if len(data_su) > 0: ax2.hist(data_su, bins=bins, alpha=0.55,
                                   color="#64ff8c", label="SUPPRESS", density=True)
    ax2.set_xlabel("Tremor amplitude (deg/s)", color="#888", fontsize=8)
    ax2.set_ylabel("Density", color="#888", fontsize=8)
    ax2.set_title("Distribution", color="#ccc", fontsize=9)
    ax2.legend(fontsize=8, facecolor="#222", labelcolor="#ccc")

    plt.tight_layout()
    plt.show()


# ── Helpers ────────────────────────────────────────────────────────────

def _shade_state(ax, df, t, state_colors):
    """Shade background in bands wherever state is constant."""
    states = df["state"].values
    n = len(states)
    i = 0
    while i < n:
        sid = states[i]
        col = state_colors.get(sid)
        if col is None:
            i += 1
            continue
        j = i + 1
        while j < n and states[j] == sid:
            j += 1
        ax.axvspan(t[i], t[j-1], color=col, alpha=0.35, linewidth=0)
        i = j


def _add_threshold_line(ax, df):
    """Draw threshold as a dashed horizontal line (read from data if possible)."""
    # The threshold isn't stored in the CSV, but we can infer it from
    # when the classifier first switches to TREMOR.
    # Simpler: just draw the default 20 deg/s line.
    ax.axhline(20, color="#ff5050", lw=0.8, ls="--", alpha=0.6,
               label="default threshold")


# ══════════════════════════════════════════════════════════════════════
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Tremor session analysis")
    parser.add_argument("csv", nargs="?", help="Path to session CSV (default: latest)")
    parser.add_argument("--list", action="store_true",
                        help="List available session files and exit")
    args = parser.parse_args()

    csv_dir = os.path.join(os.path.dirname(__file__), "CSV_Logs")

    if args.list:
        files = sorted(glob.glob(os.path.join(csv_dir, "session_*.csv")))
        if not files:
            print("No session files found in", csv_dir)
        else:
            print(f"Sessions in {csv_dir}:")
            for f in files:
                size_kb = os.path.getsize(f) // 1024
                print(f"  {os.path.basename(f)}  ({size_kb} KB)")
        sys.exit(0)

    path = args.csv if args.csv else latest_csv()
    print(f"Loading {path} …")

    df = load(path)
    if df.empty:
        print("CSV is empty — run a session first.")
        sys.exit(1)

    print_summary(df, path)
    plot(df, path)
