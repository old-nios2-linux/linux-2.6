#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>

#include <asm/nios2.h>

#define sw_led_MAJOR 125

static char sw_led_name[]="sw_led";

static int sw_led_open(struct inode* inode,struct file * file)
{
	printk("Enter sw_led_open, major :%d , minor : %d \n",MAJOR(inode->i_rdev),MINOR(inode->i_rdev));
	return 0;
}
static int sw_led_release(struct inode* inode,struct file * file)
{
	printk("Enter sw_led_release\n");
	return 0;
}

static 	ssize_t sw_led_read (struct file * file, char __user * buff, size_t len, loff_t * off)
{
	printk("Enter sw_led_read\n");
	unsigned int sw_data;
	sw_data=na_pio_switch->np_piodata;
	printk("sw_data=%d\n",sw_data);
	if(copy_to_user(buff,(char *)&sw_data,sizeof(int)))
		return -EFAULT;
	
//	na_led_red->np_piodata=sw_data;	
//	outl(sw_data,&(na_led_red->np_piodata));
	return len;
}

static 	ssize_t sw_led_write (struct file * file, const char __user * buff, size_t len, loff_t * off)
{

	unsigned int led_data;
	led_data=0;
	if(copy_from_user((char *)&led_data,buff,sizeof(int)))
		return -EFAULT;
	printk("sw_led_write got %d  from user App\n",led_data);

	na_pio_red_led->np_piodata=led_data;

	return len;

}

static struct file_operations sw_led_fops={
	owner:THIS_MODULE,
	open:sw_led_open,
	release:sw_led_release,
	read:sw_led_read,
	write:sw_led_write
	};
static int __init enter_module(void) 
{ 
	printk("<1>Enter enter_module\n");
	int err;		
	if((err=register_chrdev(sw_led_MAJOR,sw_led_name,&sw_led_fops))<0)
	{
		printk("register_chrdev fail!,error code = %d\n",err);
		return err;
	}
	return 0; 
}
static void __exit exit_module(void) 
{
	printk("<1>Enter exit_module,Exiting...\n");
	int res;	
	unregister_chrdev(sw_led_MAJOR,sw_led_name);
	printk("unregister_chrdev ,result code : %d\n",res);
 }
module_init(enter_module);
module_exit(exit_module);
MODULE_AUTHOR("dxzhang@ustc.edu");
MODULE_DESCRIPTION("Driver for sw_led");
MODULE_LICENSE("GPL");
