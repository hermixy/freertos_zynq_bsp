# Freertos bsp for Zynq (Microzed, Zedboard, etc.)

*tested on microzed 7010/7020*

## 1. Features

The zynq bsp for freeRTOS, using the cutting-edge version of freeRTOS (currently v8.2.0rc1) and the demos in `CORTEX_A9_Zynq_ZC702` of the official freeRTOS repository.

`freertos_zynq.mld` and `freertos_zynq.tcl` are migrating from Xilinx community ports, the latest version is available via [interactive freertos 2014.2](http://interactive.freertos.org/entries/31659559-Xilinx-Zynq-FreeRTOS-and-lwIP-demo-XAPP1026-Vivado-2014-2) or the official xapp1026 [xapp1026 App note](http://www.xilinx.com/support/documentation/application_notes/xapp1026.pdf), which shipping a BSP of FreeRTOS version v7.0.2.

This bsp aims to illustrate the basis of freertos by porting all demos in official FreeRTOS repository while also includes lwip capabilities.

The demo files of `CORTEX_A9_Zynq_ZC702` are available from official svn or via [freertos sync at github](https://github.com/cjlano/freertos). The building instructions on ZC702 can be found at the official freeRTOS website [RTOS-Xilinx-Zynq](http://www.freertos.org/RTOS-Xilinx-Zynq.html).

## 2. Supported drivers

Currently, the support drivers are :
- [x] FreeRTOS CLI package,
- [x] lwIP package,

TODO list :
- [ ] other demos and sw apps in the `Demo/common` folder,
- [ ] board-aware (like LED toggling, buttons etc.,) demos to the package,
- [ ] matrix algebra package.

## 3. Instructions

Open SDK, and set repository via `Xilinx Tools`, `Repositories` and add (new) the location of the cloned source code.

Open `File`, `New Project`, add the demos, compile and run.
