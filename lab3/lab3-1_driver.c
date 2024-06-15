#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>  //copy_to/from_user()
#include <linux/gpio.h>  //GPIO
#include <linux/timer.h>

//LED is connected to this GPIO
#define GPIO_1 (1)
#define GPIO_12 (12)
#define GPIO_16 (16)
#define GPIO_21 (21)
int GPIO[] = {1, 12, 16, 21};

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

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
	uint8_t gpio_state[4] = {0};
	//reading GPIO value
	for (int i = 0; i < 4; ++i) {
        gpio_state[i] = gpio_get_value(GPIO[i]);
    }
	//write to user
	len = 1;
	if( copy_to_user(buf, &gpio_state, len) > 0) {
		pr_err("ERROR: Not all the bytes have been copied to user\n");
	}
	for (int i = 0; i < 4; ++i) {
        pr_info("Read function : GPIO %d = %d \n", GPIO[i], gpio_state[i]);
    } 
	return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	uint8_t led_state;
	uint8_t rec_buf[10] = {0};

	if(copy_from_user(rec_buf, buf, len ) > 0) {
		pr_err("ERROR: Not all the bytes have been copied from user\n");
	}
	for (int i = 0; i < 4; ++i) {
    	led_state = (rec_buf[0] & (1 << i)) ? 1 : 0;
  	    gpio_set_value(GPIO[i], led_state);
  	}
	return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) < 0){
		pr_err("Cannot allocate major number\n");
		goto r_unreg;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
	/*Creating cdev structure*/
	cdev_init(&etx_cdev,&fops);
	/*Adding character device to the system*/
	if((cdev_add(&etx_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_del;
	}
	/*Creating struct class*/
	if((dev_class = class_create(THIS_MODULE,"etx_class")) == NULL){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}
	/*Creating device*/
	if((device_create(dev_class,NULL,dev,NULL,"etx_device")) == NULL){
		pr_err( "Cannot create the Device \n");
		goto r_device;
	}
	
	//Checking the GPIO is valid or not
	for (int i = 0; i < 4; ++i) {
        if(gpio_is_valid(GPIO[i]) == false){
			pr_err("GPIO %d is not valid\n", GPIO[i]);
			goto r_device;
		}
    }

	//Requesting the GPIO
	if(gpio_request(1,"GPIO_1") < 0){
		pr_err("ERROR: GPIO %d request\n", 1);
		goto r_gpio;
	}
	if(gpio_request(12,"GPIO_12") < 0){
		pr_err("ERROR: GPIO %d request\n", 12);
		goto r_gpio;
	}
	if(gpio_request(16,"GPIO_16") < 0){
		pr_err("ERROR: GPIO %d request\n", 1);
		goto r_gpio;
	}
	if(gpio_request(21,"GPIO_21") < 0){
		pr_err("ERROR: GPIO %d request\n", 21);
		goto r_gpio;
	}

	//configure the GPIO as output
	for (int i = 0; i < 4; ++i) {
        gpio_direction_output(GPIO[i], 0);
    }
	
	/* Using this call the GPIO 21 will be visible in /sys/class/gpio/
	** Now you can change the gpio values by using below commands also.
	** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
	** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
	** cat /sys/class/gpio/gpio21/value (read the value LED)
	**
	** the second argument prevents the direction from being changed.
	*/
	for (int i = 0; i < 4; ++i) {
        gpio_export(GPIO[i], false);
    }

	pr_info("Device Driver Insert...Done!!!\n");
	return 0;

r_gpio:
	for (int i = 0; i < 4; ++i) {
        gpio_free(GPIO[i]);
    }
r_device:
	device_destroy(dev_class,dev);
r_class:
	class_destroy(dev_class);
r_del:
	cdev_del(&etx_cdev);
r_unreg:
	unregister_chrdev_region(dev,1);
	return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
	
	for (int i = 0; i < 4; ++i) {
        gpio_unexport(GPIO[i]);
        gpio_free(GPIO[i]);
    }
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - GPIO Driver");
MODULE_VERSION("1.32");
