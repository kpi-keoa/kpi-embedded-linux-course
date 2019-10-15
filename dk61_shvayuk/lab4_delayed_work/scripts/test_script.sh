clear
sudo dmesg -C
sudo insmod ./lab4_module.ko
sleep 3s
dmesg
echo 
echo 
echo 
sudo rmmod lab4_module
dmesg
