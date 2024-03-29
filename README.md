# nano-revival

> [!WARNING]  
> Work in progress! 

A portable media player to take you back to the early 2000s!

Using the [Sipeed M0S module](https://wiki.sipeed.com/hardware/en/maixzero/m0s/m0s.html) with the BL616 RISC-V microcontroller.

Built using the [Boufallo SDK](https://github.com/bouffalolab/bouffalo_sdk)

## Firmware:

- [X] Audio playback via DAC peripheral
- [X] Debugging shell over USB. Thanks to Emil Lerch's [amazing blog post](https://emil.lerch.org/getting-to-hello-world-with-sipeed-m0s-bl616/) ❤️
- [ ] File operations from SD card

## Hardware:

- [X] [M0S Sipeed Module](https://www.aliexpress.us/item/3256804956152184.html?gatewayAdapt=glo2usa4itemAdapt)
- [X] [microSD Card Breakout Board](https://www.adafruit.com/product/254)
- [X] [3.5mm Jack Breakout Board](https://www.sparkfun.com/products/11570)
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

> [!IMPORTANT]  
> On the Sipeed M0S Dock, make sure to perform the followings steps before attempting to flash:
> 1. Unplug USB-C cable
> 2. Hold down the `BOOT` button
> 3. While holding down `BOOT`, plug in the USB-C cable
> 4. Flash 
>
> After flashing, reboot board without holding `BOOT` to get out of flashing mode

Specify the device's correct serial port (usually /dev/ttyACM0 on Linux):
```sh
cd firmware
make flash CHIP=bl616 COMX=/dev/ttyACM0
```

or

```sh
docker run --rm --device /dev/ttyACM0 -v $(pwd):/build git.lerch.org/lobo/bouffalo_open_sdk:2f6477f flash BOARD=bl616dk CHIP=bl616 COMX=/dev/ttyACM0
```
