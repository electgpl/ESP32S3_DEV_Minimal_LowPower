# ESP32S3_DEV_Minimal_LowPower

This project implements a **minimal low-power development board** based on the **ESP32-S3**.  
The design focuses on reducing component count, board complexity, and idle power consumption, while still exposing the full capabilities of the ESP32-S3 for advanced development.  

Unlike traditional development boards, this design does **not include a USB-to-UART bridge**. Instead, it leverages the **native USB interface** of the ESP32-S3 for programming and communication, resulting in a simpler and more efficient design.  

---

## 🔑 Features

- **ESP32-S3 MCU** with external **QSPI Flash memory**.  
- **Native USB 2.0 interface** used for flashing and serial communication.  
- **MCP1826S LDO regulator**:  
  - Low quiescent current (IQ) for low-power operation.  
  - Stable supply for ESP32-S3 core.  
- **4-layer PCB**:  
  - Designed and manufactured at **NextPCB (PCBA service)**.  
  - Optimized for **low EMI/EMC compliance**.  
- **Castellated edges** for easy integration into other projects.  
- **Full pinout exposure**:  
  - All ESP32-S3 GPIOs available, including those not accessible on standard WROOM modules.  
  - Enables advanced use cases and maximum flexibility.  
- **On-board PCB antenna (MIFA type)**:  
  - Designed with **uSimmics EM simulation**.  
  - Verified radiation characteristics, dispersion parameters, and stack-up effects.  

---

## 📂 Repository structure

- **/schematic/** → Circuit schematic files.  
- **/pcb/** → PCB layout and Gerber files.  
- **/doc/** → Antenna simulation results and design notes.  
- **/firmware/** → Demo applications and example projects.  
- **/img/** → PCB renders and prototype photos.  

---

## 🧪 Demo Application

Included with this project is a **human presence detection demo** based on Wi-Fi spectral footprint analysis:  

- Scans available Wi-Fi networks, collecting **SSID** and **RSSI** values.  
- Applies a **probability-based algorithm** using **normal distribution** to estimate environmental changes.  
- Detects human presence based on variations in the Wi-Fi spectrum.  
- Displays results on a **color IPS display**.  

This demonstrates the potential of the ESP32-S3 in **low-cost sensing and AIoT applications** without requiring dedicated motion sensors.  

---

## 🔧 Typical applications

- Low-power IoT node development.  
- Integration into custom PCBs via castellated module design.  
- Edge-AI and machine learning applications.  
- Antenna research and RF prototyping.  

---

## 📑 References

- [ESP32-S3 Technical Reference Manual (Espressif)](https://www.espressif.com/en/support/download/technical-documents)  
- [MCP1826S LDO Datasheet (Microchip)](https://ww1.microchip.com/downloads/en/DeviceDoc/22057B.pdf)  
- [uSimmics EM Simulation Software](https://www.usimmics.com/)  

---

## 📸 Project preview

*(Insert here PCB renders, antenna simulation plots, and prototype photos)*  

---

## ⚖️ License

This project is released under the **MIT License**.  
You are free to use, modify, and distribute it, provided that proper credit is given.  

---
