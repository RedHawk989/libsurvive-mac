#!/bin/bash
# Run libsurvive to get Vive tracker position in terminal.
# This script is copied to the build dir by CMake; run it from there (same dir as impsurvive_api_example).
# Requires: Vive tracker plugged in via USB, base stations on, SteamVR closed.
# On macOS, sudo is needed for USB access.
# Calibration is reset each run: we delete the saved config then pass --force-calibrate.
CONFIG_PATH="${XDG_CONFIG_HOME:-$HOME/.config}/libsurvive/config.json"
rm -f "$CONFIG_PATH"
sudo bash -c 'rm -f "${XDG_CONFIG_HOME:-$HOME/.config}/libsurvive/config.json"'
cd "$(dirname "$0")"
sudo ./impsurvive_api_example --force-calibrate
