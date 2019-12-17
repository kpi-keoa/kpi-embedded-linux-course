
qemu-system-x86_64 \
-kernel ../binaries/bzImage \
-initrd ../binaries/initramfs.igz \
-nographic \
-append "earlyprintk=serial,ttyS0 console=ttyS0,115200 nokaslr kgdboc=ttyS1,115200 kgdbcon" \
-s \
-drive file=../ext4_rootfs.img,cache=none,format=raw \
-net nic,model=rtl8139 -net bridge,br=brkvm
#-netdev user,id=vmnic,hostname=maxqemuvm -device virtio-net,netdev=vmnic
#-append "earlyprintk=serial,ttyS0 console=ttyS0,115200 nokaslr kgdboc=ttyS1,115200 kgdbwait kgdbcon " \
#-drive file=fat:rw:../share,cache=none
