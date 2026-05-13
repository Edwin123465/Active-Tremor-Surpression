# Active Tremor Suppression  

**Authors:** Samu Boving & Edwin Arnalsteen  
**Course:** Haptic Interfaces Experience – KU Leuven  
**Professor:** Carlos Rodriguez-Guerrero  
## Description
see later
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




 Word document into wordt hier geplakt. 
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

Word sectie wordt hier geplakt. 

---

## System Architecture  

Word sectie wordt hier geplakt.

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

---

## References  

Word sectie wordt hier geplakt.

---

## Demo Video  

To make this project accessible to a broader audience, an engaging introductory video was also created. It can be viewed at the following link:

https://youtu.be/2Iga29aqnwY

---

## Repository Structure  
