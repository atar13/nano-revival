# nano-revival

> [!WARNING]  
> Work in progress! 

A portable media player to take you back to the early 2000s!

Using the [Sipeed M0S module](https://wiki.sipeed.com/hardware/en/maixzero/m0s/m0s.html) with the BL616 RISC-V microcontroller.

Built using the [Boufallo SDK](https://github.com/bouffalolab/bouffalo_sdk)

## Components:

- [X] Audio playback via DAC peripheral
- [X] Debugging shell over USB. Thanks to Emil Lerch's [amazing blog post](https://emil.lerch.org/getting-to-hello-world-with-sipeed-m0s-bl616/) ❤️
- [ ] File loading from SD card
- [ ] PCB to connect M0S module and external peripherals
- [ ] 3D printed case

## Build

Setup development environment for [Boufallo SDK](https://github.com/bouffalolab/bouffalo_sdk#command-line-development). Make sure to modify the `CMAKE_PREFIX_PATH` defined in `firmware/CMakeLists.txt` and `BL_SDK_BASE`  in `firmware/Makefile` to point to your local Boufallo SDK installation.

```sh
cd firmware
make CHIP=bl616 BOARD=bl616dk
```

Or use the [(mostly) open sourced version of the SDK](https://git.lerch.org/lobo/bouffalo_open_sdk)

```sh
cd firmware
docker run --rm -t -v $(pwd):/build git.lerch.org/lobo/bouffalo_open_sdk:2f6477f BOARD=bl616dk CHIP=bl616
```

## Flash

Specify the device's correct serial port (usually /dev/ttyACM0 on Linux):
```sh
cd firmware
make flash CPU_ID=m0 CHIP=bl616 COMX=/dev/ttyACM0
```

or

```sh
docker run --rm --device /dev/ttyACM0 -v $(pwd):/build git.lerch.org/lobo/bouffalo_open_sdk:2f6477f flash BOARD=bl616dk CHIP=bl616 COMX=/dev/ttyACM0
```
