# Active Tremor Suppression  

**Authors:** Samu Boving & Edwin Arnalsteen  
**Course:** Haptic Interfaces Experience – KU Leuven  
**Professor:** Carlos Rodriguez-Guerrero  
## Description
This project describes the design of a tremor suppression arm using the EduExo Lite kit in combination with additional components. The project was carried out as part of a coursework assignment for the Haptic Interfaces course at KU Leuven.

---

## Introduction  
Tremor is characterised by an involuntary rhythmic oscillation occurring in a body part [1]. It is one of the most common movement disorders in adults and affects approximately 0.4% of the global population [2]. Consequently, tremor can have a significant negative impact on the performance of daily activities for many individuals. Therefore, a device capable of suppressing these oscillations is of clear medical relevance.
Tremor may arise from a wide range of underlying causes, although the exact mechanisms are not yet fully understood [1]. Different types of tremor can be classified according to several key features, including tremor frequency, the conditions under which the tremor occurs, and additional information obtained from the patient’s medical and family history. Most tremors occur within a frequency range of 0-12 Hz [3]. 

For further clarification, it is useful to distinguish between essential tremor, which is the primary focus of this work, and the more widely known tremor associated with Parkinson's disease. Although tremor can present itself in different forms, tremor in Parkinson’s disease is classically characterised as a resting tremor, meaning that it mainly occurs when the affected body part is relaxed and not actively engaged in movement. In contrast, essential tremor is typically classified as an action tremor, meaning that it occurs during voluntary movement or while maintaining a posture. Nevertheless, considerable overlap exists between different tremor types, making an absolute distinction difficult [4].

For this reason, the present tremor suppression application is aimed at tremor in general, regardless of its underlying cause, as a tool capable of reducing tremor may assist users in performing everyday tasks more effectively.
However, due to the limited hardware capabilities of the EduExo kit used in this project, severe tremors and tremors occurring at higher frequencies may be more difficult to suppress effectively.

### Existing solutions 
To provide further inspiration for the future implementation of tremor suppression solutions, this section discusses several existing approaches for tremor reduction.
Current solutions can generally be divided into four main categories. The first category consists of robotic exoskeletons, which are often implemented as conventional wearable robotic devices. A further subdivision within this category includes soft robotic exoskeletons. In addition, there are tremor suppression approaches based on Functional Electrical Stimulation (FES). Finally, some of the more recent developments focus on afferent neurostimulation techniques.

#### Robotic exoskeletons 
The fundamental principle of robotic exoskeletons is biomechanical loading. By applying controlled forces or additional inertia to a limb, tremorous motion can be attenuated. The application developed in this project using the Edu Exo kit also falls within this category. Robotic exoskeletons are often considered one of the most effective approaches for tremor reduction, as they can provide direct and active mechanical suppression of unwanted oscillations. However, a major drawback of these systems is that they are often relatively bulky, noisy, and uncomfortable to wear for extended periods. This limits their suitability for continuous use in everyday life [5].

One of the earliest applications in this category is WOTAS (Wearable Orthosis for Tremor Assessment and Suppression), a wearable upper-limb exoskeleton designed to measure and suppress tremor by applying mechanical forces to the arm. The system was developed as a non-grounded tremor suppression platform, meaning that no external fixed support structure is required. WOTAS follows the kinematic structure of the human arm and supports multiple degrees of freedom, including elbow flexion-extension, forearm pronation-supination and wrist flexion-extension [6].

The exoskeleton is equipped with several sensors, primarily MEMS gyroscopes, to measure tremor characteristics such as frequency and amplitude. In addition, force sensors based on strain gauges are integrated to monitor the interaction forces between the user and the orthosis. Actuation is achieved using compact brushless DC motors combined with harmonic drive transmissions [6].

To suppress tremor, WOTAS biomechanically modifies the motion of the arm through strategies such as impedance control and notch filtering. The control system first separates voluntary movement from tremorous motion and subsequently estimates tremor frequency and amplitude in real time. In this way, the device can selectively counteract tremor while preserving intentional movement as much as possible [6]. Figure 1 shows an overview of the WOTAS exoskeleton.

<img width="498" height="480" alt="image" src="https://github.com/user-attachments/assets/152af769-cf90-4a6a-853e-53510e5cb611" />

  <b>Figure 1:</b> An overview of the WOTOS exoskeleton [6]

<br><br>
#### Soft robotic exoskeletons
Soft actuators are compliant actuators composed of flexible materials such as elastomers or silicone, allowing improved conformity to the human body and increased wearing comfort. Similar to other systems within this category, they suppress tremor by generating opposing forces or locally increasing stiffness, while preserving voluntary movement as much as possible. Their main advantage is the potential for more discrete and wearable designs compared to rigid exoskeletons, although their tremor suppression effectiveness is generally lower [5].

An example of a soft actuator-based system is the wearable tremor suppression glove (WTSG) [7]. The device operates through a mechanically cable-driven system that suppresses tremor in the fingers and wrist. Inertial measurement units (IMUs) continuously monitor hand motion and distinguish voluntary movement from tremor using a tremor estimation algorithm. Subsequently, brushless DC (BLDC) motors actuate flexible non-stretchable cables that apply pulling forces to the finger and wrist joints. In this way, tremor is mechanically damped while voluntary movement is largely preserved. The system therefore combines active motion sensing with cable-based mechanical suppression in a compact wearable glove design [7]. Figure 2 shows the WTSG prototype worn on the hand.

<img width="410" height="442" alt="image" src="https://github.com/user-attachments/assets/f2c95a0d-3c0b-4e58-9170-3ff1b55b877d" />

  <b>Figure 2:</b> WTSG prototype on hand [7]
<br><br>

#### Functional Electrical Stimulation (FES)
Functional electrical stimulation (FES) uses electrical pulses to activate specific muscles in order to counteract tremor motion. Two principal strategies are commonly applied. The first is co-contraction, in which antagonist muscles are simultaneously activated to increase joint impedance. The second is out-of-phase stimulation, where the antagonist of the tremor-producing muscle is stimulated to generate an opposing force [5].

The main advantages of FES are its aesthetic benefits, as the systems can be designed to be significantly smaller and more discreet than mechanical alternatives. However, the technique requires highly accurate real-time synchronisation, as delays in stimulation can significantly reduce its effectiveness [5].

A portable functional electrical stimulation device for knee osteoarthritis is an example of motion modification using this technique. The system employs a pressure sensor placed under the heel to detect the early stance phase during walking. At this moment, electrical stimulation is applied to the quadriceps muscles to induce eccentric contraction, thereby increasing knee flexion and improving shock absorption during gait. Consequently, mechanical loading on the knee joint is reduced while normal walking motion is largely preserved [8]. Figure 3 illustrates the concept used in this experimental setup.

<img width="671" height="399" alt="image" src="https://github.com/user-attachments/assets/552a59d5-82f7-40cb-9543-27aac5d4f906" />

 <b>Figure 3:</b> Experimental setup for FES device [8] 
<br><br>
#### Afferent neurostimulation
The latest main technique is based on afferent neurostimulation. Afferent neurostimulation is a tremor suppression approach that does not directly act on the muscles, but instead stimulates sensory nerve pathways to modulate central nervous system activity. The underlying idea is that afferent input influences tremor-related neural circuits in the brain and spinal cord, leading to a corrective motor response that can reduce oscillatory activity [5].

Overall, the effect is based on neural modulation rather than direct mechanical opposition, as in exoskeleton-based systems. While studies report promising results, the effectiveness is variable across patients and generally less consistent than some other wearable tremor suppression approaches [5].

An example of a device that uses afferent neurostimulation is a wireless wearable tremor modulation system designed for the suppression of upper-limb kinetic tremor. The system continuously records wrist motion using a three-axis accelerometer to detect tremor in real time and delivers electrical stimulation to branches of the radial nerve via surface electrodes placed at the wrist. This peripheral stimulation provides afferent sensory input that modulates central tremor-generating networks, resulting in a reduction of tremor amplitude and power during functional movements [9].

The device is designed as a compact, wrist-worn system and can be compared to wearing a wristwatch in terms of placement and general form factor. Stimulation is applied in either a continuous open-loop mode or a phase-locked closed-loop mode, depending on the detected tremor dynamics. By combining real-time motion sensing with feedback-based neuromodulation, the system aims to attenuate tremor while preserving voluntary movement [9].
Figure 4 shows a schematic representation of the device and its operating principle.

<img width="725" height="324" alt="image" src="https://github.com/user-attachments/assets/8d12dca3-ba58-47e3-b10e-c49e71a1a7dd" />

  <b>Figure 4:</b> Schematic representation of the working principle of the wrist device [9]
<br><br>

Finally, Figure 5 provides a schematic overview of the four main categories of wearable devices for tremor suppression.

<img width="675" height="516" alt="image" src="https://github.com/user-attachments/assets/7f2760d0-f607-488d-89f2-f901ec4c3ecc" />

  <b>Figure 5:</b> Schematic overview of the four major categories of wearable devices for tremor suppression [9]
<br><br>

### Knowledge gaps/limitations
Although the discussed approaches demonstrate promising results in tremor suppression, several important limitations remain. Robotic exoskeletons, while effective, are often too bulky and uncomfortable for prolonged daily use [5]. Soft robotic alternatives improve wearability but generally achieve lower suppression performance [5]. FES-based systems require precise timing and are sensitive to delays, limiting their robustness in real-world conditions [5]. Afferent neurostimulation shows variable effectiveness across patients and lacks the consistency needed for reliable tremor management [5]. 

A common limitation across all these categories is the complexity of separating voluntary movement from tremorous motion in real time, which remains an open challenge. Furthermore, most existing systems are designed as standalone suppression devices and do not leverage haptic feedback as a means of enhancing user awareness or improving motor control. The integration of haptic technology could offer a complementary advantage by providing the user with real-time sensory cues, potentially improving voluntary movement accuracy alongside mechanical suppression. 

The present project aims to address this gap by exploring tremor suppression through the Edu Exo robotic exoskeleton platform, with a focus on accessibility and ease of use for the end user. The next section provides an overview of the materials required to realise this project.

---
## Supplies (Bill of Materials)

The table below provides an overview of the materials and components required for this project. The primary component is the EduExo Lite kit, which already includes several sensors and other essential components. Where possible, a link to the exact component has been included, or alternatively, to a suitable substitute when the exact component could not be identified online. Some prices listed in the table were converted from foreign currencies to euros using the exchange rates applicable at the beginning of May 2026. Figure 6 presents all components included in the EduExo Lite kit.

<figure align="center" style="text-align: center;">
  <img width="759" height="544"
  src="https://github.com/user-attachments/assets/20c3b90b-7534-4180-bff5-db5af7710528" />

 <b>Figure 6:</b> Overview of components included in the EduExo Lite kit [10]


<br><br>

| Component | Purpose | Qty | Estimated Price | Link |
|---|---|---|---|---|
| EduExo Lite kit [10] | Kit containing the primary hardware components | 1 | ± €490 (incl. home delivery) | [Auxivo EduExo Lite](https://www.auxivo.com/product-page/eduexo-2-0) |
| Stepper motor (included in kit) | Actuation | 1 | Included in EduExo | Included in EduExo |
| IMU (MPU6050) [11] | Motion sensing | 2 | ± €11 (per unit) | [Adafruit MPU6050](https://www.adafruit.com/product/3886) |
| Arduino Nano (included in kit) | Control and processing | 1 | Included in EduExo | Included in EduExo |
| Drake Impact Haptic Actuator [12] | User feedback | 1 | ± €150 (pack of 4) | [Titan Haptics Drake](https://titanhaptics.com/drake/) |
| Driver (DRV2605L) [13] | Haptic actuator control | 1 | €7 | [Adafruit DRV2605L](https://www.adafruit.com/product/2305) |
| Potentiometer [14] | Sensitivity tuning | 1 | ± €1.50 | [Potentiometer](https://www.thomann.de/be/partsland_potentiometer_500kohm.htm) |
| 9V battery [15] | System power supply | 1 | ± €1.60 | [9V Battery](https://www.allekabels.be/blok-batterij/7289/4312202/blok-batterij.html) |
| Wiring and connectors [16] | System integration | - | ± €5 | [Jumper wires](https://www.123-3d.nl/123-3D-Jumper-kabel-dupont-mannelijk-naar-mannelijk-50-cm-40-stuks-i3168.html) |
| Nylon sock with sewing supplies | Mounting of haptic and muscle sensors | 1 | €5 – €10 | Any suitable sock |
| Glue [17] | Securing wires to the EduExo kit | - | ± €1.50 | [Glue sticks](https://www.123-3d.nl/123inkt-lijmpatronen-7-mm-12-stuks-i11554.html) |

---

## Methods  

### Conceptual framework
This section discusses the selection of the different components and how they are intended to work together.

The basic concept operates by using two IMUs (MPU6050) to measure the acceleration and position of both the forearm and upper arm of the right arm. One IMU is positioned on the outer side of the forearm, while the second IMU is attached to the outer side of the upper arm. These measurements can then be compared with each other. In combination with the data obtained from the muscle sensor, which detects involuntary muscle contractions, the system can estimate the severity of the tremor, including its amplitude and frequency. Based on these measurements, the software controls the servo motor, which is responsible for suppressing the detected tremor.

The potentiometer serves a dual role. When turned past approximately 55% of its travel, it triggers SUPPRESS mode; when turned below 45%, it exits SUPPRESS mode. A dead-band between these thresholds prevents oscillation near the midpoint. Within the upper half of the potentiometer range, the ADC value is mapped to a gain between 0.0 and 1.0 that controls the strength of the position-hold force applied during tremor detection. 

The Drake haptic actuator provides haptic feedback to indicate events such as system start-up or the currently selected operating mode of the arm. The actuator is intended to be sewn onto the nylon sock so that, when the sock is worn, the haptic vibrations can be effectively transferred to the user. The exact position depends on how the actuator is sewn onto the sock, but the intended placement is on the forearm near the wrist. The actuator is controlled through the DRV2605L driver, which enables different haptic effects to be communicated to the user.

The DRV2605L haptic driver is initialised at startup. Regretfully, the startup click effect was disabled during development after it was found that the inrush current from the strong-click waveform caused a voltage drop on the shared 5V rail sufficient to trigger the ATmega328P brownout detector, resulting in an infinite reset loop. A decoupling capacitor of 470–1000 µF across the motor supply would be required before re-enabling this feature.


All components are connected using jumper wires and are neatly routed towards the central Arduino Nano with the aid of glue. Where necessary, multiple wires were soldered together into a single connection to allow easier integration with the Arduino Nano. A complete [wiring diagram](Technical%20docs/Wiring_Overview.pdf) showing all connections to the Arduino Nano can be found in the Technical Docs folder. Additional documentation regarding the connection ports of the [EMG muscle sensor](Technical%20docs/MyowareUserManualAT_04_001(muscle_sensor).pdf), the [IMU (MPU6050)](Technical%20docs/mpu6050-6-dof-accelerometer-and-gyro.pdf), the [haptic actuator driver (DRV2605L)](Technical%20docs/adafruit-drv2605-haptic-controller-breakout.pdf), and the [input/output pins of the Arduino Nano](Technical%20docs/ArduinoNanoConnections.jpeg) can also be found in this folder.

The entire system can be powered using a 9 V battery positioned next to the Arduino Nano on the inside of the upper arm section of the exoskeleton. In practice, however, the arm is often powered directly through a laptop during development. This would allow the arm to work as an individual system. But, because of the many sensors connected to the microcontroller, the 9 V battery would discharge relatively quickly during prolonged standalone operation.

### Practical building plan 
A step-by-step assembly guide is provided together with the EduExo kit, which assisted in constructing the arm. The manual also includes instructions for wiring the sensors and motor components. In this project, however, the threaded inserts for the screws were heat-inserted using a soldering iron instead of being screwed into place, as this resulted in a more secure and reliable fit.

Furthermore, the sensors were mounted on the exterior of the arm using adhesive. The wires were carefully bundled together with zip ties and glued along the structure towards the sensors to improve cable management. The potentiometer was attached to the side of the arm in an ergonomic position to allow comfortable operation. Finally, the Drake haptic actuator was sewn onto the sleeve using needle and thread. Figure 7 illustrates the different components integrated into the arm.

<img width="883" height="683" alt="image" src="https://github.com/user-attachments/assets/57e5ac80-b853-419f-8558-0fc599bd737c" />

<b>Figure 7:</b> Different components used: IMU 1 (a), Driver (DRV2605L) (b), haptic actuator (c), servomotor (d), potentiometer (e), IMU 2 (f), EMG sensor (g)
<br><br>

With the exoskeleton arm supports attached, the final assembly of the arm is shown in Figure 8.

<img width="580" height="521" alt="image" src="https://github.com/user-attachments/assets/79ed286d-dd18-4903-9b5a-24c29cb95e49" />

<b>Figure 8:</b> Exoskeleton with arm supports
<br><br>

### System architecture 
The system operates as five sequential functional layers running at 100 Hz. Figure 9 provides a schematic overview.

<img width="940" height="470" alt="image" src="https://github.com/user-attachments/assets/40127e58-64e7-47ee-aaf4-2e178d1f481d" />

<b>Figure 9:</b> System architecture
<br><br>

The sensing layer reads both MPU-6050 IMUs over I²C, the MyoWare EMG signal on A1, the potentiometer on A0, and the servo feedback on A3 at the start of each 10 ms cycle.

The filtering layer subtracts the bias-corrected upper-arm gyroscope from the forearm gyroscope to produce a differential signal with common-mode rejection. The L1 norm of this vector passes through a second-order Butterworth bandpass filter (2–8 Hz), a full-wave rectifier, and a 50 ms IIR envelope follower, yielding a scalar tremor amplitude in deg/s.

The classification layer compares smoothed EMG and tremor amplitude against two EEPROM-stored thresholds. Voluntary contraction (high EMG) takes priority and always prevents suppression. If tremor amplitude exceeds its threshold while EMG is low, the classifier outputs state 2 (tremor). Otherwise state 0 (rest) is assigned.

The actuation layer uses a position-hold strategy. The servo detaches completely during rest and voluntary movement, leaving the arm free. On tremor onset, it attaches at the arm's current position and holds it with gain-weighted force. After tremor stops, it remains attached for 400 ms to avoid rapid cycling, then detaches.

The feedback layer assigns distinct DRV2605L haptic effects to system events, like suppression engagement, fault, and gain saturation,  through the Drake actuator on the forearm sock.

All data is streamed at 115200 baud over a fixed ASCII CSV protocol to a Python dashboard that displays live plots of tremor amplitude, EMG, gyroscope signals, and servo angle.

### Firmware implementation
The firmware is split into independent modules: filter.cpp (Butterworth bandpass and envelope follower), calib.cpp (EEPROM calibration struct), state.cpp (FSM transitions), control.cpp (servo position-hold logic), and firmware.ino (main loop, command parser, telemetry).

Timing uses micros() scheduling with a fixed 10,000 µs increment, avoiding the cumulative drift of delay-based approaches. Worst-case cycle time on the ATmega328P is approximately 3–4 ms, well within the 10 ms budget.

Two AVR constraints shaped the implementation directly. AVR snprintf does not support %f, so all floats are transmitted as scaled integers (e.g. 12.34 deg/s becomes 12,34 via %u.%02u). The sqrtf() function was found to cause stack overflow on the 2 KB RAM device, so the L1 norm replaces Euclidean magnitude throughout. All buffers are statically allocated and no dynamic memory is used.

Calibration data (36 bytes) is stored in EEPROM from address 0, validated by a magic number sentinel (0xCAB1). If absent on boot, safe defaults load automatically. The serial protocol uses three prefix types: T for 100 Hz telemetry, E for events and state transitions, and C for commands from the dashboard. The full specification is in shared/PROTOCOL.md.

### Calibration procedure

Before first use, a one-time calibration captures individual sensor baselines and servo range. Pressing C in the dashboard sends C,CALIB_START. The user holds the arm still for three seconds while the firmware averages 300 samples of gyroscope and EMG data to compute bias values. The user then moves through their full range of motion, allowing the firmware to record the servo feedback limits. Pressing S sends C,CALIB_SAVE, committing all values to EEPROM. On subsequent boots, calibration loads automatically.

### Troubleshooting/improvements

Several issues were encountered and resolved during development, documented here for reproducibility.
Firmware upload failed initially because the Arduino IDE was configured for the jtag2updi programmer, which targets newer AVR devices. Switching the board profile from Arduino Nano Every to Arduino Nano resolved this.

The Zadig USB driver utility was used experimentally, which replaced the CDC serial driver with WinUSB and caused the COM port to disappear. Recovery required uninstalling the device in Device Manager and reconnecting the Arduino to restore the original driver.

After closing the Arduino IDE, a background arduino-cli.exe process retained the COM port, blocking the Python dashboard. The fix was to fully terminate the IDE process tree before launching the dashboard.

The sqrtf() function caused a stack overflow on the 2 KB ATmega328P due to AVR floating-point library overhead. Replacing it with the L1 norm eliminated the crash and reduced flash usage with no meaningful loss of detection accuracy.

The EMG signal initially saturated near the ADC maximum (>900/1023) due to suboptimal electrode placement. By repositioning the electrodes on the belly of the flexor carpi radialis muscle and improving skin contact, the baseline signal was reduced to approximately 50–100 ADC counts, while still providing clear peaks during voluntary muscle contraction.

In addition, simulating a tremor without unintentionally activating the muscle sensor proved to be challenging in subjects without an actual tremor. As a result, the EMG measurements during active tremor suppression can sometimes contain distorted or less representative values.

#### Hardware limitations 
Due to the large number of hardware components that need to be powered, the 9V battery drains quickly. Therefore, during this development phase, the arm is continuously powered through a cable connected directly to the laptop. In addition, this cable is required for reading sensor data. In a more advanced version, this limitation could be addressed by implementing a dedicated battery pack together with a Bluetooth module for wireless data streaming.

Furthermore, the large number of wires connected to the different components occasionally caused wiring issues. In many cases, cables had to be soldered together before being connected to the Arduino through a single wire. As a result, the interior of the arm contains a dense and difficult-to-organize wiring setup. Future iterations could therefore focus on improving the efficiency and organization of the wiring system. Figure 8 shows the wiring inside the arm. 

<img width="331" height="442" alt="image" src="https://github.com/user-attachments/assets/983853d7-0a70-4755-96cb-1e75ba35fe4a" />

<b>Figure 10:</b> Complex wiring inside the arm
<br><br>

Since the EduExo kit is a robotic exoskeleton, it also inherits several disadvantages discussed in the introduction, such as being relatively bulky and uncomfortable to wear for extended periods of time. A more refined end product could attempt to minimize these limitations by reducing the overall weight of the system, for example.

Finally, the servomotor included in the EduExo kit is currently too limited to provide fully active tremor suppression during large tremors while the arm is moving. In this development phase, tremor suppression is therefore mainly demonstrated during stationary arm positions. As such, the current implementation should be considered primarily as a proof of concept to measure tremors and demonstrate that suppression can occur, or could be further improved with the use of a more powerful motor.

---

## Results & Discussion  

Word sectie wordt hier geplakt. 

Key observations:  
- Effective reduction of tremor amplitude at specific frequencies  
- Improved user comfort compared to electrical stimulation  
- Sensitivity tuning is essential for optimal performance  

Limitations:  
- Limited torque output of the stepper motor  
- Latency in signal processing  
- Mechanical alignment challenges  

---

## Conclusion & Future Work  

Word sectie wordt hier geplakt.

---

## References  

Word sectie wordt hier geplakt.

---

## Demo Video  

To make this project accessible to a broader audience, an engaging introductory video was also created. It can be viewed at the following link:

https://youtu.be/2Iga29aqnwY

---

## Repository Structure  
