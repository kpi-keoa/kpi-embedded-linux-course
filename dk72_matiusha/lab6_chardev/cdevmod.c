
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/debugfs.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/kdev_t.h>

MODULE_DESCRIPTION("Base64 chardev encoder|decoder");
MODULE_AUTHOR("raibu");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

static int major = 0;
module_param(major, int, 0);
MODULE_PARM_DESC(major, "Major number. Defaults to 0 (automatic allocation)");

static int max_length = 100;
module_param(max_length, uint, 0);
MODULE_PARM_DESC(max_length, "maximum length of encoded|decoded char sequence");

static bool enc_or_dec = false;
module_param(enc_or_dec, bool, 0);
MODULE_PARM_DESC(enc_or_dec, "!encode / decode");

typedef struct {
    uint max_length;
    bool enc_or_dec;
    char *buffer;
} b64params;

#define SET_MAXLENGTH _IOW('i', 0, uint)
#define SETMODE _IOW('i', 1, bool)

static const char *devname = THIS_MODULE->name;

static uint files_counter = 0;
static dev_t my_dev = 0;    // Stores our device handle
static struct cdev my_cdev;    // scull-initialized
static struct class *cl;

char *b64encode(unsigned char *src, size_t len);
char *b64decode(void *data, size_t len);
/**
 * cdev_open() - callback for open() file operation
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 *
 * 
 */
static int cdev_open(struct inode *inode, struct file *file)
{
    b64params *params = kzalloc(sizeof(*params), GFP_KERNEL);
    if (NULL == params) {
        pr_err("Params allocate failed for %016llx\n", (uint64_t)file);
        return -ENOMEM;
    }

    params->max_length = max_length;
    params->enc_or_dec = enc_or_dec;
    params->buffer = NULL;

    file->private_data = params;
    files_counter++;

    return 0;
}

/**
 * cdev_release() - file close() callback
 * @inode: information to manipulate the file (unused)
 * @file: VFS file opened by a process
 */
static int cdev_release(struct inode *inode, struct file *file)
{
    b64params *params = file->private_data;
    if (params->buffer != NULL) {
        kfree(params->buffer);
        params->buffer = NULL;
    }
    kfree(file->private_data);
    files_counter--;

    return 0;
}

/**
 * cdev_read() - called on file read() operation
 * @file: VFS file opened by a process
 * @buf:
 * @count:
 * @loff:
 */
static ssize_t cdev_read(struct file *file, char __user *buf, size_t count, loff_t *loff)
{
    b64params *params = file->private_data;
    unsigned long bytes_read = 0;

    char *output = NULL;    //for output
    switch (params->enc_or_dec) {
    case true:    //decode
        output = b64decode(params->buffer, strlen(params->buffer));
        break;
    case false:    //encode
    default:
        output = b64encode(params->buffer, strlen(params->buffer));
    }
    if (output == NULL) {
        return 0;
    }
    unsigned long n = strlen(output);
    bytes_read = copy_to_user(buf, output, n);

    kfree(output);
    output = NULL;
    return n - bytes_read;
}

static ssize_t cdev_write(struct file *file, const char __user *buf, size_t count, loff_t *loff)
{
    b64params *params = file->private_data;
    if (params->buffer != NULL) {
        kfree(params->buffer);
        params->buffer = NULL;
    }

    unsigned long write_fail = 0;
    unsigned long n = params->max_length < count ? params->max_length : count;

    params->buffer = kzalloc(n, GFP_KERNEL);

    write_fail = copy_from_user(params->buffer, buf, n);
    params->buffer[n - 1] = 0;

    return write_fail;
}

/**
 * my_ioctl - callback ioctl
 * @file:        file pointer
 * @cmd:   SETMODE
 * @arg:
 */
static long cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    b64params *params = file->private_data;

    if (_IOC_TYPE(cmd) != 'i')
        return -ENOTTY;
    if (_IOC_NR(cmd) > 1)
        return -ENOTTY;

    switch (cmd) {
    case SET_MAXLENGTH:

        params->max_length = (uint)arg;
        pr_info("New maximum length is %d\n", (uint)arg);

        break;
    case SETMODE:

        params->enc_or_dec = (bool)arg;
        pr_info("Mode set: %scode\n", ((bool)arg) ? "de" : "en");

        break;
    default:
        return -ENOTTY;
    }

    return 0;
}

// This structure is partially initialized here
// and the rest is initialized by the kernel after call
// to cdev_init()
static struct file_operations b64ed_fops = {
    .open = &cdev_open,
    .release = &cdev_release,
    .read = &cdev_read,
    .unlocked_ioctl = &cdev_ioctl,
    .write = &cdev_write,
    // required to prevent module unloading while fops are in use
    .owner = THIS_MODULE,
};

static int __init cdevmod_init(void)
{
    int status = 0;

    if (max_length <= 0) {
        pr_err("Called with negative buffer length %d\n", max_length);
        status = -EINVAL;
        goto err_handler;
    }

    if (0 == major) {
        // use dynamic allocation (automatic)
        status = alloc_chrdev_region(&my_dev, 0, 1, devname);

    } else {
        // stick with what user provided
        my_dev = MKDEV(major, 0);
        status = register_chrdev_region(my_dev, 1, devname);
    }
    if (status) {
        goto err_handler;
    }

    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) {
        unregister_chrdev_region(my_dev, 1);
        status = -ENOTTY;
        goto err_handler;
    }

    if (device_create(cl, NULL, my_dev, NULL, "cbase64") == NULL) {
        class_destroy(cl);
        unregister_chrdev_region(my_dev, 1);
        status = -ENOTTY;
        goto err_handler;
    }

    cdev_init(&my_cdev, &b64ed_fops);
    if ((status = cdev_add(&my_cdev, my_dev, 1))) {
        device_destroy(cl, my_dev);
        class_destroy(cl);
        unregister_chrdev_region(my_dev, 1);
        goto err_handler;
    }
    pr_info("Registered device with %d:%d\n", MAJOR(my_dev), MINOR(my_dev));
    pr_info("Max length is %d, mode: %scode\n", max_length, (enc_or_dec) ? "de" : "en");
    return 0;

err_handler:
    return status;
}

static void __exit cdevmod_exit(void)
{
    cdev_del(&my_cdev);
    device_destroy(cl, my_dev);
    class_destroy(cl);
    unregister_chrdev_region(my_dev, 1);

    // paranoid checking (afterwards to ensure all fops ended)
    if (files_counter != 0) {
        pr_err("Some files are still opened");    // should never happen
    }
}

module_init(cdevmod_init);
module_exit(cdevmod_exit);

static const unsigned char base64_table[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char b64decode_error[] = "Non-base64 char sequence passed to decoder";

char *b64encode(unsigned char *src, size_t len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;
    size_t olen;
    olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

    if (olen < len)
        return NULL; /* integer overflow */

    char *outStr = NULL;
    outStr = kzalloc(olen, GFP_KERNEL);
    out = outStr;

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        } else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }
    *pos++ = 0;
    return outStr;
}

static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
                                   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
                                   0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
                                   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
                                   0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                                   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

char *b64decode(void *data, size_t len)
{
    unsigned char *p = (unsigned char *)data;
    int pad = len > 0 && (len % 4 || p[len - 1] == '=');

    const size_t L = ((len + 3) / 4 - pad) * 4;
    char *str = kzalloc(L / 4 * 3 + pad, GFP_KERNEL);

    int k;
    for (k = 0; k < len; ++k) {
        if ((p[k] < '/' && p[k] != '+') || (p[k] > '9' && p[k] < 'A' && p[k] != '=')
            || (p[k] > 'Z' && p[k] < 'a') || (p[k] > 'z')) {
            strncpy(str, b64decode_error,
                    (sizeof(b64decode_error) < len) ? sizeof(b64decode_error) : len);
            return str;
        }
    }

    size_t i = 0, j = 0;
    for (; i < L; i += 4) {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6
                | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad) {
        int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
        str[j++] = n >> 16;

        if (len > L + 2 && p[L + 2] != '=') {
            n |= B64index[p[L + 2]] << 6;
            str[j++] = (n >> 8 & 0xFF);
        }
    }
    str[j++] = 0;
    return str;
}
