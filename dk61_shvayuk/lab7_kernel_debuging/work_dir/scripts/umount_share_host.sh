SHARE_DIR_NAME="../share_ext4"

sudo umount /dev/loop0
sudo losetup -d /dev/loop0
sudo rmmod loop
rm -r $SHARE_DIR_NAME
