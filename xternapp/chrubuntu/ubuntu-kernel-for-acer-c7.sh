#!/bin/bash
#https://gist.github.com/carletes/4674386#file-ubuntu-kernel-for-acer-c7-sh

set -x

#
# Grab verified boot utilities from ChromeOS.
#
mkdir -p /usr/share/vboot
mount -o ro /dev/sda3 /mnt
cp /mnt/usr/bin/vbutil_* /usr/bin
cp /mnt/usr/bin/dump_kernel_config /usr/bin
rsync -avz /mnt/usr/share/vboot/ /usr/share/vboot/
umount /mnt

#
# On the Acer C7, ChromeOS is 32-bit, so the verified boot binaries need a
# few 32-bit shared libraries to run under ChrUbuntu, which is 64-bit.
#
apt-get install libc6:i386 libssl1.0.0:i386

#
# Fetch ChromeOS kernel sources from the Git repo.
#
apt-get install git-core
cd /usr/src
git clone  https://git.chromium.org/git/chromiumos/third_party/kernel.git
cd kernel
git checkout origin/chromeos-3.4

#
# Configure the kernel
#
# First we patch ``base.config`` to set ``CONFIG_SECURITY_CHROMIUMOS``
# to ``n`` ...
cp ./chromeos/config/base.config ./chromeos/config/base.config.orig
sed -e \
  's/CONFIG_SECURITY_CHROMIUMOS=y/CONFIG_SECURITY_CHROMIUMOS=n/' \
  ./chromeos/config/base.config.orig > ./chromeos/config/base.config
./chromeos/scripts/prepareconfig chromeos-intel-pineview
#
# ... and then we proceed as per Olaf's instructions
#
yes "" | make oldconfig

#
# Build the Ubuntu kernel packages
#
apt-get install kernel-package
make-kpkg kernel_image kernel_headers

#
# Backup current kernel and kernel modules
#
tstamp=$(date +%Y-%m-%d-%H%M)
dd if=/dev/sda6 of=/kernel-backup-$tstamp
cp -Rp /lib/modules/3.4.0 /lib/modules/3.4.0-backup-$tstamp

#
# Install kernel image and modules from the Ubuntu kernel packages we
# just created.
#
dpkg -i /usr/src/linux-*.deb

#
# Extract old kernel config
#
vbutil_kernel --verify /dev/sda6 --verbose | tail -1 > /config-$tstamp-orig.txt
#
# Add ``disablevmx=off`` to the command line, so that VMX is enabled (for VirtualBox & Co)
#
sed -e 's/$/ disablevmx=off/' \
  /config-$tstamp-orig.txt > /config-$tstamp.txt

#
# Wrap the new kernel with the verified block and with the new config.
#
vbutil_kernel --pack /newkernel \
  --keyblock /usr/share/vboot/devkeys/kernel.keyblock \
  --version 1 \
  --signprivate /usr/share/vboot/devkeys/kernel_data_key.vbprivk \
  --config=/config-$tstamp.txt \
  --vmlinuz /boot/vmlinuz-3.4.0 \
  --arch x86_64

#
# Make sure the new kernel verifies OK.
#
vbutil_kernel --verify /newkernel

#
# Copy the new kernel to the KERN-C partition.
#
dd if=/newkernel of=/dev/sda6
