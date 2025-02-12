#!/usr/bin/env python3
import argparse
import subprocess
import sys

# Define board-to-runner mapping
BOARD_RUNNERS = {
    "nrf52840dk/nrf52840": "jlink",
    "nucleo_f401re": "openocd",
    "native_sim": None
}

def main():
    # Argument parsing
    parser = argparse.ArgumentParser(description="Zephyr Build & Flash Script")
    parser.add_argument("board", help="Board name (e.g., nrf52840dk/nrf52840)")
    parser.add_argument("--pristine", choices=["auto", "always", "never"], default="auto",
                        help="Pistine mode: auto, always, never (default: auto)")
    parser.add_argument("--app", default=".",
                        help="Path to the application directory (default: current directory)")
    parser.add_argument("--flash", choices=["true", "false"], default="true",
                        help="Enable or disable flashing (default: true)")
                        
    args = parser.parse_args()
    build_dir = args.app + "/build"

    # Determine the appropriate runner
    runner = BOARD_RUNNERS.get(args.board)
    if args.board not in BOARD_RUNNERS.keys():
        print(f"Error: Unknown board '{args.board}'.")
        sys.exit(1)

    # Build the application
    print(f"Building application in '{args.app}' for board '{args.board}' with pristine mode '{args.pristine}'...")
    build_cmd = ["west", "build", "-b", args.board, "--pristine", args.pristine, "-s", args.app, "-d", build_dir]

    if subprocess.call(build_cmd) != 0:
        print("Build failed!")
        sys.exit(1)

    # Flash only if enabled
    if args.flash == "true":
        if runner is None:
            print("Flashing is not supported for this board!")
            sys.exit

        # Flash the firmware
        print(f"Flashing using runner '{runner}'...")
        flash_cmd = ["west", "flash", "--skip-rebuild", "-d", build_dir, "--runner", runner]

        if subprocess.call(flash_cmd) != 0:
            print("Flashing failed!")
            sys.exit(1)

    print("Build successful!" + (" Flash successful!" if args.flash == "true" else " (Flashing skipped)"))

if __name__ == "__main__":
    main()
