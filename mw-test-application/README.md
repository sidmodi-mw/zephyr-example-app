# Zbus Example Application

## Overview
This is a simple **Zephyr RTOS** application demonstrating the use of **Zbus** for inter-thread communication.

### **Application Flow**
- The application consists of **two threads**: `button_thread` and `led_thread`.
- The **button module** monitors a **physical GPIO button** and triggers an **interrupt on each edge**.
- The **interrupt handler** sends the event to an **internal message queue**.
- The **button thread** pushes the event into a **Zbus channel**.
- The **LED module** registers a listener to the **button channel** and sends the event to an **internal message queue**.
- The **LED thread** receives this event and toggles an LED based on the button state:
  - **Button Pressed** → LED **ON**
  - **Button Released** → LED **OFF**

### **Key Features**
- Uses **Zbus** for event-driven communication.
- Demonstrates **thread synchronization using Zbus and internal queues**.
- Includes **unit tests using Ztest**.
- Supports **GPIO emulation** for testing on `native_sim`.

## Supported Boards
The example has been tested on:
- **nRF52840DK** (`nrf52840dk/nrf52840`)
- **Zephyr Native Simulator** (`native_sim`)

## Unit Tests
This application includes **two unit tests** built with **Ztest**:
- Tests the **button thread** and **LED thread** using **GPIO emulation**.
- Tests the **Zbus communication** between the button and LED threads.

## **Building and Running**

### **Linux Requirements**
Ensure the following are installed:
- **Docker** (Ensure Docker can run without root privileges)
- **VS Code**
- **Remote - Containers Extension** for VS Code

### **Steps to Build and Run**
1. **Launch VS Code** in the repository folder.
2. **Reopen in Container** (`Remote - Containers` extension required).
3. Use **Task Manager**:
   - **Build Task** (`west build` for `native_sim` or `nrf52840dk`)
   - **Test Run Task** (`twister` for unit tests)

Note: The `devcontainer.json` file runs the `west init` and `west update` commands after the container launches.

### **Flashing on nRF52 DK**
- Build the image using task manager for the nRF52dk board.
- Use the command `west flash --runner jlink` from the `mw-test-application` folder.
- Open the **Serial Monitor** tab and start monitoring the logs.
