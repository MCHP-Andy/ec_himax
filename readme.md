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


## Connect

MCHP EVB: modules\app\boards\microchip\ec_card\doc\MEC1753-240 Evaluation Board.PDF

Himax EVB: tools\himax

|Himax 端訊號|MCHP EVB 端位置|備註|
|:---:|:---:|:---:|
|INT (PA0)|GPIO140 (TP252)|中斷訊號線|
|I3C (SDA)|GPIO012 (TP257)|數據線|
|I3C (SCL)|GPIO013 (TP257)|時鐘線|
|GND|GND|共地|
|---|GPIO054 (JP25),輸出至風扇|PWM|
||||

```mermaid
graph LR
    subgraph Himax_Module ["Himax Module"]
        H_INT["INT (PA0)"]
        H_SDA["I3C (SDA)"]
        H_SCL["I3C (SCL)"]
        H_GND["GND"]
    end

    subgraph MCHP_EVB ["MCHP EVB MEC175x"]
        M_G140["GPIO140 (TP252)"]
        M_G012["GPIO012 (TP257)"]
        M_G013["GPIO013 (TP257)"]
        M_GND["GND"]
        M_PWM["PWM GPIO054 (JP25)"]
    end

    subgraph External_Device ["Module"]
        F_PWM["Fan PWM input"]
    end

    %% Himax to MCHP Connections
    H_INT --- M_G140
    H_SDA --- M_G012
    H_SCL --- M_G013
    H_GND --- M_GND

    %% MCHP to Fan Connection
    M_PWM --- F_PWM

    %% Styling
    style Himax_Module fill:#f9f,stroke:#333,stroke-width:2px
    style MCHP_EVB fill:#bbf,stroke:#333,stroke-width:2px
    style External_Device fill:#dfd,stroke:#333,stroke-width:2px
```
