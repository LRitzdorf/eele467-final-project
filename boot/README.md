# HPS Boot Configuration


## Rationale

Certain SoC tasks require more configuration than simply regenetaring QSYS (Platform Designer) components and re-synthesizing the FPGA system.
In particular, reconfiguring hard processor system (HPS) I/O pins requires that the "preloader" be recompiled.

The preloader is the second stage of the HPS boot sequence — it is executed by the first-stage boot code (which is stored in on-chip ROM), and is thus the earliest element of the boot sequence over which we have control.
Among other tasks, the preloader is responsible for configuring HPS I/O pin multiplexing, which controls connection points for certain peripherals, such as I2C and SPI modules.

This document explains and demonstrates the process of building the U-Boot bootloader and its associated preloader (known as the Secondary Program Loader, or SPL) for the DE10-Nano FPGA board with Cyclone V SoC.
It is based in large part on [RocketBoards' documentation](https://www.rocketboards.org/foswiki/Documentation/BuildingBootloaderCycloneVAndArria10) for the same process, but targets the DE10-Nano in particular, and provides more concrete examples.


## Preparation

> [!NOTE]
> The steps in this document were developed and tested in a Linux environment.
> As such, they should be followed in one as well.
> This could be a bare-metal Linux installation, or a virtual machine.

Before building U-Boot, it is critical that the target QSYS (Platform Designer) system be generated.
Further, the system must be configured with the desired I/O pin settings, as these will be compiled into the preloader during the following process.

While not required for the compilation process, a compiled device tree (i.e. a `.dtb` file) for the target system will be needed later in this guide.
It may be helpful to prepare this while Platform Designer is still open and it is easy to find the addresses of any custom components.

In order to build U-Boot, we also need to obtain the U-Boot source code.
Do this by cloning the Altera U-Boot repository: `$ git clone https://github.com/altera-opensource/u-boot-socfpga.git`.


## Compiling U-Boot

### Configuring U-Boot

Before actually compiling U-Boot, we must configure it for our DE10-Nano board.

First, set the cross-compiler with `$ export CROSS_COMPILE=arm-linux-gnueabihf-`.
Then, apply the default DE10-Nano configuration by running `$ make socfpga_de10_nano_defconfig`.
Further changes, as desired, can be made via `$ make menuconfig`, though none are necessary for our purposes.

### Applying Platform Settings

While the Altera U-Boot repository ships default HPS configurations, we wish to apply our own — in fact, this is the most common reason for us to compile U-Boot in the first place.

Ensure you have moved into the `u-boot-socfpga` directory, then extract settings from the target QSYS platform by running:
```sh
$ python arch/arm/mach-socfpga/cv_bsp_generator/cv_bsp_generator.py \
  -i <path to Quartus project>/hps_isw_handoff/soc_system_hps \
  -o board/terasic/de10-nano/qts
```
This will generate a set of header files (with names matching `*_config.h`) in the specified `qts` directory.
These contain our extracted platform settings, and will be integrated into U-Boot during the compile step.

> [!IMPORTANT]
> At the time of writing, the `cv_bsp_generator.py` script is slightly broken, and writes header files that contain incorrectly named macros.
> These will break compilation if not fixed.
> A [pull request](https://github.com/altera-opensource/u-boot-socfpga/pull/14) has been raised to fix this.

If the above pull request has been merged (or "closed as completed") when you read this, the script should work as-is; if not, further action is required.
We'll need to retrieve fixed scripts from the pull request, by running the following commands:
```sh
$ git fetch origin pull/14/head:fix-config-prefix
$ git restore --source fix-config-prefix -- arch/arm/mach-socfpga/cv_bsp_generator/
```
This will apply fixes to the relevant scripts, and the above Python command can be re-run to generate correctly named header macros.

### Compilation

With the required setup steps complete, we can now compile U-Boot for the DE10-Nano.
Before proceeding, ensure that the `CROSS_COMPILE` environment variable is still set.
Running `$ printenv CROSS_COMPILE` should display the compiler prefix set earlier; if it does not, re-run the earlier `export` command to set it again.

Move to the top level of the U-Boot repository (if not already there), and compile U-Boot with `$ make -j$(nproc)`.
This will take a few moments to complete, and may use a significant portion of your system's processing power.


## SD Card Imaging

### Image Creation

The generated U-Boot binary must be appropriately packaged into an SD card image for booting by the DE10-Nano's Cyclone V HPS.

Create an `sdcard` working directory for SD card setup in a convenient location (such as next to the `u-boot-socfpga` repository).
Copy the following files into the `sdcard` directory:
- RocketBoards' `make_sdimage_p3.py` script (included in this directory, or available [here](https://releases.rocketboards.org/2021.04/gsrd/tools/make_sdimage_p3.py))
- The `u-boot-with-spl.sfp` image (located at the top level of the U-Boot repo, after compilation)
- The boot script `boot-mmc.script` from this directory
Also, create a directory named `sdfs` within `sdcard`, and copy the following files into it:
- The compiled device tree blob (`.dtb` file) for your system, as mentioned in <#Preparation>
- The kernel image you wish to use (probably named `zImage`)
- The raw binary file (`.rbf`) for your FPGA system (generated using the **File > Convert Programming Files** menu option in Quartus)

Compile the MMC bootscript into a form readable by U-Boot:
```sh
$ mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d boot-mmc.script sdfs/boot.scr
```

And finally, generate the SD card image:
```sh
$ sudo python3 make_sdimage_p3.py -f \
    -P u-boot-with-spl.sfp,num=3,format=raw,size=1M,type=A2 \
    -P sdfs,num=1,format=fat32,size=100M \
    -P /srv/nfs/de10nano/ubuntu-rootfs/,num=2,format=ext3,size=4G \
    -s 5G -n sdcard.img
```
The resulting image will be saved as `sdcard.img`, and will be 5GB in size.

> [!TIP]
> In addition to the U-Boot binary and `sdfs` directory, we specified the path to our NFS network share.
> This means we've baked the network share's root filesystem into our SD card image as well, and can use it to boot without a network connection!
> This is covered in more detail below.

### Image Flashing

Flash `sdcard.img` to a microSD card for the DE10-Nano.
On Linux, this can be accomplished using the `dd` utility:
```sh
# dd if=sdcard.img of=/dev/<microSD card device> status=progress
```
On Windows, use your disk flashing utility of choice.
If unsure, [BalenaEtcher](https://etcher.balena.io) is an excellent option.


## First Boot Setup


## Summary

- Rocketboards' setup tutorial
  - Note which names need to be changed
- Initial env var setup (ethaddr, save)
- Boot methods
  - MMC boot
  - Netboot with MMC fallback
