// Linux Platform Device Driver for the HPS_Multi_PWM component

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

//-----------------------------------------------------------------------
// DEFINE STATEMENTS
//-----------------------------------------------------------------------
#include "reg_offsets.h"


//-----------------------------------------------------------------------
// HPS_Multi_PWM device structure
//-----------------------------------------------------------------------
/**
 * struct hps_multi_pwm_dev - Private hps_multi_pwm device struct.
 * @miscdev: miscdevice used to create a char device for the hps_multi_pwm
 *           component
 * @base_addr: Base address of the hps_multi_pwm component
 * @lock: mutex used to prevent concurrent writes to the hps_multi_pwm
 *        component
 *
 * An hps_multi_pwm struct gets created for each hps_multi_pwm component in the
 * system.
 */
struct hps_multi_pwm_dev {
    struct miscdevice miscdev;
    void __iomem *base_addr;
    struct mutex lock;
};
/**
 * struct dev_reg_kind_attribute - Struct to store attributes for registers of
 *                                 a similar kind.
 * @attr: Normal device attribute struct
 * @reg_offset: Offset to the specific register being represented
 */
struct dev_reg_kind_attribute {
    struct device_attribute attr;
    unsigned int reg_offset;
};


//-----------------------------------------------------------------------
// REG0: Period register read function show()
//-----------------------------------------------------------------------
/**
 * period_show() - Return the period value to user-space via sysfs.
 * @dev: Device structure for the hps_multi_pwm component. This device struct
 *       is embedded in the hps_multi_pwm's device struct.
 * @attr: Unused.
 * @buf: Buffer that gets returned to user-space.
 *
 * Return: The number of bytes read.
 */
static ssize_t period_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    // Get the private hps_multi_pwm data out of the dev struct
    struct hps_multi_pwm_dev *priv = dev_get_drvdata(dev);

    u32 period = ioread32(priv->base_addr + REG_PERIOD_OFFSET);

    return scnprintf(buf, PAGE_SIZE, "0x%X\n", period);
}

//-----------------------------------------------------------------------
// REG0: Period register write function store()
//-----------------------------------------------------------------------
/**
 * period_store() - Store the period value.
 * @dev: Device structure for the hps_multi_pwm component. This device struct
 *       is embedded in the hps_multi_pwm's platform device struct.
 * @attr: Unused.
 * @buf: Buffer that contains the period value being written.
 * @size: The number of bytes being written.
 *
 * Return: The number of bytes stored.
 */
static ssize_t period_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t size)
{
    struct hps_multi_pwm_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a bool
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    u32 period;
    int ret = kstrtou32(buf, 0, &period);
    if (ret < 0) {
        // kstrtou32 returned an error
        return ret;
    }

    iowrite32(period, priv->base_addr + REG_PERIOD_OFFSET);

    // Write was succesful, so we return the number of bytes we wrote.
    return size;
}


//-----------------------------------------------------------------------
// REGs 1-3: Duty cycle register read function show()
//-----------------------------------------------------------------------
/**
 * duty_cycle_show() - Return the duty cycle value to user-space via sysfs.
 * @dev: Device structure for the hps_multi_pwm component. This device struct
 *       is embedded in the hps_multi_pwm's platform device struct.
 * @attr: Unused.
 * @buf: Buffer that gets returned to user-space.
 *
 * Return: The number of bytes read.
 */
static ssize_t duty_cycle_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct hps_multi_pwm_dev *priv = dev_get_drvdata(dev);
    struct dev_reg_kind_attribute *duty_cycle_reg_attr
        = container_of(attr, struct dev_reg_kind_attribute, attr);

    u32 duty_cycle = ioread32(priv->base_addr + duty_cycle_reg_attr->reg_offset);

    return scnprintf(buf, PAGE_SIZE, "0x%X\n", duty_cycle);
}

//-----------------------------------------------------------------------
// REGs 1-3: Duty cycle register write function store()
//-----------------------------------------------------------------------
/**
 * duty_cycle_store() - Store the duty cycle value.
 * @dev: Device structure for the hps_multi_pwm component. This device struct
 *       is embedded in the hps_multi_pwm's platform device struct.
 * @attr: Unused.
 * @buf: Buffer that contains the duty cycle value being written.
 * @size: The number of bytes being written.
 *
 * Return: The number of bytes stored.
 */
static ssize_t duty_cycle_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t size)
{
    struct hps_multi_pwm_dev *priv = dev_get_drvdata(dev);
    struct dev_reg_kind_attribute *duty_cycle_reg_attr
        = container_of(attr, struct dev_reg_kind_attribute, attr);

    // Parse the string we received as a u32
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    u32 duty_cycle;
    int ret = kstrtou32(buf, 0, &duty_cycle);
    if (ret < 0) {
        // kstrtou32 returned an error
        return ret;
    }

    iowrite32(duty_cycle, priv->base_addr + duty_cycle_reg_attr->reg_offset);

    // Write was succesful, so we return the number of bytes we wrote.
    return size;
}


//-----------------------------------------------------------------------
// sysfs Attributes
//-----------------------------------------------------------------------
#define DEVICE_ATTR_RW_KIND(_name, _kind, _reg_offset) \
struct dev_reg_kind_attribute dev_attr_##_name = \
    { __ATTR(_name, 0644, _kind##_show, _kind##_store), _reg_offset }
// Define sysfs attributes
static DEVICE_ATTR_RW(period);
static DEVICE_ATTR_RW_KIND(duty_cycle_1, duty_cycle, REG_DC1_OFFSET);
static DEVICE_ATTR_RW_KIND(duty_cycle_2, duty_cycle, REG_DC2_OFFSET);
static DEVICE_ATTR_RW_KIND(duty_cycle_3, duty_cycle, REG_DC3_OFFSET);

// Create an attribute group so the device core can export the attributes for
// us.
static struct attribute *hps_multi_pwm_attrs[] = {
    &dev_attr_period.attr,
    &dev_attr_duty_cycle_1.attr.attr,
    &dev_attr_duty_cycle_2.attr.attr,
    &dev_attr_duty_cycle_3.attr.attr,
    NULL,
};
ATTRIBUTE_GROUPS(hps_multi_pwm);


//-----------------------------------------------------------------------
// File Operations read()
//-----------------------------------------------------------------------
/**
 * hps_multi_pwm_read() - Read method for the hps_multi_pwm char device
 * @file: Pointer to the char device file struct.
 * @buf: User-space buffer to read the value into.
 * @count: The number of bytes being requested.
 * @offset: The byte offset in the file being read from.
 *
 * Return: On success, the number of bytes written is returned and the offset
 *         @offset is advanced by this number. On error, a negative error value
 *         is returned.
 */
static ssize_t hps_multi_pwm_read(struct file *file, char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    loff_t pos = *offset;

    /* Get the device's private data from the file struct's private_data field.
     * The private_data field is equal to the miscdev field in the
     * hps_multi_pwm_dev struct. container_of returns the hps_multi_pwm_dev
     * struct that contains the miscdev in private_data.
     */
    struct hps_multi_pwm_dev *priv = container_of(file->private_data,
            struct hps_multi_pwm_dev, miscdev);

    // Check file offset to make sure we are reading to a valid location.
    if (pos < 0) {
        // We can't read from a negative file position.
        return -EINVAL;
    }
    if (pos >= SPAN) {
        // We can't read from a position past the end of our device.
        return 0;
    }
    if ((pos % 0x4) != 0) {
        /* Prevent unaligned access. Even though the hardware technically
         * supports unaligned access, we want to ensure that we only access
         * 32-bit-aligned addresses because our registers are 32-bit-aligned.
         */
        pr_warn("hps_multi_pwm_read: unaligned access\n");
        return -EFAULT;
    }

    // If the user didn't request any bytes, don't return any bytes :)
    if (count == 0) {
        return 0;
    }

    // Read the value at offset pos.
    val = ioread32(priv->base_addr + pos);

    ret = copy_to_user(buf, &val, sizeof(val));
    if (ret == sizeof(val)) {
        // Nothing was copied to the user.
        pr_warn("hps_multi_pwm_read: nothing copied\n");
        return -EFAULT;
    }

    // Increment the file offset by the number of bytes we read.
    *offset = pos + sizeof(val);

    return sizeof(val);
}

//-----------------------------------------------------------------------
// File Operations write()
//-----------------------------------------------------------------------
/**
 * hps_multi_pwm_write() - Write method for the hps_multi_pwm char device
 * @file: Pointer to the char device file struct.
 * @buf: User-space buffer to read the value from.
 * @count: The number of bytes being written.
 * @offset: The byte offset in the file being written to.
 *
 * Return: On success, the number of bytes written is returned and the offset
 *         @offset is advanced by this number. On error, a negative error value
 *         is returned.
 */
static ssize_t hps_multi_pwm_write(struct file *file, const char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    loff_t pos = *offset;

    /* Get the device's private data from the file struct's private_data field.
     * The private_data field is equal to the miscdev field in the
     * hps_multi_pwm_dev struct. container_of returns the hps_multi_pwm_dev
     * struct that contains the miscdev in private_data.
     */
    struct hps_multi_pwm_dev *priv = container_of(file->private_data,
            struct hps_multi_pwm_dev, miscdev);

    // Check file offset to make sure we are writing to a valid location.
    if (pos < 0) {
        // We can't write to a negative file position.
        return -EINVAL;
    }
    if (pos >= SPAN) {
        // We can't write to a position past the end of our device.
        return 0;
    }
    if ((pos % 0x4) != 0) {
        /* Prevent unaligned access. Even though the hardware technically
         * supports unaligned access, we want to ensure that we only access
         * 32-bit-aligned addresses because our registers are 32-bit-aligned.
         */
        pr_warn("hps_multi_pwm_write: unaligned access\n");
        return -EFAULT;
    }

    // If the user didn't request to write anything, return 0.
    if (count == 0) {
        return 0;
    }

    mutex_lock(&priv->lock);

    ret = copy_from_user(&val, buf, sizeof(val));
    if (ret == sizeof(val)) {
        // Nothing was copied from the user.
        pr_warn("hps_multi_pwm_write: nothing copied from user space\n");
        ret = -EFAULT;
        goto unlock;
    }

    // Write the value we were given at the address offset given by pos.
    iowrite32(val, priv->base_addr + pos);

    // Increment the file offset by the number of bytes we wrote.
    *offset = pos + sizeof(val);

    // Return the number of bytes we wrote.
    ret = sizeof(val);

unlock:
    mutex_unlock(&priv->lock);
    return ret;
}


//-----------------------------------------------------------------------
// File Operations Supported
//-----------------------------------------------------------------------
/**
 * hps_multi_pwm_fops - File operations supported by the hps_multi_pwm driver
 * @owner: The hps_multi_pwm driver owns the file operations; this ensures that
 *         the driver can't be removed while the character device is still in
 *         use.
 * @read: The read function.
 * @write: The write function.
 * @llseek: We use the kernel's default_llseek() function; this allows users to
 *          change what position they are writing/reading to/from.
 */
static const struct file_operations hps_multi_pwm_fops = {
    .owner = THIS_MODULE,
    .read = hps_multi_pwm_read,
    .write = hps_multi_pwm_write,
    .llseek = default_llseek,
};


//-----------------------------------------------------------------------
// Platform Driver Probe (Initialization) Function
//-----------------------------------------------------------------------
/**
 * hps_multi_pwm_probe() - Initialize device when a match is found
 * @pdev: Platform device structure associated with our hps_multi_pwm
 *        device; pdev is automatically created by the driver core based upon
 *        our hps_multi_pwm device tree node.
 *
 * When a device that is compatible with this hps_multi_pwm driver is found,
 * the driver's probe function is called. This probe function gets called by
 * the kernel when an hps_multi_pwm device is found in the device tree.
 */
static int hps_multi_pwm_probe(struct platform_device *pdev)
{
    struct hps_multi_pwm_dev *priv;
    int ret;

    /* Allocate kernel memory for the hps_multi_pwm device and set it to 0.
     * GFP_KERNEL specifies that we are allocating normal kernel RAM; see the
     * kmalloc documentation for more info. The allocated memory is
     * automatically freed when the device is removed.
     */
    priv = devm_kzalloc(&pdev->dev, sizeof(struct hps_multi_pwm_dev), GFP_KERNEL);
    if (!priv) {
        pr_err("Failed to allocate kernel memory for hps_multi_pwm\n");
        return -ENOMEM;
    }

    /* Request and remap the device's memory region. Requesting the region
     * makes sure nobody else can use that memory. The memory is remapped into
     * the kernel's virtual address space becuase we don't have access to
     * physical memory locations.
     */
    priv->base_addr = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(priv->base_addr)) {
        pr_err("Failed to request/remap platform device resource (hps_multi_pwm)\n");
        return PTR_ERR(priv->base_addr);
    }

    // Initialize the misc device parameters
    priv->miscdev.minor = MISC_DYNAMIC_MINOR;
    priv->miscdev.name = "hps_multi_pwm";
    priv->miscdev.fops = &hps_multi_pwm_fops;
    priv->miscdev.parent = &pdev->dev;
    priv->miscdev.groups = hps_multi_pwm_groups;

    // Register the misc device; this creates a char dev at /dev/hps_multi_pwm
    ret = misc_register(&priv->miscdev);
    if (ret) {
        pr_err("Failed to register misc device for hps_multi_pwm\n");
        return ret;
    }

    // Attach the hps_multi_pwm's private data to the platform device's
    // struct.
    platform_set_drvdata(pdev, priv);

    pr_info("hps_multi_pwm probed successfully\n");

    return 0;
}

//-----------------------------------------------------------------------
// Platform Driver Remove Function
//-----------------------------------------------------------------------
/**
 * hps_multi_pwm_remove() - Remove an hps_multi_pwm device.
 * @pdev: Platform device structure associated with our hps_multi_pwm device.
 *
 * This function is called when an hps_multi_pwm devicee is removed or the
 * driver is removed.
 */
static int hps_multi_pwm_remove(struct platform_device *pdev)
{
    // Get the hps_multi_pwm's private data from the platform device.
    struct hps_multi_pwm_dev *priv = platform_get_drvdata(pdev);

    // Deregister the misc device and remove the /dev/hps_multi_pwm file.
    misc_deregister(&priv->miscdev);

    pr_info("hps_multi_pwm removed successfully\n");

    return 0;
}

//-----------------------------------------------------------------------
// Compatible Match String
//-----------------------------------------------------------------------
/* Define the compatible property used for matching devices to this driver,
 * then add our device id structure to the kernel's device table. For a device
 * to be matched with this driver, its device tree node must use the same
 * compatible string as defined here.
 */
static const struct of_device_id hps_multi_pwm_of_match[] = {
    // NOTE: This .compatible string must be identical to the .compatible
    // string in the Device Tree Node for hps_multi_pwm
    { .compatible = "lr,hps_multi_pwm", },
    { }
};
MODULE_DEVICE_TABLE(of, hps_multi_pwm_of_match);

//-----------------------------------------------------------------------
// Platform Driver Structure
//-----------------------------------------------------------------------
/**
 * struct hps_multi_pwm_driver - Platform driver struct for the hps_multi_pwm
 *                               driver
 * @probe: Function that's called when a device is found
 * @remove: Function that's called when a device is removed
 * @driver.owner: Which module owns this driver
 * @driver.name: Name of the hps_multi_pwm driver
 * @driver.of_match_table: Device tree match table
 * @driver.dev_groups: hps_multi_pwm sysfs attribute group; this allows the
 *                     driver core to create the attribute(s) without race
 *                     conditions.
 */
static struct platform_driver hps_multi_pwm_driver = {
    .probe = hps_multi_pwm_probe,
    .remove = hps_multi_pwm_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "hps_multi_pwm",
        .of_match_table = hps_multi_pwm_of_match,
        .dev_groups = hps_multi_pwm_groups,
    },
};

/* We don't need to do anything special in module init/exit. This macro
 * automatically handles module init/exit.
 */
module_platform_driver(hps_multi_pwm_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Lucas Ritzdorf");  // Adapted from Ross Snider and Trevor Vannoy's Echo Driver
MODULE_DESCRIPTION("hps_multi_pwm driver");
MODULE_VERSION("1.0");
