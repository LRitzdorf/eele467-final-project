// Linux Platform Device Driver for the ADC Controller for DE-Series Boards component

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
// ADC Controller device structure
//-----------------------------------------------------------------------
/**
 * struct adc_controller_dev - Private adc_controller device struct.
 * @miscdev: miscdevice used to create a char device for the adc_controller
 *           component
 * @base_addr: Base address of the adc_controller component
 * @lock: mutex used to prevent concurrent writes to the adc_controller
 *        component
 *
 * An adc_controller struct gets created for each adc_controller component in the
 * system.
 */
struct adc_controller_dev {
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
/**
 * tracked_state_t - Describes states tracked by this driver, in the form
 *                   {true, false, unknown}. Useful primarily for write-only
 *                   registers, whose value is initially unknown, but can be
 *                   tracked by the driver after their first write.
 */
typedef enum {
    TRACKSTATE_TRUE    = 1,
    TRACKSTATE_FALSE   = 0,
    TRACKSTATE_UNKNOWN = -1
} tracked_state_t;
/**
 * struct dev_reg_tracked_attribute - Struct to store attributes for registers
 *                                    tracked by this driver.
 * @attr: Normal device attribute struct
 * @state: Currently tracked state of the register. Should probably be
 *         initialized to the unknown state.
 */
struct dev_reg_tracked_attribute {
    struct device_attribute attr;
    tracked_state_t state;
};


//-----------------------------------------------------------------------
// REG0 Write: Update register write function store()
//-----------------------------------------------------------------------
/**
 * update_store() - Trigger an ADC update.
 * @dev: Device structure for the adc_controller component. This device struct
 *       is embedded in the adc_controller's platform device struct.
 * @attr: Unused.
 * @buf: Unused.
 * @size: The number of bytes being written. Unused.
 *
 * Return: The number of bytes stored. Always equal to number of bytes written.
 */
static ssize_t update_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t size)
{
    struct adc_controller_dev *priv = dev_get_drvdata(dev);

    // Parse the string we received as a bool
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    bool update;
    int ret = kstrtobool(buf, &update);
    if (ret < 0) {
        // kstrtobool returned an error
        return ret;
    }

    // Writing any value (even zero) to the update register triggers an update,
    // so if a falsy value is passed, we skip the write entirely.
    if (update) {
        iowrite32(1, priv->base_addr + REG_W_UPDATE_OFFSET);
    }
    // Write was succesful, so we return the number of bytes we "wrote".
    return size;
}


//-----------------------------------------------------------------------
// REG1 Write: Auto-Update register write function store()
//-----------------------------------------------------------------------
/**
 * auto_update_store() - Control ADC auto-update.
 * @dev: Device structure for the adc_controller component. This device struct
 *       is embedded in the adc_controller's platform device struct.
 * @attr: Unused.
 * @buf: Buffer that contains the auto-update value being written.
 * @size: The number of bytes being written.
 *
 * Return: The number of bytes stored.
 */
static ssize_t auto_update_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t size)
{
    struct adc_controller_dev *priv = dev_get_drvdata(dev);
    struct dev_reg_tracked_attribute *auto_update_reg_attr
        = container_of(attr, struct dev_reg_tracked_attribute, attr);

    // Parse the string we received as a bool
    // See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
    bool auto_update;
    int ret = kstrtobool(buf, &auto_update);
    if (ret < 0) {
        // kstrtobool returned an error
        return ret;
    }

    iowrite32(auto_update, priv->base_addr + REG_W_AUTO_UPDATE_OFFSET);
    auto_update_reg_attr->state = auto_update ? TRACKSTATE_TRUE : TRACKSTATE_FALSE;

    // Write was succesful, so we return the number of bytes we wrote.
    return size;
}

//-----------------------------------------------------------------------
// REG1 Read: Auto-Update register read function show()
//-----------------------------------------------------------------------
/**
 * auto_update_show() - Return the tracked auto-update value to user-space via
 *                      sysfs.
 * @dev: Unused.
 * @attr: Device attribute structure for the relevant register.
 * @buf: Buffer that gets returned to user-space.
 *
 * Return: The number of bytes read.
 */
static ssize_t auto_update_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    // Get the private adc_controller data out of the dev struct
    struct dev_reg_tracked_attribute *auto_update_reg_attr
        = container_of(attr, struct dev_reg_tracked_attribute, attr);

    tracked_state_t auto_update = auto_update_reg_attr->state;

    if (auto_update == TRACKSTATE_UNKNOWN) {
        return scnprintf(buf, PAGE_SIZE, "Unknown; write boolean value to set\n");
    }
    return scnprintf(buf, PAGE_SIZE, "%d\n", auto_update);
}


//-----------------------------------------------------------------------
// REGs 0-7 Read: Channel register read function show()
//-----------------------------------------------------------------------
/**
 * channel_show() - Return the channel reading to user-space via sysfs.
 * @dev: Device structure for the adc_controller component. This device struct
 *       is embedded in the adc_controller's platform device struct.
 * @attr: Device attribute structure for the relevant register.
 * @buf: Buffer that gets returned to user-space.
 *
 * Return: The number of bytes read.
 */
static ssize_t channel_show(struct device *dev,
    struct device_attribute *attr, char *buf)
{
    struct adc_controller_dev *priv = dev_get_drvdata(dev);
    struct dev_reg_kind_attribute *channel_reg_attr
        = container_of(attr, struct dev_reg_kind_attribute, attr);

    u32 reading = ioread32(priv->base_addr + channel_reg_attr->reg_offset);

    return scnprintf(buf, PAGE_SIZE, "0x%X\n", reading);
}


//-----------------------------------------------------------------------
// sysfs Attributes
//-----------------------------------------------------------------------
#define DEVICE_ATTR_RO_KIND(_name, _kind, _reg_offset) \
struct dev_reg_kind_attribute dev_attr_##_name = \
    { __ATTR(_name, 0444, _kind##_show, NULL), _reg_offset }
#define DEVICE_ATTR_TRACKED(_mode, _name, _init_state) \
struct dev_reg_tracked_attribute dev_attr_##_name = \
    { __ATTR_##_mode(_name), _init_state }
// Define sysfs attributes
static DEVICE_ATTR_WO(update);
static DEVICE_ATTR_TRACKED(RW, auto_update, TRACKSTATE_UNKNOWN);
static DEVICE_ATTR_RO_KIND(channel_0, channel, REG_R_CH0_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_1, channel, REG_R_CH1_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_2, channel, REG_R_CH2_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_3, channel, REG_R_CH3_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_4, channel, REG_R_CH4_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_5, channel, REG_R_CH5_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_6, channel, REG_R_CH6_OFFSET);
static DEVICE_ATTR_RO_KIND(channel_7, channel, REG_R_CH7_OFFSET);

// Create an attribute group so the device core can export the attributes for
// us.
static struct attribute *adc_controller_attrs[] = {
    &dev_attr_update.attr,
    &dev_attr_auto_update.attr.attr,
    &dev_attr_channel_0.attr.attr,
    &dev_attr_channel_1.attr.attr,
    &dev_attr_channel_2.attr.attr,
    &dev_attr_channel_3.attr.attr,
    &dev_attr_channel_4.attr.attr,
    &dev_attr_channel_5.attr.attr,
    &dev_attr_channel_6.attr.attr,
    &dev_attr_channel_7.attr.attr,
    NULL,
};
ATTRIBUTE_GROUPS(adc_controller);


//-----------------------------------------------------------------------
// File Operations read()
//-----------------------------------------------------------------------
/**
 * adc_controller_read() - Read method for the adc_controller char device
 * @file: Pointer to the char device file struct.
 * @buf: User-space buffer to read the value into.
 * @count: The number of bytes being requested.
 * @offset: The byte offset in the file being read from.
 *
 * Return: On success, the number of bytes written is returned and the offset
 *         @offset is advanced by this number. On error, a negative error value
 *         is returned.
 */
static ssize_t adc_controller_read(struct file *file, char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    loff_t pos = *offset;

    /* Get the device's private data from the file struct's private_data field.
     * The private_data field is equal to the miscdev field in the
     * adc_controller_dev struct. container_of returns the adc_controller_dev
     * struct that contains the miscdev in private_data.
     */
    struct adc_controller_dev *priv = container_of(file->private_data,
            struct adc_controller_dev, miscdev);

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
        pr_warn("adc_controller_read: unaligned access\n");
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
        pr_warn("adc_controller_read: nothing copied\n");
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
 * adc_controller_write() - Write method for the adc_controller char device
 * @file: Pointer to the char device file struct.
 * @buf: User-space buffer to read the value from.
 * @count: The number of bytes being written.
 * @offset: The byte offset in the file being written to.
 *
 * Return: On success, the number of bytes written is returned and the offset
 *         @offset is advanced by this number. On error, a negative error value
 *         is returned.
 */
static ssize_t adc_controller_write(struct file *file, const char __user *buf,
    size_t count, loff_t *offset)
{
    size_t ret;
    u32 val;

    loff_t pos = *offset;

    /* Get the device's private data from the file struct's private_data field.
     * The private_data field is equal to the miscdev field in the
     * adc_controller_dev struct. container_of returns the adc_controller_dev
     * struct that contains the miscdev in private_data.
     */
    struct adc_controller_dev *priv = container_of(file->private_data,
            struct adc_controller_dev, miscdev);

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
        pr_warn("adc_controller_write: unaligned access\n");
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
        pr_warn("adc_controller_write: nothing copied from user space\n");
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
 * adc_controller_fops - File operations supported by the adc_controller driver
 * @owner: The adc_controller driver owns the file operations; this ensures that
 *         the driver can't be removed while the character device is still in
 *         use.
 * @read: The read function.
 * @write: The write function.
 * @llseek: We use the kernel's default_llseek() function; this allows users to
 *          change what position they are writing/reading to/from.
 */
static const struct file_operations adc_controller_fops = {
    .owner = THIS_MODULE,
    .read = adc_controller_read,
    .write = adc_controller_write,
    .llseek = default_llseek,
};


//-----------------------------------------------------------------------
// Platform Driver Probe (Initialization) Function
//-----------------------------------------------------------------------
/**
 * adc_controller_probe() - Initialize device when a match is found
 * @pdev: Platform device structure associated with our adc_controller
 *        device; pdev is automatically created by the driver core based upon
 *        our adc_controller device tree node.
 *
 * When a device that is compatible with this adc_controller driver is found,
 * the driver's probe function is called. This probe function gets called by
 * the kernel when an adc_controller device is found in the device tree.
 */
static int adc_controller_probe(struct platform_device *pdev)
{
    struct adc_controller_dev *priv;
    int ret;

    /* Allocate kernel memory for the adc_controller device and set it to 0.
     * GFP_KERNEL specifies that we are allocating normal kernel RAM; see the
     * kmalloc documentation for more info. The allocated memory is
     * automatically freed when the device is removed.
     */
    priv = devm_kzalloc(&pdev->dev, sizeof(struct adc_controller_dev), GFP_KERNEL);
    if (!priv) {
        pr_err("Failed to allocate kernel memory for adc_controller\n");
        return -ENOMEM;
    }

    /* Request and remap the device's memory region. Requesting the region
     * makes sure nobody else can use that memory. The memory is remapped into
     * the kernel's virtual address space becuase we don't have access to
     * physical memory locations.
     */
    priv->base_addr = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(priv->base_addr)) {
        pr_err("Failed to request/remap platform device resource (adc_controller)\n");
        return PTR_ERR(priv->base_addr);
    }

    // Initialize the misc device parameters
    priv->miscdev.minor = MISC_DYNAMIC_MINOR;
    priv->miscdev.name = "adc_controller";
    priv->miscdev.fops = &adc_controller_fops;
    priv->miscdev.parent = &pdev->dev;
    priv->miscdev.groups = adc_controller_groups;

    // Register the misc device; this creates a char dev at /dev/adc_controller
    ret = misc_register(&priv->miscdev);
    if (ret) {
        pr_err("Failed to register misc device for adc_controller\n");
        return ret;
    }

    // Attach the adc_controller's private data to the platform device's
    // struct.
    platform_set_drvdata(pdev, priv);

    pr_info("adc_controller probed successfully\n");

    return 0;
}

//-----------------------------------------------------------------------
// Platform Driver Remove Function
//-----------------------------------------------------------------------
/**
 * adc_controller_remove() - Remove an adc_controller device.
 * @pdev: Platform device structure associated with our adc_controller device.
 *
 * This function is called when an adc_controller devicee is removed or the
 * driver is removed.
 */
static int adc_controller_remove(struct platform_device *pdev)
{
    // Get the adc_controller's private data from the platform device.
    struct adc_controller_dev *priv = platform_get_drvdata(pdev);

    // Deregister the misc device and remove the /dev/adc_controller file.
    misc_deregister(&priv->miscdev);

    pr_info("adc_controller removed successfully\n");

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
static const struct of_device_id adc_controller_of_match[] = {
    // NOTE: This .compatible string must be identical to the .compatible
    // string in the Device Tree Node for adc_controller
    { .compatible = "lr,adc_controller_de", },
    { }
};
MODULE_DEVICE_TABLE(of, adc_controller_of_match);

//-----------------------------------------------------------------------
// Platform Driver Structure
//-----------------------------------------------------------------------
/**
 * struct adc_controller_driver - Platform driver struct for the adc_controller
 *                                driver
 * @probe: Function that's called when a device is found
 * @remove: Function that's called when a device is removed
 * @driver.owner: Which module owns this driver
 * @driver.name: Name of the adc_controller driver
 * @driver.of_match_table: Device tree match table
 * @driver.dev_groups: adc_controller sysfs attribute group; this allows the
 *                     driver core to create the attribute(s) without race
 *                     conditions.
 */
static struct platform_driver adc_controller_driver = {
    .probe = adc_controller_probe,
    .remove = adc_controller_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "adc_controller",
        .of_match_table = adc_controller_of_match,
        .dev_groups = adc_controller_groups,
    },
};

/* We don't need to do anything special in module init/exit. This macro
 * automatically handles module init/exit.
 */
module_platform_driver(adc_controller_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Lucas Ritzdorf");  // Adapted from Ross Snider and Trevor Vannoy's Echo Driver
MODULE_DESCRIPTION("ADC Controller for DE-Series Boards driver");
MODULE_VERSION("1.0");
