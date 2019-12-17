 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/kernel.h>

 int rday_1;
 int rday_2 = 20;
 int rday_3 = 30;

 EXPORT_SYMBOL(rday_3);

 static int __init hi(void)
 {
     printk(KERN_INFO "Module gdb1 being loaded.n");
     return 0;
 }

 static void __exit bye(void)
 {
     printk(KERN_INFO "Module gdb1 being unloaded.n");
 }

 module_init(hi);
 module_exit(bye);

 MODULE_LICENSE("GPL");
 MODULE_DESCRIPTION("Module debugging with gdb."); 
