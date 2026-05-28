# Tremor Orthosis — Python Dashboard

Three scripts cover the full workflow: **wizard → dashboard → analysis**.

---

## Requirements

```bash
pip install pyserial matplotlib pandas scipy numpy
```

---

## Scripts

### `wizard.py` — first-time setup
Run this once before anything else.  
Guides you through COM port selection, calibration, and threshold tuning.
After the wizard completes the device is ready to use.

```bash
python wizard.py
```

---

### `dashboard.py` — live session monitor
Real-time display of tremor amplitude, EMG, gyro and servo angle.
Records every session automatically to `CSV_Logs/session_YYYYMMDD_HHMMSS.csv`.

**Before first run:** open `dashboard.py` and set the correct COM port on line 33:
```python
PORT = "COM6"   # ← change to your port (e.g. COM3, COM8 …)
```

```bash
python dashboard.py
```

| Key | Action |
|-----|--------|
| `O` | Suppression ON |
| `F` | Suppression OFF |
| `P` | Ping device |
| `E` | Emergency stop |
| `Q` | Quit and save CSV |

---

### `analysis.py` — post-session analysis
Loads the most recent CSV log and generates:
- Tremor amplitude timeline with classifier overlay
- EMG and servo angle timeline
- Tremor distribution: STREAM vs SUPPRESS (box plot + histogram)
- Printed statistics: duration, packet rate, tremor reduction estimate

```bash
python analysis.py                        # auto-loads latest session
python analysis.py CSV_Logs/session_X.csv # specific session
python analysis.py --list                 # list all recorded sessions
```

---

## Folder layout

```
dashboard_py/
├── wizard.py          ← run first: setup & calibration
├── dashboard.py       ← live monitor + CSV recording
├── analysis.py        ← post-session analysis
└── CSV_Logs/          ← auto-created; one CSV per session
```
