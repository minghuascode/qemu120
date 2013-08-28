#!/bin/sh
# -------------------------------------
#   Chromebook GNU/Linux USB generator
# -------------------------------------
#   http://www.chromebook-linux.com
#
#   by: Jordi <jordi@chromebook-linux.com>
#
# This script format and prepares an USB stick to boot GNU/Linux on Chromebook
#

#
# Just check with 'file' is the USB drive passed exists
#
CHECK_USBDRIVE="yes"

VERSION=`cat VERSION`
KERNEL_ARGS="config.txt"
CGPT_PATH="`pwd`/tools/cgpt"
VBUTIL_KERNEL="`pwd`/tools/vbutil_kernel"
TYPE_ROOTFS="ext2"
ONLY_SIGN="no"
FORMAT="yes"

#
# dd block size is set by default to 4096 to improve speed when copying
#

BS=4096

#
# Temporary mount point to mount the rootfs and copy the kernel modules
# (will be created and deleted, do not change)
#

TEMP_MOUNTPOINT="/mnt/chrolin"

#
# Functions
#

usage()
{
    echo "Usage: sudo sh $0 -u <usb drive> -i <root filesystem> [-t <root filsystem type>] [-b <dd block size>] [-k] [-f]\n"
    echo "\t-u  Set USB drive to copy GNU/Linux. Ex: /dev/sdg"
    echo "\t-i  Install the selected GNU/Linux root filesystem. Value: path with rootfs"
    echo "\t-t  Set type filesystem for the rootfs image. Optional. Default: ext2"
    echo "\t-b  Set the block size to dd the files. Optional. Default: 4096"
    echo "\t-k  Sign again the kernel and copy to KERN-A without installing the rootfs. Optional. Useful when change the kernel parameters"
    echo "\t-f  Don't format data on KERN-A and ROOT-A partitions. Optional\n"
    echo "Example: sudo sh $0 -u /dev/sdg -i builds/debian_rootfs-6.0.3.bin\n"
    exit 0
}

sign_kernel()
{   
    if [ $FORMAT = "yes" ];
    then
	echo "- Cleaning data on KERN-A $KERN_A..."
	dd if=/dev/zero of=$KERN_A bs=512 count=32768 || exit $?
    fi

    echo "- Signing kernel 3.4.0 with $KERNEL_ARGS..."
    if [ -f "kernel-3.4.0.bin.new" ];
    then
	rm kernel-3.4.0.bin.new
    fi
    $VBUTIL_KERNEL --repack kernel-3.4.0.bin.new --config $KERNEL_ARGS --signprivate kernel_data_key.vbprivk --oldblob kernel-3.4.0.bin || exit $?

    echo "- Copying kernel blob to KERN-A $KERN_A..."
    dd if=kernel-3.4.0.bin.new of=$KERN_A bs=$BS count=32768 || exit $?
}

menu_install()
{
    echo "- To install:"
    echo "\tGNU/Linux $GNULINUX"
    echo "\ton device: $USBDRIVE"
    echo "\twith kernel 3.4.0 (default Chrome OS)"
    echo "\tusing this command-line parameters:" `cat $KERNEL_ARGS`
    echo ""

    read -p "- Format device: $USBDRIVE and install $GNULINUX using this options. Please check kernel command-line parameters. Proceed? [y/n] " CONFIRM

    case $CONFIRM in
	[yY])
	  begin_install
	  ;;
	*) ;;
    esac
}

#
# Install GNU/Linux function
#

begin_install()
{
    CURRENT_SIZE=`stat -c %b $GNULINUX`
    HUMAN_SIZE=$(($CURRENT_SIZE / (1024 * 1024 * 2)))

    read -p "- Enter the size in GB to reserve for your filesystem. [default and minimum: $HUMAN_SIZE]: " NEW_SIZE 
    
    if [ -z $NEW_SIZE ];
    then
        NEW_SIZE=$HUMAN_SIZE
    fi

    if [ $NEW_SIZE -lt $HUMAN_SIZE ];
    then
        GNULINUX_SIZE=$(($HUMAN_SIZE * 1024 * 1024 * 2))
    else
        GNULINUX_SIZE=$(($NEW_SIZE * 1024 * 1024 * 2))
    fi

    NEW_HUMAN_SIZE=$(($GNULINUX_SIZE / (1024 * 1024 * 2)))
    
    echo "- Will reserve $NEW_HUMAN_SIZE GB of space for filesystem"

    echo "- Checking if $USBDRIVE is mounted... \c"
    mount | grep $USBDRIVE > /dev/null
    if [ $? -eq 0 ];
    then
	TO_UMOUNT=`mount | grep $USBDRIVE | awk '{ print $3 }'`
	echo "yes.\n- Trying to umount $TO_UMOUNT ($USBDRIVE)..."
	umount $TO_UMOUNT || exit $?
    else
	echo "no."
    fi

    echo "- Deleting current partition table on $USBDRIVE..."
    dd if=/dev/zero of=$USBDRIVE bs=512 count=1 || exit $?

    echo "- Creating GUID partition on $USBDRIVE..."
    $CGPT_PATH create $USBDRIVE || exit $?

    echo "- Setting protective old-style MBR on $USBDRIVE..."
    $CGPT_PATH boot -p $USBDRIVE || exit $?

    echo "- Scanning partitions on $USBDRIVE..."
    blockdev --rereadpt $USBDRIVE || exit $?

    echo "- Creating Chrome OS KERN-A..."
    $CGPT_PATH add -b 512 -s 32768 -t kernel -P 1 -S 1 -l KERN-A $USBDRIVE || exit $?
    
    echo "- Creating rootfs ROOT-A $ROOT_A... "
    $CGPT_PATH add -b 33792 -s $GNULINUX_SIZE -t rootfs -P 1 -S 1 -l ROOT-A $USBDRIVE || exit $?

    echo "- Rescanning partitions on $USBDRIVE... "
    blockdev --rereadpt $USBDRIVE || exit $?

    $CGPT_PATH show $USBDRIVE || exit $?

    sign_kernel

    if [ $FORMAT = "yes" ];
    then
	echo "- Cleaning data on ROOT-A $ROOT_A (formatting $NEW_HUMAN_SIZE GB of space)..."
	echo "(This may take a while... please don't remove your USB stick)"
        # 1024 * 1024 * 1024 = number of bytes in GB
        COUNT=$(($NEW_HUMAN_SIZE * 1024 * 1024 * 1024 / $BS))
        #echo "count calculated as: $COUNT"
	dd if=/dev/zero of=$ROOT_A bs=$BS count=$COUNT || exit $?
    fi

    echo "- Copying GNU/Linux root filesystem $GNULINUX to ROOT-A $ROOT_A (copying $HUMAN_SIZE GB of data)..."
    echo "(This may take a while... please don't remove your USB stick)"
    dd if=$GNULINUX of=$ROOT_A bs=$BS count=$CURRENT_SIZE || exit $?	

    echo "- Copying 3.4.0 kernel modules to $ROOT_A /lib/modules..."    
    if [ ! -d $TEMP_MOUNTPOINT ];
    then
	mkdir -p $TEMP_MOUNTPOINT || exit $?
    fi
    mount -t $TYPE_ROOTFS $ROOT_A $TEMP_MOUNTPOINT || exit $?
    cp -fr 3.4.0 $TEMP_MOUNTPOINT/lib/modules/. || exit $?

    umount $TEMP_MOUNTPOINT
    rmdir $TEMP_MOUNTPOINT

    echo "- Checking ROOT-A ($ROOT_A) filesystem..."
    e2fsck -fp $ROOT_A

    echo "- Extending ROOT-A ($ROOT_A) filesystem..."
    resize2fs -p $ROOT_A

    echo "\n\n- All done! Your USB stick with GNU/Linux is ready!\n"
}

##
## Process
##

echo "\n-- Chromebook Linux USB $VERSION --\n\thttp://www.chromebook-linux.com\n"

#
# Check at least 3 arguments or show the help
#

if [ $# -lt 3 ];
then
    usage
fi

#
# Parse arguments
#

while [ $# -gt 0 ]; 
do
    case $1 in
	-u) shift
	    USBDRIVE=$1
	    KERN_A=$1""1
	    ROOT_A=$1""2
	    ;;
	-i) shift
	    GNULINUX=$1
	    ;;
	-t) shift
	    TYPE_ROOTFS=$1
	    ;;
	-b) shift
	    BS=$1
	    ;;
	-k) ONLY_SIGN="yes"
	    ;;
	-f) FORMAT="no"
	    ;;
	 *) echo "\nError: Argument $1 doesn't exist\n"
	    usage
	    ;;
    esac
    shift
done

#
# We need root privileges to do some actions
#

if [ `whoami` != root ];
    then
	echo "Please run this script as root or using 'sudo'\n"
	exit 0
    fi

#
# Simple security check to verify that we're not formatting our hard disk
#

if [ $USBDRIVE = "/dev/sda" -o $USBDRIVE = "/dev/hda" ];
then
    echo "Oops..! $USBDRIVE doesn't look like an USB drive... you don't really want to format your hard disk, right? :)\n"
    exit 1
fi

#
# Checking if exists some required files
#

if [ ! -f $GNULINUX ];
then
    echo "The file: $GNULINUX doesn't exists.\nPlease download a prepared GNU/Linux image for Chromebook on http://www.chromebook-linux.com\n"
    exit 1
fi

if [ ! -f $KERNEL_ARGS ];
then
    echo "$KERNEL_ARGS file not found! You need this file with kernel command-line arguments inside\n"
    exit 1
fi

#
# Check if the USBDRIVE exists
#

if [ $CHECK_USBDRIVE = "yes" ];
then
    file -s $USBDRIVE || exit $?
fi

#
# Do the real work
# 

if [ $ONLY_SIGN = "yes" ];
then
    sign_kernel
    echo "\n\n- Done! Your kernel is ready on $KERN_A\n"
else
    menu_install
fi

echo ""
exit 0
