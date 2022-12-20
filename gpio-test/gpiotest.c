#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#define gpiotest_CNT	1
#define gpiotest_NAME	"gpiotest"

struct gpiotest_dev {
	dev_t devid;				/* 设备号 	 */
	struct cdev cdev;			/* cdev 	*/
	struct class *class;		/* 类 		*/
	struct device *device;		/* 设备 	 */
	struct device_node	*nd; 	/* 设备节点 */
	int major;					/* 主设备号 */
	void *private_data;			/* 私有数据 */
    struct iio_channel *chan;   /*adc通道*/
    struct delayed_work my_delayed_work;    /*延时队列*/
	int gpio;		/*gpio*/
};

static struct gpiotest_dev gpiotestdev;

static int gpiotest_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int gpiotest_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &gpiotestdev; /* 设置私有数据 */
	return 0;
}

static void read_func(struct work_struct *pwork)
{       
    
    schedule_delayed_work(&gpiotestdev.my_delayed_work, msecs_to_jiffies(1000));

}

static int gpiotest_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
	long err = 0;
	struct gpiotest_dev *dev = (struct gpiotest_dev *)filp->private_data;
	int inputvalue = gpio_get_value(gpiotestdev.gpio);
	err = copy_to_user(buf, &inputvalue, sizeof(int));
	return 0;
}

static int gpiotest_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *off)
{
	long err = 0;
	struct gpiotest_dev *dev = (struct gpiotest_dev *)filp->private_data;
	int outvalue;
	err = copy_from_user(&outvalue, buf,sizeof(int));
	gpio_direction_output(gpiotestdev.gpio, outvalue);
	return 0;
}

/* gpiotest操作函数 */
static const struct file_operations gpiotest_ops = {
	.owner = THIS_MODULE,
	.open = gpiotest_open,
	.read = gpiotest_read,
	.write = gpiotest_write,
	.release = gpiotest_release,
};

static int gpiotest_probe(struct platform_device *pdev)
{
    printk("gpiotest_probe!\n");
	int ret = 0;

	/* 1、构建设备号 */
	if (gpiotestdev.major) {
		gpiotestdev.devid = MKDEV(gpiotestdev.major, 0);
		register_chrdev_region(gpiotestdev.devid, gpiotest_CNT, gpiotest_NAME);
	} else {
		alloc_chrdev_region(&gpiotestdev.devid, 0, gpiotest_CNT, gpiotest_NAME);
		gpiotestdev.major = MAJOR(gpiotestdev.devid);
	}

	/* 2、注册设备 */
	cdev_init(&gpiotestdev.cdev, &gpiotest_ops);
	cdev_add(&gpiotestdev.cdev, gpiotestdev.devid, gpiotest_CNT);

	/* 3、创建类 */
	gpiotestdev.class = class_create(THIS_MODULE, gpiotest_NAME);
	if (IS_ERR(gpiotestdev.class)) {
		return PTR_ERR(gpiotestdev.class);
	}

	/* 4、创建设备 */
	gpiotestdev.device = device_create(gpiotestdev.class, NULL, gpiotestdev.devid, NULL, gpiotest_NAME);
	if (IS_ERR(gpiotestdev.device)) {
		return PTR_ERR(gpiotestdev.device);
	}

	gpiotestdev.private_data = pdev; /* 设置私有数据 */
	struct device_node *gpiotest_node = pdev->dev.of_node;/*取设备节点*/
	int gpio;
	enum of_gpio_flags flag;
	gpio = of_get_named_gpio_flags(gpiotest_node, "test-gpio", 0, &flag);
	if (!gpio_is_valid(gpio)) {
    	printk("test-gpio: %d is invalid\n", gpio); 
		return -ENODEV;
	}
	if (gpio_request(gpio, "test-gpio")) {
        printk("gpio %d request failed!\n", gpio);
        gpio_free(gpio);
        return -ENODEV;
	}
	gpiotestdev.gpio = gpio;
	int first_out = (flag == OF_GPIO_ACTIVE_LOW) ? 0:1;
   	gpio_direction_output(gpiotestdev.gpio, first_out);
	printk("Test gpio putout %d\n", gpio);

    INIT_DELAYED_WORK(&gpiotestdev.my_delayed_work, read_func);
    schedule_delayed_work(&gpiotestdev.my_delayed_work, msecs_to_jiffies(1000));
	return 0;
}

static int gpiotest_remove(struct platform_device *pdev)
{
	gpio_free(gpiotestdev.gpio);
	/* 删除设备 */
	cdev_del(&gpiotestdev.cdev);
	unregister_chrdev_region(gpiotestdev.devid, gpiotest_CNT);

	/* 注销掉类和设备 */
	device_destroy(gpiotestdev.class, gpiotestdev.devid);
	class_destroy(gpiotestdev.class);
	return 0;
}

/* 设备树匹配列表 */
static const struct of_device_id gpiotest_of_match[] = {
	{ .compatible = "Litchi,gpiotest"},
	{ /* Sentinel */ }
};

static struct platform_driver gpiotest_driver = {
	.probe = gpiotest_probe,
	.remove = gpiotest_remove,
	.driver = {
			.owner = THIS_MODULE,
		   	.name = "Litchi",
		   	.of_match_table = gpiotest_of_match, 
		   },
};
		   
static int __init gpiotest_init(void)
{
    printk("register platform \r\n");
    return platform_driver_register(&gpiotest_driver);
}

static void __exit gpiotest_exit(void)
{
    platform_driver_unregister(&gpiotest_driver);
    cancel_delayed_work_sync(&gpiotestdev.my_delayed_work);   
    printk("unregister platform \r\n");
}

module_init(gpiotest_init);
module_exit(gpiotest_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Litchi");