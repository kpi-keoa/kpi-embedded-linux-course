clear
sudo dmesg -C
sudo insmod ./lab3_module.ko num_threads=5 thread_inc_iterations=10000
sleep 2s
dmesg
echo 
echo 
echo 
sudo rmmod lab3_module
dmesg
