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
## Potentiometer behaviour

The physical knob on the device controls suppression state and servo gain.
Turning it from left to right passes through three zones:

- **Left zone (0 – 45 % travel)**
  Suppression is OFF. The servo is detached and the arm moves completely freely.
  If suppression was active, turning into this zone releases it immediately.
  *Haptic: single soft bump.*

- **Middle zone / dead-band (45 – 55 % travel)**
  No state change. Useful as a neutral resting position — the system stays in
  whatever state it was already in, preventing accidental toggling.
  *Haptic: single brief tick when crossing into this zone.*

- **Right zone (55 – 100 % travel)**
  Suppression is ON. The servo engages whenever tremor is detected and holds
  the arm at its captured position. Turning further right increases the hold
  gain (more resistance against tremor). At the zone threshold (~55 %) the
  servo hold is at roughly half strength; at full right it is at maximum.
  During rest or voluntary movement the servo stays detached regardless of
  gain — it only engages on tremor onset.
  *Haptic: double medium buzz.*

- **After a tremor episode ends**
  Once tremor stops, the servo remains attached for a short delay (~400 ms)
  to avoid rapid cycling, then detaches automatically.
  *Haptic: triple light pulse confirming the episode is complete.*

---

## Folder layout

```
dashboard_py/
├── wizard.py          ← run first: setup & calibration
├── dashboard.py       ← live monitor + CSV recording
├── analysis.py        ← post-session analysis
└── CSV_Logs/          ← auto-created; one CSV per session
```
