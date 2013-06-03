
search: rockchip linux source


RK3066 Linux Kernel Source Code
================================

Rockchip and the companies developing products based on their processors usually drag their feet to release the GPL source code. bq, a Spanish company, appears to be the first company to release the GPL source with RK3066 devices, more exactly their bq Edison and Edison 3G tablets.

They released the source as a tarball (GPL_Edison.tar.gz), but omegamoon imported the code to his github account, and added what’s necessary to build the kernel for MK808 mini PC. Please note that only the Linux kernel source code was released, and apparently the bootloader and no part of Android fall under the GPL license…

As the .config in the root directory is for MK808 at the time of writing, I gave it a try as follows:

git clone https://github.com/omegamoon/rockchip-rk30xx-mk808.git
cd rockchip-rk30xx-mk808
make -j 12 CROSS_COMPILE=arm-linux-gnueabihf- uImage
make -j 12 CROSS_COMPILE=arm-linux-gnueabihf- modules

and was able to build the code successfully on a Debian Wheezy 64-bit build machine. The first time it actually failed because uuencode utility was missing. If you encounter the same issue you can install it with apt-get:

sudo apt-get install sharutils

I did not try the kernel since I don’t have RK3066 hardware, but the guys at ArmTvTech appeared to be successful.

I always interesting to look in arch/arm/configs to see what configs are available for a given platform:

    rk30_sdk_defconfig – RK30 SDK board (Apparently a tablet board)
    rk3066_sdk_defconfig – RK3066 SDK board. Basically the same as above except a different gyroscope and a different WiFi (memory?) option are used.
    bqEdison_defconfig – This is the configuration file for bq Edison tablet

In the KConfig we can also find some other hardware (mobile phone boards), but no defconfig files are using those in the source code released:

    RK30 smart phone board (MACH_RK30_PHONE)
    RK30 smart phone loquat board (MACH_RK30_PHONE_LOQUAT)
    RK30 smart phone a22 board (MACH_RK30_PHONE_A22)

Since we now have the source code, and some people have started to work on a Linux port, but this will certainly take some, and don’t expect GPU acceleration ever on RK30xx Linux. And the lack of SD card boot capability really makes it a pain, since you’ll have to choose Linux over Android and vice versa. However, this kernel has already been useful for Android as some kernel modules have been build for MK802/UG802 (e.g. Bluetooth)

Read more: http://www.cnx-software.com/2012/11/04/rockchip-rk3066-rk30xx-processor-documentation-source-code-and-tools/#ixzz2V7x1q28I


RK3066 Tools
===============

There are also some tools available for RK29xx & RK3066 (rktools) mainly to modify ROMs which you can retrieve and build as follows:

git clone https://github.com/rk3066/rk-tools.git
cd rk-tools
sudo apt-get install libssl-dev libcrypto++-dev
make

This will generate 4 tools:

    afptool – Tool to unpack and pack the firmware files
    Command line:

        afptool -pack xxx update.img
        afptool -unpack update.img xxx
    img_maker – Tool to create rkimage files (and it seems to convert the old firmware format to the new firmware format)
    Command line: img_maker [-rk30|-rk29] [loader] [major version] [minor version] [subversion] [old image] [out image]
    img_unpack – A tool to unpack (new format) firmware images
    Command line: ./img_unpack <source> <destination>
    mkkrnlimg – Tool to pack and unpack Kernel images (Also part of omegamoon github account in binary form).
    Command line: ./mkkrnlimg [-a|-r] <input> <output>

Another tools call rkflashtool can be used to reflash the NAND. The source code is here, and it’s for RK29xx / RK28xx processor, but RK3066 modding instructions are available. “This tool uses a low level protocol supported by the internal bootloader of the RK processor. Because of that, this tool doesn’t need anything to be present on NAND flash, and can be used to successfully reflash bricked tablets”. See Arctablet for details.

Finally Romdump allows you to dump RK3066 ROM to an SD card, check Vondroid for details.


Read more: http://www.cnx-software.com/2012/11/04/rockchip-rk3066-rk30xx-processor-documentation-source-code-and-tools/#ixzz2V7xItrPB


