# Custom Device Drivers

These drivers are implemented as loadable, out-of-tree kernel modules.
They were designed for use with the [Altera SoC FPGA kernel](https://github.com/altera-opensource/linux-socfpga).


## Environment Variables

While working with device drivers, it is frequently helpful to have the `ARCH` and `CROSS_COMPILE` environment variables exported for `make` to pick up on.
To this end, a helper script is provided: [`arm_env.sh`](/linux/arm_env.sh).
This script can be executed directly or sourced, and will spawn a new subshell for compilation and related tasks.
Alternatively, you may export the appropriate `ARCH` and `CROSS_COMPILE` environment variables yourself.


## Kernel Setup

1. Clone the [Altera kernel repo](https://github.com/altera-opensource/linux-socfpga) and check out the appropriate branch.
   These drivers were developed for 6.1.38 LTS (i.e. the `socfpga-6.1.38-lts` branch), but should work more generally as well.
   > **Tip:**
   > The Linux kernel is quite large.
   > Fortunately, Git allows cloning a single branch, via the invocation `git clone <uri> -b <branch> --single-branch`!
   > This should reduce download time somewhat, and can be combined with a shallow clone (i.e. `--depth 1`) for greater effect.
1. Ensure your environment is correctly configured, as detailed [above](#environment-variables).
1. Configure the kernel.
   Obtain a default setup using `make ARCH=arm socfpga_defconfig`
1. Compile the kernel as normal, via `make -j$(nproc)`.
   This will consume significant CPU resources while compilation occurs.


## Driver Compilation

1. Ensure your environment is correctly configured, as detailed [above](#environment-variables).
1. Export the `KDIR` environment variable to point to your kernel source tree, if not the default (`~/linux-socfpga`).
1. Move into the desired driver's directory and run `make` to compile it.
1. Install the driver on the SoC system.
   This assumes the default rootfs location for EELE 467; be sure to change this if your rootfs is placed elsewhere.
   ```sh
   $ make \
       INSTALL_MOD_PATH=/srv/nfs/de10nano/ubuntu-rootfs \
       modules_install
   ```