@ECHO OFF
TITLE crow firmware update
ECHO ========================
ECHO Updating crow firmware...
ECHO ========================
dfu-util -a 0 -s 0x08020000 -R -D crow.bin -d ,0483:df11 && (
    ECHO Update successful!
) || (
    ECHO dfu-util failed: For help troubleshooting, see: monome.org/docs/crow/update/
    ECHO If that doesn't help, copy and paste the above output, and post
    ECHO on lines: llllllll.co/t/crow-help-gneral-connectivity-device-qs-ecosystem/25866/
)
PAUSE
