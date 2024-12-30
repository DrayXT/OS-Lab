#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomsk State University");
MODULE_DESCRIPTION("Simple kernel module example");

#define PROC_FILE_NAME "Lab4"

static struct proc_dir_entry* proc_file;

static ssize_t read(struct file* file, char __user* buf, size_t count, loff_t* pos)
{
    char message[512];
    int len;

    if (*pos > 0) return 0;

    long distance_km = 149597871;
    int lightspeed_km_s = 299793;
    int seconds_s = (39 * 60 + 18) * 60 + 16;
    double result_s = distance_km / lightspeed_km_s;
    int fresult = seconds_s / (2 * result_s);

    len = snprintf(message, sizeof(message), "Light can travel from the Sun to the Earth and back %d times till the end of the month \n", fresult);

    if (copy_to_user(buf, message, len)) return -EFAULT;

    *pos += len;
    return len;
}

static const struct proc_ops proc_file_ops = {
    .proc_read = read,
};

static int __init tsu_module_init(void)
{
    pr_info("Welcome to the Tomsk State University\n");
    proc_file = proc_create(PROC_FILE_NAME, 0444, NULL, &proc_file_ops);
    pr_info("/proc/%s created\n", PROC_FILE_NAME);

    return 1;
}

static void __exit tsu_module_exit(void)
{
    proc_remove(proc_file);
    pr_info("/proc/%s removed\n", PROC_FILE_NAME);
    pr_info("Tomsk State University forever!\n");
}

module_init(tsu_module_init);
module_exit(tsu_module_exit);