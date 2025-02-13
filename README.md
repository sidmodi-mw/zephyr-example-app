# VS Code DevContainer for Zephyr RTOS Development

This repository provides a **template workspace** for developing **Zephyr RTOS applications** inside a [VS Code Development Container](https://code.visualstudio.com/docs/devcontainers/containers).

---

## Requirements
Before using this repository, ensure the following prerequisites are met:

### General Requirements
- Install [Docker](https://www.docker.com/products/docker-desktop) and ensure it is running.
- Install the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) in VS Code.
- Ensure Docker can run without root privileges.

### Linux Requirements
- No additional setup is needed.

### Windows Requirements
- Install **WSL** following [this guide](https://learn.microsoft.com/en-us/windows/wsl/install).
- Enable the **WSL2 backend** for Docker as described [here](https://learn.microsoft.com/en-us/windows/wsl/tutorials/wsl-containers).
- Configure WSL memory allocation per [these instructions](https://www.aleksandrhovhannisyan.com/blog/limiting-memory-usage-in-wsl-2/).
- **Ensure the repo is cloned inside the WSL filesystem**, e.g., `/home/username/`.
- To allow hardware access within the dev container without root privileges:
  1. Copy the `.devcontainer/rules/*` files to `/etc/udev/rules.d/`.
  2. Run the following after a system reboot or WSL restart:
     ```sh
     sudo service udev restart
     sudo udevadm control --reload
     sudo udevadm trigger
     ```
- Follow [this guide](https://learn.microsoft.com/en-us/windows/wsl/connect-usb) to connect USB devices to WSL.
  - When using `usbipd attach`, add `--auto-attach` to persist connections across power cycles.

---

## Getting Started
1. Open the repository in **VS Code**.
2. Open the command palette (**CTRL+SHIFT+P**) and select:  
   **"Dev Containers: Reopen in Container"**.

---

## West Workspace Configuration
This project follows Zephyr's **[Star Topology](https://docs.zephyrproject.org/latest/develop/west/workspaces.html#t2-star-topology-application-is-the-manifest-repository)** where:
- The West manifest (`west.yaml`) is located inside `<project-name>`.
- `west init` and `west update` are executed automatically when the container starts (check the `postCreateCommand` in the `devcontainer.json` file and the `setup_west.sh` file).
- The manifest specifies the external modules, including Zephyr RTOS.
- To modify modules, update the manifest and run:
  ```sh
  west update
  ```

---

## Zbus Example Application
### Overview
This project demonstrates Zbus-based inter-thread communication in Zephyr.

### Key Features
✅ Zbus based event-driven communication  
✅ Thread synchronization using internal queues  
✅ Unit tests using Ztest  
✅ GPIO & UART emulation for `native_sim`  
✅ Zephyr Shell integration for debugging

### Application Flow
- The Button module:
  - Detects button presses via a GPIO interrupt.
  - Sends events to Zbus via the button channel.
- The LED module:
  - Listens for button events on the button channel.
  - Toggles an LED based on button state:
    - Pressed → ON
    - Released → OFF
- The UART module:
  - Uses Zephyr’s Async API to collect 256-byte messages.
  - Sends data into a Zbus channel.
- Zephyr Shell serves as the logging backend.

---

## Supported Boards
| **Board**              | **Identifier**        |
|------------------------|----------------------|
| nRF52840 Development Kit | `nrf52840dk/nrf52840` |
| STM32 Nucleo F401RE    | `nucleo_f401re`      |
| Zephyr Native Simulator | `native_sim`        |

---

## Unit Testing
This project includes unit tests for:
- **Button & LED modules** (GPIO emulation).
- **UART module** (UART emulation).
- **Zbus communication**.

### Running Unit Tests
1. Using Task Manager:
   - Select **Twister Run**.
2. Using CLI:
   ```sh
   west twister --clobber-output -p native_sim -T <path_to_tests/>
   ```

---

## Building

### Option 1: Using Task Manager
- Select the **Build Task** (prompts for board selection).

### Option 2: Using CLI
```sh
west build --p auto -b native_sim -d <path_to_app/build> -s <path_to_app/>
```

---

## **Flashing on Target Hardware**
### Option 1: Using Task Manager
- Run **Build & Flash Task** (prompts for board selection).

### Option 2: Using CLI
For **nRF52840DK**:
```sh
west flash -d <path_to_app/build> --runner jlink
```
For **STM32 Nucleo**:
```sh
west flash -d <path_to_app/build> --runner openocd
```
After flashing, open **Serial Monitor** to view logs.

---

## Debugging on Target Hardware
Debugging is configured via `.vscode/launch.json`:
Launch a debug session via the **Run & Debug** panel.

---

## **Nordic MCU-Specific Setup**
To use **Nordic's Zephyr fork** & **nRF Connect SDK**:
1. Modify `.devcontainer/devcontainer.json`:
   ```json
   "image": "ghcr.io/mistywest/zephyr-nrf:v2.9.0-amd64"
   ```
2. Add the following to the `extensions` section of `.devcontainer/devcontainer.json`:
   ```json
   "nordic-semiconductor.nrf-connect",
   "nordic-semiconductor.nrf-connect-extension-pack"
   ```
3. Ctrl + Shift + P: Rebuild Container.
4. Navigate to the **nRF Connect Extension**, add the `mw-test-application` using the **"Add an existing project"** feature.
5. Use the Build/Flash/Debug features from the extension.
