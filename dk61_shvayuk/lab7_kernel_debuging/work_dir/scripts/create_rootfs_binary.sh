rm ../binaries/initramfs.igz
cd ./../initramfs/
find . | cpio -H newc -o > ../binaries/initramfs.cpio
cat ./../binaries/initramfs.cpio | gzip > ./../binaries/initramfs.igz
rm ../binaries/initramfs.cpio
