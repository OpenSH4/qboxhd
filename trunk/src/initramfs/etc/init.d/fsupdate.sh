#!/bin/sh
#
# @brief Search and, if found, execute a recovery/update/backup in a USB flash device
#	 If it's a file, it must have the name qboxhd*.tbz 
#	 If it's a dir, it must have the name qboxhd*  where * is [-_a-zA-Z0-9]*
# 	 WARNING: We search in up to 4 USB flash devices
#
# Copyright (c) 2009-2010 Duolabs Spa
#
# Author: Pedro Aguilar (pedro@duolabs.com)
#

qboxhd_update()
{
	# If the update/backup file/dir exists, decompress it and execute its script
	if [ -e /mnt/update_fs/qboxhd$1.tbz -o -e /mnt/update_fs/qboxhd_update/update.sh -o -e /mnt/update_fs/qboxhd_backup/update.sh ]; then
		echo " "
		echo "Package found in $DEV"

		if [ -e /mnt/update_fs/qboxhd$1.tbz ]; then
			df_showimage /etc/images/update_extracting_720x576.jpg &
			$DISPLAY_IMAGE /etc/images/update_extracting$IMG.bin
			echo "Compressed update file found, decompressing it"
			echo "Please wait..."
			tar -xj -f /mnt/update_fs/qboxhd$1.tbz -C /mnt/update_fs/
			# Rename the tbz to avoid further updates if the user forgets to remove the USB drive
			mv /mnt/update_fs/qboxhd$1.tbz /mnt/update_fs/done_qboxhd$1.tbz
		fi

		if [ -e /mnt/update_fs/qboxhd_update/update.sh -o -e /mnt/update_fs/qboxhd_backup/update.sh ]; then
			echo "Script found, executing it"
			if [ -e /mnt/update_fs/qboxhd_update/update.sh ]; then
				/mnt/update_fs/qboxhd_update/update.sh $BOARD
			else
				/mnt/update_fs/qboxhd_backup/update.sh $BOARD
			fi
			if [ "$?" -eq "0" ]; then 
				df_showimage /etc/images/update_finished_720x576.jpg &
				$DISPLAY_IMAGE /etc/images/update_finished$IMG.bin
				umount /mnt/update_fs
				exit 0
			else
				df_showimage /etc/images/update_error_720x576.jpg &
				$DISPLAY_IMAGE /etc/images/update_error$IMG.bin
				umount /mnt/update_fs
				exit 1
			fi
		else
			df_showimage /etc/images/update_error_720x576.jpg &
			$DISPLAY_IMAGE /etc/images/update_error$IMG.bin
			umount /mnt/update_fs
			echo "WARNING: Could not find update script in $DEV"
			exit 1
		fi
	fi
}

qboxhd_any()
{
	cd /mnt/update_fs

	# Get the first qboxhd*.tbz file
	qboxhd_list=$(ls qboxhd*.tbz 2> /dev/null)
	qboxhd_elem=""
	for i in $qboxhd_list; do
		if [ -f $i ]; then
			echo "File found: $i"
			qboxhd_elem=$(echo "$i" | sed 's/\(qboxhd\)\([.-_a-zA-Z0-9]*\)\(\.tbz\)/\2/')
			#echo "File after removing prefix/suffix: $qboxhd_elem"
			break
		fi
	done

	cd - >/dev/null

	if [ "$qboxhd_elem" != "" ]; then
		qboxhd_update $qboxhd_elem
	fi
}

BOARD=$1
if [ "$BOARD" != "qboxhd-mini" -a "$BOARD" != "qboxhd" ]; then
    # For backwards comptability with initramfs v1.14 and older
    CPUINFO=$(cat /proc/cpuinfo | awk -F : '/machine/ { print $2 }' | sed 's/^ /\1/')
    if [ "$CPUINFO" == "QBoxHD mini board" ]; then
        BOARD="qboxhd-mini"
    elif [ "$CPUINFO" == "QBoxHD board" ]; then
        BOARD="qboxhd"
    else
        echo " "
        echo "********************************************************************************"
        echo "FATAL: Invalid board type '$BOARD'"
        exit 1
    fi  
fi

if [ "$BOARD" == "qboxhd-mini" ]; then
	# Get the rootfs device if it exists
	rootfs_dev=$(blkid | awk -F: '/LABEL="QBOX_HD_MINI"/ { print $1 }')
	rootfs_dev=$(echo $rootfs_dev | sed 's/\/dev\/\(sd[a-z]\)[1-9]/\1/')

    DISPLAY_IMAGE=/usr/bin/display_image_mini
	IMG="_mini"
else
	# Get the rootfs device if it exists
	rootfs_dev=$(blkid | awk -F: '/LABEL="QBOX_HD"/ { print $1 }')
	if [ "$rootfs_dev" = "" ]; then
		rootfs_dev=$(blkid | awk -F: '/LABEL="QBox_HD"/ { print $1 }')
	fi
	rootfs_dev=$(echo $rootfs_dev | sed 's/\/dev\/\(sd[a-z]\)[1-9]/\1/')

    DISPLAY_IMAGE=/usr/bin/display_image
	IMG=""
fi

for i in sda sdb sdc sdd
do
	echo "Searching update in $i"

	# Skip device if it is the rootfs
	if [ "$i" == "$rootfs_dev" ]; then
		continue
	fi

	# Skip device if it doesn't exist
	if [ ! -e "/dev/"$i"1" -a ! -e "/dev/"$i ]; then
		continue
	fi

	# rootfs can be in sdN1 or sdN
	if [ -e "/dev/"$i"1" ]; then
		DEV="/dev/"$i"1"
	else
		DEV="/dev/"$i
	fi

	# Continue if we couldn't mount the vfat drive
	mount -t vfat $DEV /mnt/update_fs 2> /dev/null
	if [ "$?" -ne "0" ]; then
		mount -t ext3 $DEV /mnt/update_fs 2> /dev/null
		if [ "$?" -ne "0" ]; then
			continue
		fi
	fi

	# Drop to a shell if this file is found
	if [ -e /mnt/update_fs/INITRAMFS_CONSOLE ]; then
		echo " "
    	echo "********************************************************************************"
		echo "WARNING: INITRAMFS_CONSOLE file found, dropping to a shell"
		df_showimage /etc/images/initramfs_console_found_720x576.jpg &
		$DISPLAY_IMAGE /etc/images/initramfs_console_found$IMG.bin
		/usr/bin/lcd_on_off_brightness &
		exec sh
	fi

	qboxhd_update _update
	#qboxhd_update _backup
	qboxhd_any

	umount /mnt/update_fs
done

echo "No update/backup found, executing normal booting procedure."
exit 0
