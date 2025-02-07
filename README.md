# VSCode devcontainer for Zephyr RTOS

This is a template workspace for developing Zephyr RTOS application in a 
[VS Code development container](https://code.visualstudio.com/docs/devcontainers/containers). 

## Requirements

Make sure the prerequisites are satisfied, as described in the [Dev Containers tutorial](https://code.visualstudio.com/docs/devcontainers/tutorial). In particular, that [Docker](https://www.docker.com/products/docker-desktop/) is up and running and the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) is installed. 


Finally, launch the container from inside VSCode with **CTRL+SHIFT+P** then 
**Dev Containers: Reopen in container**.

# Zbus Example Application

## Overview
This is a simple **Zephyr RTOS** application demonstrating the use of **Zbus** for inter-thread communication.

### **Application Flow**
- The application consists of **two threads**: `button_thread` and `led_thread`.
- The **button module** monitors a **physical GPIO button** and triggers an **interrupt on each edge**.
- The **interrupt handler** sends the event to an **internal message queue**.
- The **button thread** monitors the queue and pushes the event into a **Zbus channel**.
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

### **Ways to Build and Run**
1. Use **Task Manager**:
   - **Build Task** (`west build` for `native_sim` or `nrf52840dk`)
   - **Test Run Task** (`twister` for unit tests)

**OR**

2. Run manual `west` commands from the CLI.

Note: The `devcontainer.json` file runs the `west init` and `west update` commands after the container launches.

### **Flashing on nRF52 DK**
- Build the image using task manager for the nRF52dk board.
- Use the command `west flash --runner jlink` from the `mw-test-application` folder.
- Open the **Serial Monitor** tab and start monitoring the logs.
