SHARE_DIR_NAME="../share_ext4"

rm -rv $SHARE_DIR_NAME
sudo modprobe loop
sudo losetup loop0 ../ext4_rootfs.img
mkdir $SHARE_DIR_NAME
sudo mount -t ext4 --source /dev/loop0 --target $SHARE_DIR_NAME
