#!/bin/sh
#
# File: update.sh
#
# Brief: Update the QBoxHD's NOR flash and USB flash-based filesystem.
#		 The NOR flash contains the bootloader, kernel and initramfs.
#
# Copyright (c) 2009-2010 Duolabs Spa
#
# Author: Pedro Aguilar (pedro@duolabs.com)
#
# Changelog
# Date    Author      Comments
# ------------------------------------------------------------------------------
# 0902    paguilar    v1.0 Original
# 0907    paguilar    v1.1 Force 4K block size when creating the ext3 partition
# 0907    paguilar    v1.2 Add support for installing settings, 
# 090928  paguilar    v1.3 Rename the dir qboxhd_udpate after the update 
#					  procedure. Remove the cpio verbose flag
# 091026  paguilar    v1.4 Added support for updating the NOR flash
# 091030  paguilar    v1.5 Create a file that rcS will find and reboot when the
#					  NOR has been updated
# 100209  paguilar    v1.6 Substitute flash_eraseall and dd by flashcp 
#					  when writing the NOR flash
# 100225  paguilar    v1.7 It seems that sometimes the erase done by flashcp is 
#					  not working. Use flash_eraseall and flashcp (erase will be
#					  done twice, but first it'll be of the whole mtd partition,
#					  then only the size of the file that will be written to). 
# 100413  paguilar    v1.8 If the board is a QBoxHD mini, the filesystem's label
#					  must be QBOX_HD_MINI
# 100421  paguilar    v1.9 Improved mechanism for displaying images
# 100524  paguilar    v1.10 Use /proc/stb/info/model for gathering information
#					  about the board
# 100604  paguilar    v1.11 Update NOR flash only if the md5 checksum of the
#					  given file in flash differs from the md5 checksum of the
#					  file in the update. This uses the new applet mtd_md5sum.
# 100715  paguilar    v1.12 Check the correctness of the updated files in the
#					  NOR flash. Try 3 times for each file before skipping it.
# 100813  paguilar    v1.13 Send to /dev/null cpio's stderr
# 100906  paguilar    v1.14 If mtd_md5sum is not present, write to NOR without 
#					  checking. Remove a double write of the initramfs
# 100920  paguilar    v1.15 Reset if the NOR flash was updated
# 101004  paguilar    v1.16 Do not upgrade if we're trying to update a different 
#					  board model 
# 141004  paguilar    v1.17 Rename qboxhd_update_1 to qboxhd_update
# 101804  paguilar    v1.18 sync after copying. Remove deprecated code.
# 110117  paguilar    v1.19 Display images to video output with df_showimage.
# 110502  devel  	  v1.20 Wait a few seconds to search the USB keys and added the initial sync command.


# Ash doesn't support arrays, so we have to check manually if the devs exist. 
# Warning: We only check 2 devs: the update/backup drive that was just 
#		   inserted and the to-be-updated drive. 
#		   We assume that they must be in sd[a-b], their order is irrelevant.
search_dev ()
{
	if [ "$1" == "/dev/sda" ]; then
		SDA=1
	elif [ "$1" == "/dev/sdb" ]; then
		SDB=1
	else 
		SDX=$(( SDX + 1 ))
	fi
}

PACKAGE=qboxhd

#sync for compressed file
sync

BOARD=$1
if [ "$BOARD" != "qboxhd-mini" -a "$BOARD" != "qboxhd" ]; then
	# For backwards compatibility with initramfs v1.14 and older
	insmod /mnt/update_fs/qboxhd_update/nor/qboxhdinfo_proc.ko
	BOARD=$(cat /proc/stb/info/model)
fi

# Abort if we're trying to update a different board model
hw_id=$(cat /proc/stb/info/id | sed 's/\(^[0-f][0-f]\).*/\1/')
if [ "$hw_id" == "00" -a "$PACKAGE" != "qboxhd" -o "$hw_id" == "01" -a "$PACKAGE" != "qboxhd-mini" ]; then
	echo " "
	echo "********************************************************************************"
	echo "FATAL: This board is a '$BOARD', but the update package seems to be"
	echo "       for the '$PACKAGE'. Aborting update procedure!"
	exit 1
fi

if [ "$BOARD" == "qboxhd-mini" ]; then
	DISPLAY_IMAGE=/usr/bin/display_image_mini
	IMG="_mini"
else
	DISPLAY_IMAGE=/usr/bin/display_image
	IMG=""
fi


echo " "
echo "****************************"
echo "  Update Software v1.20"
echo "****************************"

###
### Update the NOR flash
###

nor_update=0
max_flash_rewrites=3

df_showimage /etc/images/update_flash_720x576.jpg
$DISPLAY_IMAGE /etc/images/update_flash$IMG.bin

if [ -e /usr/bin/mtd_md5sum ]; then
	if [ -e /mnt/update_fs/qboxhd_update/nor/uboot ]; then
		md5_update_file=$(md5sum /mnt/update_fs/qboxhd_update/nor/uboot | awk -F' ' '/\d+/ { print $1 }')
		file_size=$(ls -l /mnt/update_fs/qboxhd_update/nor/uboot | awk -F' ' '/\d+/ { print  $5 }')
		md5_flash_file=$(mtd_md5sum if=/dev/mtd2 length=$file_size)
		if [ "$md5_update_file" != "$md5_flash_file" ]; then
			echo "Update: New bootloader found. Updating..."
			cnt=0
			while [[ "$cnt" -lt "$max_flash_rewrites" ]]; do
				/usr/sbin/flash_eraseall /dev/mtd2
				/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/uboot /dev/mtd2

				md5_done_file=$(mtd_md5sum if=/dev/mtd2 length=$file_size)
				if [ "$md5_done_file" == "$md5_update_file" ]; then
					break
				fi

				echo "Update: WARNING: Could not write bootloader to flash. Attempt $((cnt + 1))"
				cnt=$((cnt + 1))
			done

			if [ "$cnt" -eq "$max_flash_rewrites" ]; then
				$DISPLAY_IMAGE /etc/images/corrupted_file$IMG.bin
				echo "Update: ERROR: Could not write bootloader to flash. Skipping..."
				echo "Update: Do NOT power-off. The update procedure will try to continue..."
				sleep 10
			fi

			nor_update=$(( nor_update + 1 ))
		else
			echo "Update: Same bootloader found. Skipping..."
		fi
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/logo ]; then
		md5_update_file=$(md5sum /mnt/update_fs/qboxhd_update/nor/logo | awk -F' ' '/\d+/ { print $1 }')
		file_size=$(ls -l /mnt/update_fs/qboxhd_update/nor/logo | awk -F' ' '/\d+/ { print  $5 }')
		md5_flash_file=$(mtd_md5sum if=/dev/mtd3 length=$file_size)
		if [ "$md5_update_file" != "$md5_flash_file" ]; then
			echo "Update: New boot logo found. Updating..."
			cnt=0
			while [[ "$cnt" -lt "$max_flash_rewrites" ]]; do
				/usr/sbin/flash_eraseall /dev/mtd3
				/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/logo /dev/mtd3

				md5_done_file=$(mtd_md5sum if=/dev/mtd3 length=$file_size)
				if [ "$md5_done_file" == "$md5_update_file" ]; then
					break
				fi

				echo "Update: WARNING: Could not write logo to flash. Attempt $((cnt + 1))"
				cnt=$((cnt + 1))
			done

			if [ "$cnt" -eq "$max_flash_rewrites" ]; then
				$DISPLAY_IMAGE /etc/images/corrupted_file$IMG.bin
				echo "Update: ERROR: Could not write logo to flash. Skipping..."
				echo "Update: Do NOT power-off. The update procedure will try to continue..."
				sleep 10
			fi

			nor_update=$(( nor_update + 1 ))
		else
			echo "Update: Same boot logo found. Skipping..."
		fi
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/bs ]; then
		md5_update_file=$(md5sum /mnt/update_fs/qboxhd_update/nor/bs | awk -F' ' '/\d+/ { print $1 }')
		file_size=$(ls -l /mnt/update_fs/qboxhd_update/nor/bs | awk -F' ' '/\d+/ { print  $5 }')
		md5_flash_file=$(mtd_md5sum if=/dev/mtd4 length=$file_size)
		if [ "$md5_update_file" != "$md5_flash_file" ]; then
			echo "Update: New bs found. Updating..."
			cnt=0
			while [[ "$cnt" -lt "$max_flash_rewrites" ]]; do
				/usr/sbin/flash_eraseall /dev/mtd4
				/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/bs /dev/mtd4

				md5_done_file=$(mtd_md5sum if=/dev/mtd4 length=$file_size)
				if [ "$md5_done_file" == "$md5_update_file" ]; then
					break
				fi

				echo "Update: WARNING: Could not write bs to flash. Attempt $((cnt + 1))"
				cnt=$((cnt + 1))
			done

			if [ "$cnt" -eq "$max_flash_rewrites" ]; then
				$DISPLAY_IMAGE /etc/images/corrupted_file$IMG.bin
				echo "Update: ERROR: Could not write bs to flash. Skipping..."
				echo "Update: Do NOT power-off. The update procedure will try to continue..."
				sleep 10
			fi

			nor_update=$(( nor_update + 1 ))
		else
			echo "Update: Same bs found. Skipping..."
		fi
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/uimage ]; then
		md5_update_file=$(md5sum /mnt/update_fs/qboxhd_update/nor/uimage | awk -F' ' '/\d+/ { print $1 }')
		file_size=$(ls -l /mnt/update_fs/qboxhd_update/nor/uimage | awk -F' ' '/\d+/ { print  $5 }')
		md5_flash_file=$(mtd_md5sum if=/dev/mtd5 length=$file_size)
		if [ "$md5_update_file" != "$md5_flash_file" ]; then
			echo "Update: New kernel image found. Updating..."
			cnt=0
			while [[ "$cnt" -lt "$max_flash_rewrites" ]]; do
				/usr/sbin/flash_eraseall /dev/mtd5
				/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/uimage /dev/mtd5

				md5_done_file=$(mtd_md5sum if=/dev/mtd5 length=$file_size)
				if [ "$md5_done_file" == "$md5_update_file" ]; then
					break
				fi

				echo "Update: WARNING: Could not write kernel image to flash. Attempt $((cnt + 1))"
				cnt=$((cnt + 1))
			done

			if [ "$cnt" -eq "$max_flash_rewrites" ]; then
				$DISPLAY_IMAGE /etc/images/corrupted_file$IMG.bin
				echo "Update: ERROR: Could not write kernel image to flash. Skipping..."
				echo "Update: Do NOT power-off. The update procedure will try to continue..."
				sleep 10
			fi

			nor_update=$(( nor_update + 1 ))
		else
			echo "Update: Same kernel found. Skipping..."
		fi
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/uinitramfs ]; then
		md5_update_file=$(md5sum /mnt/update_fs/qboxhd_update/nor/uinitramfs | awk -F' ' '/\d+/ { print $1 }')
		file_size=$(ls -l /mnt/update_fs/qboxhd_update/nor/uinitramfs | awk -F' ' '/\d+/ { print  $5 }')
		md5_flash_file=$(mtd_md5sum if=/dev/mtd6 length=$file_size)
		if [ "$md5_update_file" != "$md5_flash_file" ]; then
			echo "Update: New initramfs found. Updating..."
			cnt=0
			while [[ "$cnt" -lt "$max_flash_rewrites" ]]; do
				/usr/sbin/flash_eraseall /dev/mtd6
				/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/uinitramfs /dev/mtd6

				md5_done_file=$(mtd_md5sum if=/dev/mtd6 length=$file_size)
				if [ "$md5_done_file" == "$md5_update_file" ]; then
					break
				fi

				echo "Update: WARNING: Could not write initramfs to flash. Attempt $((cnt + 1))"
				cnt=$((cnt + 1))
			done

			if [ "$cnt" -eq "$max_flash_rewrites" ]; then
				$DISPLAY_IMAGE /etc/images/corrupted_file$IMG.bin
				echo "Update: ERROR: Could not write initramfs to flash. Skipping..."
				echo "Update: Do NOT power-off. The update procedure will try to continue..."
				sleep 10
			fi

			nor_update=$(( nor_update + 1 ))
		else
			echo "Update: Same initramfs found. Skipping..."
		fi
	fi
else
	# If we don't have mtd_md5sum, we just write to NOR without checking
	echo "Update: WARNING: The command mtd_md5sum was not found. Updating the whole NOR flash"

	if [ -e /mnt/update_fs/qboxhd_update/nor/uboot ]; then
		echo "Update: uboot found. Updating..."
		/usr/sbin/flash_eraseall /dev/mtd2
		/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/uboot /dev/mtd2
		nor_update=$(( nor_update + 1 ))
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/logo ]; then
		echo "Update: logo found. Updating..."
		/usr/sbin/flash_eraseall /dev/mtd3
		/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/logo /dev/mtd3
		nor_update=$(( nor_update + 1 ))
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/bs ]; then
		echo "Update: bs found. Updating..."
		/usr/sbin/flash_eraseall /dev/mtd4
		/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/bs /dev/mtd4
		nor_update=$(( nor_update + 1 ))
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/uimage ]; then
		echo "Update: uimage found. Updating..."
		/usr/sbin/flash_eraseall /dev/mtd5
		/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/uimage /dev/mtd5
		nor_update=$(( nor_update + 1 ))
	fi

	if [ -e /mnt/update_fs/qboxhd_update/nor/uinitramfs ]; then
		echo "Update: initramfs found. Updating..."
		/usr/sbin/flash_eraseall /dev/mtd6
		/usr/sbin/flashcp -v /mnt/update_fs/qboxhd_update/nor/uinitramfs /dev/mtd6
		nor_update=$(( nor_update + 1 ))
	fi
fi

if [ "$nor_update" -gt "0" ]; then
	echo "Update: Flash updated with $nor_update file(s)"
	if [ -e /usr/sbin/reset_st40 ]; then
		reset_st40=/usr/sbin/reset_st40
	elif [ -e /mnt/update_fs/qboxhd_update/nor/reset_st40 ]; then
		reset_st40=/mnt/update_fs/qboxhd_update/nor/reset_st40
	else
		echo "Update: WARNING: Reset cmd not found. The update will keep going"
	fi

	if [ "$reset_st40" != "" ]; then
		echo "Update: The system will reboot, the filesystem will be updated afterwards"
		$reset_st40
		if [ "$?" -ne "0" ]; then
			echo "Update: WARNING: Could not reset. The update will keep going"
		fi
	fi
fi

###
### Update the filesystem in the USB drive
###
df_showimage /etc/images/update_filesystem_720x576.jpg &
$DISPLAY_IMAGE /etc/images/update_filesystem$IMG.bin

#some USB key are a bit slow
echo "Wait 7 seconds to detect the usb keys"
sleep 7
sync

SDA=0
SDB=0
SDX=0

# Search all the /dev/sdM devices connected to the board
# discarding their partitions such as sdMN 
# M: [a-z]
# N: [0-9]
DEVS=$(ls /sys/block/sd* | awk -F: '/\/sys\/block/ { print $1 }')
#echo "DEVS: $DEVS"

for i in $DEVS
do
    sd_dev=$(echo "$i" | awk '/\/sys\/block\/sd[a-z]/')
	if [ "$sd_dev" != "" ]; then
		echo "Update: Device detected: $i"
		sd_dev_ch=$(echo "$sd_dev" | sed 's/\(\/sys\/block\/\)\(sd[a-z]\)/\/dev\/\2/')
		search_dev $sd_dev_ch
	fi
done

TOTAL_DEVS=$((SDA + SDB + SDX ))
#echo "TOTAL_DEVS: $TOTAL_DEVS, sda: $SDA, sdb: $SDB, sdX: $SDX"

if [ "$TOTAL_DEVS" -ne "2" -o "$SDA" -ne "1" -o "$SDB" -ne "1" ]; then
	echo " "
	echo "****************************"
	echo "FATAL: There must be only two USB flash drives,"
	echo "       Devices found: sda: $SDA, sdb: $SDB, unknown: $SDX"
	exit 1
fi

# Search the device where /mnt/update_fs is mounted
# We must format the OTHER device
MOUNTS=$(mount | awk -F' ' '/mnt\/update_fs/ { print $1 }')

if [ "$MOUNTS" == "/dev/sda1" -o "$MOUNTS" == "/dev/sda" ]; then
	BAD_DEV="sdb"
else
	BAD_DEV="sda"
fi

DEV_SIZE=$(fdisk /dev/$BAD_DEV -l | awk -F' ' '/ MB\,/ { print $3 }')
echo "Update: Formatting a $DEV_SIZE MB USB flash drive"
sfdisk /dev/$BAD_DEV << EOF
,,L
;
;
;
y
EOF

if [ "$DEV_SIZE" -ge "100000" ]; then
	echo "Update: This operation will take a long time because it's a large drive"
	echo "Update: Please be patient..."
fi

if [ "$BOARD" == "qboxhd-mini" ]; then
	LABEL="QBOX_HD_MINI"
else
	LABEL="QBOX_HD"
fi

GOOD_DEV="/dev/"$BAD_DEV"1"
echo "Update: Creating ext3 filesystem in $GOOD_DEV"
mkfs.ext3 -b 4096 -L $LABEL $GOOD_DEV

echo "Update: Mounting ext3 filesystem"
mount $GOOD_DEV /mnt/new_root

echo "Update: Installing files. Please wait..."
cd /mnt/new_root
cpio -i < /mnt/update_fs/qboxhd_update/rootfs.cpio 2> /dev/null
cd /

cd /mnt/update_fs/qboxhd_update
# Install user settings if settings.zip exists
if [ -e "settings.zip" ]; then
	echo "Update: Settings file found. Installing them. Please wait..."
	rm -rf settings
	mkdir -p settings
	cp settings.zip settings
	cd settings

	zip_list=$(unzip -l settings.zip)
	zip_dir=$(echo $zip_list| awk -F':[0-9]{2}\s+' '/:/ { print $2 }' | awk -F'\/' '/\// { print $1 }')
	unzip settings.zip
	cp -r "$zip_dir"/* /mnt/new_root/etc/enigma2
	mv /mnt/new_root/etc/enigma2/satellites.xml /mnt/new_root/etc/tuxbox

	cd ..
	rm -rf settings
else
	echo "Update: No settings file found"
fi

# Be sure that everything is written
sync

# TODO: Install add-ons. Inside an add-ons dir there must be one dir per add-on.
# Inside each add-on dir, there must be a script called install.sh that contains 
# the commands needed for installing that particular add-on.
cd /

# Rename the dir qboxhd_update to done_qboxhd_update to avoid further 
# updates if the user forgets to remove the USB drive
mv /mnt/update_fs/qboxhd_update /mnt/update_fs/done_qboxhd_update
if [ -e /mnt/update_fs/qboxhd_update_1 -a -d /mnt/update_fs/qboxhd_update_1 ]; then
	mv /mnt/update_fs/qboxhd_update_1 /mnt/update_fs/qboxhd_update
fi

echo "Update: Unmounting ext3 filesystem"
umount /mnt/new_root

echo "Update: Done!"

exit 0
