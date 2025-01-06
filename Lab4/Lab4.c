#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomsk State University");
MODULE_DESCRIPTION("Simple kernel module example");

#define PROC_FILE_NAME "Lab4"

static struct proc_dir_entry *proc_file;

static ssize_t read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    struct timespec64 ts;
    struct tm tm;
    char message[128];
    int len;

    if (*pos > 0) return 0;

    ktime_get_real_ts64(&ts);
    extern struct timezone sys_tz;
    sys_tz.tz_minuteswest = 420;
    time64_to_tm(ts.tv_sec + sys_tz.tz_minuteswest * 60, 0, &tm);

    unsigned int year = tm.tm_year + 1900;
    unsigned int month = tm.tm_mon;
    unsigned int day = tm.tm_mday;
    unsigned int hour = tm.tm_hour;
    unsigned int minute = tm.tm_min;
    unsigned int second = tm.tm_sec;

    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) days[1] = 29;
    unsigned int rday = days[month] - 1 - day;
    unsigned int rhour = 24 * rday - 1 - hour;
    unsigned int rminute = 60 * rhour - 1 - minute;
    unsigned int rsecond = 60 * rminute - second;

    long distance_km = 149597871;
    int lightspeed_km_s = 299793;
    int timefortravel_s = distance_km / lightspeed_km_s;
    int fresult = rsecond / (timefortravel_s + timefortravel_s);

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
