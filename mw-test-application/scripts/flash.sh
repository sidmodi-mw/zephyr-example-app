#!/bin/bash

# Ensure board argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <board> [pristine] [app_directory]"
    exit 1
fi

BOARD=$1
PRISTINE_MODE=${2:-auto}  # Default to 'auto' if not provided
APP_DIR=${3:-.}  # Default to current directory if not provided

# Determine the appropriate runner
case "$BOARD" in
    "nrf52840dk/nrf52840") RUNNER="jlink" ;;
    "nucleo_f401re") RUNNER="openocd" ;;
    "native_posix") RUNNER="" ;;
    *) 
        echo "Unknown board: $BOARD"
        exit 1
        ;;
esac

echo "Building application for application in directory: $APP_DIR for board: $BOARD with pristine mode: $PRISTINE_MODE"
west build -p "$PRISTINE_MODE" -b "$BOARD" "$APP_DIR"

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Flashing using runner: $RUNNER"
west flash --board "$BOARD" --runner "$RUNNER"

if [ $? -ne 0 ]; then
    echo "Flashing failed!"
    exit 1
fi

echo "Build and flash successful!"
exit 0
