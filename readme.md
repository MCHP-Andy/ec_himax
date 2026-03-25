# Himax Demo

## Zephyr toolchain
Windows Minimal: [link](https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_windows-x86_64_minimal.7z)

arm-zephyr-eabi: [link](https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/toolchain_windows-x86_64_arm-zephyr-eabi.7z)


## SDK install
```
tools\script\install.bat
``` 

## Setup
```
tools\script\setup.bat
```

## Build
```
west build -p -b ec_card/mec1753_qsz app
```

## Flash
```
cd tools\KF_JLINK_Flash_Utility_L0100
python kf_flsh_util.py -w -f ..\..\build\zephyr\spi_image.bin
```