# HPS Boot Configuration


## Rationale

Certain SoC tasks require more configuration than simply regenetaring QSYS (Platform Designer) components and re-synthesizing the FPGA system.
In particular, reconfiguring hard processor system (HPS) I/O pins requires that the "preloader" be recompiled.

The preloader is the second stage of the HPS boot sequence — it is executed by the first-stage boot code (which is stored in on-chip ROM), and is thus the earliest element of the boot sequence over which we have control.
Among other tasks, the preloader is responsible for configuring HPS I/O pin multiplexing, which controls connection points for certain peripherals, such as I2C and SPI modules.

This document explains and demonstrates the process of building the U-Boot bootloader and its associated preloader (known as the Secondary Program Loader, or SPL) for the DE10-Nano FPGA board with Cyclone V SoC.
It is based in large part on [RocketBoards' documentation](https://www.rocketboards.org/foswiki/Documentation/BuildingBootloaderCycloneVAndArria10) for the same process, but targets the DE10-Nano in particular, and provides more concrete examples.


## Preparation

Before building U-Boot, it is critical that the target QSYS (Platform Designer) system be generated.
Further, the system must be configured with the desired I/O pin settings, as these will be compiled into the preloader during the following process.

While not required for the compilation process, a compiled device tree (i.e. a `.dtb` file) for the target system will be needed later in this guide.
It may be helpful to prepare this while Platform Designer is still open and it is easy to find the addresses of any custom components.

In order to build U-Boot, we also need to obtain the U-Boot source code.
Do this by cloning the Altera U-Boot repository: `$ git clone https://github.com/altera-opensource/u-boot-socfpga.git`.


## Summary

- Rocketboards' setup tutorial
  - Note which names need to be changed
- Initial env var setup (ethaddr, save)
- Boot methods
  - MMC boot
  - Netboot with MMC fallback
