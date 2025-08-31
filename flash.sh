#!/usr/bin/env bash

# flash.sh - Flash firmware to Raspberry Pi Pico

set -euo pipefail

TIMEOUT=30   # seconds
INTERVAL=1   # seconds between retries

fw_dir="build"
if [[ "${1:-}" == "-r" ]]; then
    fw_dir="release"
fi

echo "Waiting for a pico in BOOTSEL mode..."
start_time=$(date +%s)

while true; do
    if picotool info --vid 0x2e8a --pid 0x0003 &>/dev/null; then
        break
    fi

    now=$(date +%s)
    elapsed=$(( now - start_time ))
    if (( elapsed >= TIMEOUT )); then
        echo "Error: Timed out" >&2
        exit 1
    fi

    sleep "$INTERVAL"
done

echo "Loading firmware from $fw_dir/usb_keyboard.uf2..."
if ! picotool load --vid 0x2e8a --pid 0x0003 "$fw_dir/usb_keyboard.uf2"; then
    echo "Error: Failed to load firmware." >&2
    exit 1
fi

# Step 3: Reboot
echo "Rebooting..."
if ! picotool reboot --vid 0x2e8a --pid 0x0003; then
    echo "Error: Failed to reboot device." >&2
    exit 1
fi

echo "Flashing complete"
