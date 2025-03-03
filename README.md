# Sample application to evaluate the drv2650 driver in Zephyr OS

This project is based on the sample in zephyr/samples/drivers/haptics/drv2605.

I wanted to be able to change via a shell
- the effect executed on the drv2605
- the effect-library used by the drv2605

Datasheet of the drv2605: https://www.ti.com/lit/ds/symlink/drv2605.pdf

Pre-Requisites on the host:
- git installed
- vscode installed, with plugins "Remote Containers", "Cortex-Debug"
- docker installed
- Segger JLinkServer and JLinkRTTViewer installed

How to use:
1. "git clone https://github.com/stemschmidt/haptic_moto.git"
2. "cd haptic_moto"
3. "code ." or open haptic_moto in VSCode
4. in VSCode, select "reopen folder in container"
5. after docker container has started, go to "TERMINAL" tab (see NOTE below)
6. "west init -l application"
7. "west update"
8. "source zephyr/zephyr-env.sh"
9. "cd application"
10. "west build -b adafruit_feather_nrf52840 --sysbuild" and wait for the build to finish...
11. on the host, launch the JLinkGDBServer (e.g "JLinkGDBServerCLExe -if swd -device <YOUR_DEVICE> -vd")
12. on the host, launch the JLinkRTTViewer to see debug log (e.g "JLinkRTTViewer", select "Connection to J-Link" -> "Existing session")
13. Select "Run and Debug" icon in activity bar in VSCode --> Download binary and start debugging by launching "JLink"

If you are using a different board, add an overlay file in the application/boards folder! You may also have to adapt the key event handling in main.c.

NOTE:
There is an issue with the order in which the plugins are loaded. A warning appears asking you to reload the window.
After reloading the window you will have to launch bash again: '$ bash'
