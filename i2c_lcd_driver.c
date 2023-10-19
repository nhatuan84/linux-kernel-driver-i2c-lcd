#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/uaccess.h>   
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/ioctl.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include "LiquidCrystal_I2C.h"
#include "ioctl_cmd.h"
#include "queue.h"

static char lcd_string[32] = "iotsharing.com";
static int lcd_backlight = 0;
static int lcd_home = 0;
static int lcd_clear = 0;

#define USE_THREAD

static void print_lcd(const char *str)
{
    if(lcd_string != str)
    {
        memset(lcd_string, 0, 32);
        memcpy(lcd_string, str, strlen(str));
    }
    clear();
    home();
    print(lcd_string, strlen(lcd_string));
    lcd_home = 0;
    lcd_clear = 0;
}

void change_backlight(void)
{
    clear();
    home();
    setBacklight(lcd_backlight);
    print_lcd(lcd_string);
}
void clear_lcd(void)
{
    clear();
    memset(lcd_string, 0, 32);
    print_lcd(lcd_string);
}

static void delay(int ms)
{
    msleep(ms);
}
static void delayMicroseconds(int us)
{
    usleep_range(us, us + 10);
}

#ifdef USE_THREAD
static struct task_struct *local_thread;

static int thread_function(void *pv);

static int init_thread(void)
{
    local_thread = kthread_create(thread_function, "input to thread", "thread");
    if(local_thread) 
    {
        wake_up_process(local_thread);
    } 
    else 
    {
        pr_err("Cannot create kthread\n");
        return -1;
    }
    return 0;
}

static int thread_function(void *pv)
{
    pr_err("create kthread %s\n", (char *)pv);
    while(!kthread_should_stop()) 
    {
        if(!queue_is_empty())
        {
            char *text = queue_get_node();
            pr_info("Executing Workqueue Function %s\n", text);
            print_lcd(text);
            kfree(text);
        }
    }
    return 0;
}

static void del_thread(void)
{
    kthread_stop(local_thread);
}

#else
struct my_work {
    struct work_struct workqueue;
    char* data;
};

struct my_work *work;

void workqueue_fn(struct work_struct *work);

int init_wq(void)
{
    work = kmalloc(sizeof(struct my_work), GFP_KERNEL);
    INIT_WORK(&work->workqueue, workqueue_fn);
    return 0;
}

void workqueue_fn(struct work_struct *workqueue)
{
    //struct my_work *work = container_of(workqueue, struct my_work, workqueue);
    //pr_info("Executing Workqueue Function %s\n", work->data);
    //kfree(work->data);
    while(!queue_is_empty())
    {
        char *text = queue_get_node();
        pr_info("Executing Workqueue Function %s\n", text);
        print_lcd(text);
        kfree(text);
    }
}
static void del_wq(void)
{
    kfree(work);
}

#endif

/////////////////
static dev_t lcd_dev = 0;
static struct class *lcd_dev_class;
static struct cdev lcd_cdev;
static ioctl_cmd_t ioctl_cmd;

static int      dev_open(struct inode *inode, struct file *file);
static int      dev_release(struct inode *inode, struct file *file);
static ssize_t  dev_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  dev_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = dev_read,
    .write          = dev_write,
    .open           = dev_open,
    .unlocked_ioctl = dev_ioctl,
    .release        = dev_release,
};

static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("open device\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    // print_lcd(lcd_string);
    // int len = strlen(lcd_string);
    // work->data = kmalloc(len+1, GFP_KERNEL);
    // memcpy(work->data, lcd_string, len);
    // work->data[len] = 0;
    queue_add_node(lcd_string);
    #ifndef USE_THREAD
        schedule_work(&work->workqueue);
    #endif
    pr_info("release device %s\n", lcd_string);
    return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    int l = strlen(lcd_string);
    pr_info("read device\n");
    copy_to_user(buf, lcd_string, l);
    if(*off < l)
    {
        *off += l;
        return l;
    }
    return 0;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    if(*off == 0)
    {
        memset(lcd_string, 0, 32);
    }
    if(len > 0 && (*off + len) <= 32)
    {
        copy_from_user(&lcd_string[*off], buf, len);
        *off += len;
        lcd_string[*off] = 0;
    }
    pr_info("write device\n");
    return len;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case IOCTL_WRITE:
            if (copy_from_user(&ioctl_cmd, (ioctl_cmd_t *)arg, sizeof(ioctl_cmd_t)))
            {
                return -EACCES;
            }
            if(ioctl_cmd.backlight >= 0)
            {
                lcd_backlight = ioctl_cmd.backlight;
                change_backlight();
            }
            if(ioctl_cmd.home > 0)
            {
                lcd_home = ioctl_cmd.home;
                home();
            }
            if(ioctl_cmd.clear > 0)
            {
                lcd_clear = ioctl_cmd.clear;
                clear_lcd();
            }
            pr_info("IOCTL_WRITE");
            break;
        case IOCTL_READ:
            ioctl_cmd.backlight = lcd_backlight;
            ioctl_cmd.home = lcd_home;
            ioctl_cmd.clear = lcd_clear;
            if( copy_to_user((ioctl_cmd_t *) arg, &ioctl_cmd, sizeof(ioctl_cmd)) )
            {
                return -EACCES;
            }
            pr_info("IOCTL_READ");
            break;
        default:
            break;
    }
    return 0;
}

/////////////////////
#define I2C_BUS_NR              1           
#define I2C_DEV_NAME            "lcd"     
#define I2C_DEV_ADDR            0x27   //try to move to device tree
 
static struct i2c_adapter *etx_i2c_adapter = NULL; 
static struct i2c_client  *etx_i2c_lcd = NULL;
static struct kobject *lcd_sys_kobj;

static int expanderWrite(u8 _data){
    int ret;
    _data = _data | getBacklight();
    ret = i2c_master_send(etx_i2c_lcd, &_data, 1);
    return ret;
}

static ssize_t sysfs_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
    pr_info("sys read %s\n", attr->attr.name);
    if (strcmp(attr->attr.name, "lcd_backlight") == 0)
    {
        return sprintf(buf, "%d\n", lcd_backlight);   
    }
    else if (strcmp(attr->attr.name, "lcd_home") == 0)
    {
        return sprintf(buf, "%d\n", lcd_home);
    }
    else if (strcmp(attr->attr.name, "lcd_clear") == 0)
    {
        return sprintf(buf, "%d\n", lcd_clear);
    }
    return -1;
}

static ssize_t sysfs_store(struct kobject *kobj, 
                struct kobj_attribute *attr,const char *buf, size_t count)
{
    int var;
    pr_info("sys write %s\n", attr->attr.name);
    
    sscanf(buf, "%du", &var);
    if(var >= 0)
    {
        if (strcmp(attr->attr.name, "lcd_backlight") == 0)
        {
            lcd_backlight = var;
            change_backlight();
        }
        else if (strcmp(attr->attr.name, "lcd_home") == 0)
        {
            lcd_home = var;
            if(lcd_home)
            {
                home();
            }
        }
        else if (strcmp(attr->attr.name, "lcd_clear") == 0)
        {
            lcd_clear = var;
            if(lcd_clear)
            {
                clear_lcd();
            }
        }
    }
    return count;
}

//echo 1 > /sys/kernel/lcd/lcd_backlight
struct kobj_attribute backlight_attr = __ATTR(lcd_backlight, 0660, sysfs_show, sysfs_store);
struct kobj_attribute home_attr = __ATTR(lcd_home, 0660, sysfs_show, sysfs_store);
struct kobj_attribute clear_attr = __ATTR(lcd_clear, 0660, sysfs_show, sysfs_store);

static struct attribute *attrs[] = {
	&backlight_attr.attr,
	&home_attr.attr,
	&clear_attr.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int i2c_lcd_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    assign_basic_funcs(delay, delayMicroseconds, expanderWrite);
    LiquidCrystal_I2C(0x27, 16, 2, LCD_5x8DOTS);
    begin();
    backlight();
    print_lcd("Hello, world!");
    pr_info("lcd probed\n");
    
    return 0;
}
 
static int i2c_lcd_remove(struct i2c_client *client)
{   
    clear();
    home();
    print("Bye!", strlen("Bye!"));
    
    pr_info("lcd removed\n");
    return 0;
}

//bus manage new client
static const struct i2c_device_id i2c_lcd_id[] = {
        { I2C_DEV_NAME, 0 },
        {} //terminated with a NULL entry!
};

MODULE_DEVICE_TABLE(i2c, i2c_lcd_id);
 
//create /dev/lcd
static struct i2c_driver i2c_lcd_driver = {
        .driver = {
            .name   = I2C_DEV_NAME,
            .owner  = THIS_MODULE,
        },
        .probe          = i2c_lcd_probe,
        .remove         = i2c_lcd_remove,
        .id_table       = i2c_lcd_id,
};
 
/*
** I2C Board Info strucutre
*/
static struct i2c_board_info i2c_lcd_info = {
        I2C_BOARD_INFO(I2C_DEV_NAME, I2C_DEV_ADDR)
    };

int init_dev(void)
{
    if((alloc_chrdev_region(&lcd_dev, 0, 1, "lcd_dev")) <0){
            pr_err("Cannot allocate major number\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(lcd_dev), MINOR(lcd_dev));
 
    /*Creating cdev structure*/
    cdev_init(&lcd_cdev, &fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&lcd_cdev, lcd_dev, 1)) < 0){
        pr_err("Cannot add the device to the system\n");
        unregister_chrdev_region(lcd_dev, 1);
        return -1;
    }
 
    /*Creating struct class*/
    if(IS_ERR(lcd_dev_class = class_create(THIS_MODULE, "lcd_dev_class"))){
        pr_err("Cannot create the struct class\n");
        unregister_chrdev_region(lcd_dev, 1);
        return -1;
    }
 
    /*Creating device*/
    if(IS_ERR(device_create(lcd_dev_class, NULL, lcd_dev, NULL, "lcd_dev"))){
        pr_err("Cannot create the Device 1\n");
        class_destroy(lcd_dev_class);
        return -1;
    }
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
}

int init_lcd(void)
{
    int ret = -1;
    etx_i2c_adapter     = i2c_get_adapter(I2C_BUS_NR);
    
    if( etx_i2c_adapter != NULL )
    {
        etx_i2c_lcd = i2c_new_client_device(etx_i2c_adapter, &i2c_lcd_info);
        
        if( etx_i2c_lcd != NULL )
        {
            i2c_add_driver(&i2c_lcd_driver);
            ret = 0;
        }
        
        i2c_put_adapter(etx_i2c_adapter);
        queue_init();
    }
    
    return ret;
}

int init_sys(void)
{
    int ret = -1;
    //reate the directory under /sys/kernel/
    lcd_sys_kobj = kobject_create_and_add("lcd", kernel_kobj);
	if (!lcd_sys_kobj)
    {
		return -ENOMEM;
    }
	/* Create the files associated with this kobject */
	ret = sysfs_create_group(lcd_sys_kobj, &attr_group);
	if (ret)
    {
		kobject_put(lcd_sys_kobj);
    }
    return ret;
}
static int __init i2c_lcd_driver_init(void)
{
    int ret = -1;
    ret = init_lcd();
    if(ret == 0)
    {
        ret = init_dev();
    }
    if(ret == 0)
    {
        ret = init_sys();
    }
    
#ifdef USE_THREAD
    init_thread();
#else
    init_wq();
#endif

    pr_info("driver inited\n");
    return ret;
}
 
static void __exit i2c_lcd_driver_exit(void)
{
    i2c_unregister_device(etx_i2c_lcd);
    i2c_del_driver(&i2c_lcd_driver);

    kobject_put(lcd_sys_kobj);

    device_destroy(lcd_dev_class, lcd_dev);
    class_destroy(lcd_dev_class);
    cdev_del(&lcd_cdev);
    unregister_chrdev_region(lcd_dev, 1);

#ifdef USE_THREAD
    del_thread();
#else
    del_wq();
#endif

    pr_info("driver removed\n");
}

//linux kernel management
module_init(i2c_lcd_driver_init);
module_exit(i2c_lcd_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("nha.tuan84@gmail.com");
MODULE_DESCRIPTION("I2C LCD");
MODULE_VERSION("0.1");