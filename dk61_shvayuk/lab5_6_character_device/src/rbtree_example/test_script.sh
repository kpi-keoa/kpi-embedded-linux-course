clear
sudo dmesg -C
sudo insmod ./build/rbtree_example_mod.ko
sleep 1s
echo 
echo 
echo 
sudo rmmod rbtree_example_mod
dmesg
