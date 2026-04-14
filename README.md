# Active-Tremor-Surpression
Description comes later 
# Active Tremor Suppression Orthosis  

**Authors:** Samu Boving & Edwin Arnalsteen  
**Course:** Haptic Interfaces Experience – KU Leuven  
**Professor:** Carlos Rodriguez-Guerrero  

---

## 🩺 Introduction  

Pathological tremor, including Parkinson’s disease tremor (4–6 Hz), essential tremor (6–12 Hz), and cerebellar tremor (2–4 Hz), affects millions of people worldwide and significantly impairs daily activities such as eating, writing, and self-care.  

Current treatments are often insufficient. Pharmacological approaches provide limited relief, while existing wearable devices either rely on passive damping or electrical stimulation, which may cause discomfort and muscle fatigue over time.  

This project proposes an **active, wearable tremor suppression orthosis** that uses mechanical counter-force instead of electrical stimulation. By combining real-time sensing, control, and haptic feedback, the system aims to provide a more adaptive and user-friendly solution.  

---

## 🧰 Supplies (Bill of Materials)  

| Component | Purpose | Qty |
|----------|--------|-----|
| EduExo frame | Wearable structure | 1 |
| Stepper motor | Actuation | 1 |
| IMU (MPU6050) | Motion sensing | 2 |
| Arduino Micro | Control & processing | 1 |
| Haptic actuator | User feedback | 1 |
| Potentiometer | Sensitivity tuning | 1 |
| Power supply | System power | 1 |
| Wiring & connectors | Integration | - |

---

## ⚙️ Methods  

### Step 1: Sensing  
Two IMUs are mounted on the forearm and hand to capture relative motion. These sensors provide 6-DOF data (acceleration + angular velocity), which is transmitted to the Arduino via I2C.

### Step 2: Signal Processing  
The Arduino performs real-time filtering and frequency analysis to distinguish pathological tremor from voluntary movement. The dominant tremor frequency is extracted.

### Step 3: Control Strategy  
Based on the detected tremor, a control algorithm determines the required counteracting force. The system operates in a closed-loop configuration.

### Step 4: Actuation  
A stepper motor generates a mechanical counter-force that is applied to the wrist through the orthosis structure, reducing tremor amplitude.

### Step 5: User Interface  
- A potentiometer allows real-time tuning of system sensitivity  
- A haptic actuator provides tactile feedback on suppression level  

### Step 6: System Integration  
All components are integrated into a compact wearable setup. Power management, wiring, and mechanical stability were key design considerations.

---

## 🧪 System Architecture  

**Sensing Layer:** Dual IMUs measure motion  
**Processing Layer:** Arduino performs filtering and control  
**Actuation Layer:** Stepper motor generates counter-force  
**Feedback Layer:** Haptic actuator + user control input  
**Monitoring:** Optional data streaming to PC  

*(Add a diagram here!)*  

---

## 📊 Results & Discussion  

The prototype demonstrates the feasibility of active tremor suppression using mechanical counter-force.  

Key observations:  
- Effective reduction of tremor amplitude at specific frequencies  
- Improved user comfort compared to electrical stimulation  
- Sensitivity tuning is essential for optimal performance  

Limitations:  
- Limited torque output of the stepper motor  
- Latency in signal processing  
- Mechanical alignment challenges  

---

## 🧾 Conclusion & Future Work  

This project presents a functional prototype of an active tremor suppression orthosis using real-time sensing and mechanical actuation.  

Future improvements include:  
- More advanced control algorithms (adaptive / predictive)  
- Higher-performance actuators  
- Miniaturization of electronics  
- Clinical validation with users  

---

## 📚 References  

[1] 
[2]  
[3] 

---

## 🎥 Demo Video  

*(Insert YouTube link here)*  

---

## 📂 Repository Structure  
