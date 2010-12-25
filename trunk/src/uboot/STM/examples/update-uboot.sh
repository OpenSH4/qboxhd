#!/bin/sh

# Example script for doing a live update of uboot from
# a running linux Kernel use with caution.

# Path to u-boot file:
uboot=u-boot.bin
# mtd partion containing u-boot
ubootpart=/dev/mtd0
# path to save existing u-boot file
tmpfile=/dev/uboot.backup

ubootsize=`stat -c %s $uboot`

echo "Updating u-boot bootloader - do not turn off or remove power!"

echo "Saving u-boot backup" 
dd if=$ubootpart of=$tmpfile
if cmp $ubootpart $tmpfile ; then
  echo "Erasing flash"
  flash_unlock $ubootpart
  flash_eraseall $ubootpart
  echo "Copying u-boot"
  dd if=$uboot of=$ubootpart
  if cmp $ubootpart $uboot +${ubootsize} ; then
    echo "u-boot update done"
    flash_lock $ubootpart 0 1
    exit 0
  fi
  echo "Copy failed! - restoring old u-boot"
  echo "Erasing flash"
  flash_eraseall $ubootpart
  echo "restoring u-boot"
  dd if=$tmpfile of=$ubootpart
  if cmp $ubootpart $tmpfile ; then
    echo "u-boot restored"
    exit 1
  fi
  echo "u-boot dead!"
  exit 1
else
  echo "Backup of u-boot failed!"
  exit 1
fi


