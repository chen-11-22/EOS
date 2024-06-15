#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/timer.h>

#define MAJOR_NUM 33
#define DEVICE_NAME "my_dev"

dev_t dev = 0;

char data16[17] = {0};
const char seg_for_c[27][17] = {"1111001100010001", "0000011100000101", 
"1100111100000000", 
"0000011001000101", 
"1000011100000001", 
"1000001100000001", 
"1001111100010000", 
"0011001100010001", 
"1100110001000100", 
"1100010001000100", 
"0000000001101100", 
"0000111100000000", 
"0011001110100000", 
"0011001110001000", 
"1111111100000000", 
"1000001101000001", 
"0111000001010000", 
"1110001100011001", 
"1101110100010001", 
"1100000001000100", 
"0011111100000000", 
"0000001100100010", 
"0011001100001010", 
"0000000010101010", 
"0000000010100100", 
"1100110000100010", 
"0000000000000000"};

static int my_init(void);
static void my_exit(void);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
/******************************************************/

//File operation structure
static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read = etx_read,
	.write = etx_write,
	.open = etx_open,
	.release = etx_release,
};

/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	
	len = 16;
	if( copy_to_user(buf, &data16, len) > 0) {
		pr_err("ERROR: Not all the bytes have been copied to user\n");
	}
	return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	char rec_buf[10] = {0};
	if(copy_from_user(rec_buf, buf, len ) > 0) {
		pr_err("ERROR: Not all the bytes have been copied from user\n");
	}
	for (int i = 0; i < 10; i++){
		pr_info("write!");
		if (rec_buf[i] == '\0'){
			break;
		}else{
			switch (rec_buf[i]){
			
				case 'a':
					strcpy(data16, seg_for_c[0]);
					break;
				case 'c':
					strcpy(data16, seg_for_c[2]);
					break;
				case 'h':
					strcpy(data16, seg_for_c[7]);
					break;
				case 'e':
					strcpy(data16, seg_for_c[4]);
					break;
				case 'n':
					strcpy(data16, seg_for_c[13]);
					break;
			}
			
		}
	}
	return len;
}

/*
** Module Init function
*/
static int my_init(void){
	printk("call init\n");
	if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops) < 0){
		printk("Can not get major %d\n", MAJOR_NUM);
		return (-EBUSY);
	}
	printk("My device is started and the major is %d\n", MAJOR_NUM);
	return 0;
}

static void my_exit(void){
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk("call exit\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
//MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
//MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
//MODULE_VERSION("1.32");
