#!/bin/sh
cd "$(dirname "$0")"
sudo dfu-util -a 0 -s 0x08020000 -R -D crow.bin -d ,0483:df11
