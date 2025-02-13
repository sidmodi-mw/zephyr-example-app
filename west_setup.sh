#!/bin/bash

set -e  # Exit on any error

PROJECT_DIR=$(dirname "$0")  # Get the script directory
WEST_DIR="${PROJECT_DIR}/.west"

if [ ! -d "$WEST_DIR" ]; then
    echo "No .west directory found. Initializing West..."
    west init -l mw-test-application
else
    echo ".west directory already exists. Skipping initialization."
fi

west update --narrow -o=--depth=1
west zephyr-export