#!/bin/sh
cd "$(dirname "$0")"
sudo dfu-util -a 0 -s 0x08010000 -R -D blank.bin -d ,0483:df11
