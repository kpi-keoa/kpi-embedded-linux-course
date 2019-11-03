clear
sudo dmesg -C
sudo insmod ./.build/hivemod.ko devname=hive_char_dev_test buffsize=100
sleep 3s
sudo ./test_prog > ./tmp_log_file.txt
# dmesg
echo 
echo 
echo 
ls /dev | grep hive
echo 
echo 
echo 
sudo rmmod hivemod
echo 
echo 
dmesg
echo 
echo 
cat ./tmp_log_file.txt
rm ./tmp_log_file.txt
