# Bus-Stop Shade and Street Light Controller System

## Project Overview
Developed for the **Singapore Land Transport Authority (LTA)**, this system is a real-time embedded solution designed to improve commuter comfort and urban safety. It automates the deployment of bus-stop shades and the activation of streetlights based on environmental light intensity, utilizing the **OSEK/VDX automotive operating system standard**.

## System Functionality
 The controller monitors two city sectors (**West Zone** and **East Zone**) using Light Dependent Resistors (LDR). Based on the average light intensity (Lux) from both zones, the system executes the following:

*  **Day Mode ($Lux \ge 500$):** Automatically expands the bus-stop shade via servo motors to protect commuters from the sun.
*  **Night Mode ($Lux \le 200$):** Activates LED streetlights to ensure safety and visibility.
*  **Time-Based Lighting (Optional):** Includes a 24-hour clock module to automatically trigger lights between **18:30** and **07:30**.

## Hardware Architecture
 The system is designed for the **Arduino Uno** platform and simulated within **SimulIDE**:
*  **Microcontroller:** 1 x Arduino Uno.
*  **Sensors:** 2 x LDRs (West & East Zones).
*  **Actuators:** 2 x Servo Motors (Shade Control) and 2 sets of 4 LEDs (Streetlights).
*  **Display:** $20 \times 4$ LCD for real-time system status.

## Software Stack
*  **RTOS:** Erika Enterprise OSEK/VDX.
*  **Configuration:** OIL (OSEK Implementation Language) for Task, Alarm, and Event management.
*  **IDE:** Eclipse (for C++ development).
*  **Drivers:** * `ServoTimer2`: Custom driver used to avoid Timer1 conflicts with the OSEK kernel.
    *  `LiquidCrystal`: Modified driver for Hitachi HD44780 LCD integration.

## Mathematical Model
 The system converts Analog-to-Digital Converter (ADC) values into Lux units using the following power-law formula:

$$Lux = (889985.88) \times R_{LDR}^{-1.16552}$$

*  **Shade Contracted:** 0 degrees PWM signal.
*  **Shade Expanded:** 180 degrees PWM signal.

## How to Run the Simulation
1.   **Environment:** Ensure the Erika Enterprise OSEK environment is correctly set up in your Linux VM.
2.   **Configuration:** Define system tasks and priorities in the `conf.oil` file.
3.   **Build:** Use Eclipse to compile the source code and generate the `.hex` binary.
4.   **Simulation:** * Open the `*.simu` file in **SimulIDE**.
    *  Load the generated `.hex` file onto the virtual Arduino Uno.
    *  Interact with the LDR sensors to observe the shade and light transitions.

---
 **Team Project: CEG2009 - Operating System and Automotive OS** **Submission Date:** 29-Mar-2026 

