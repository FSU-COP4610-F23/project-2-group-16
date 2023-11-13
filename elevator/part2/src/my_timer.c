#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("Example of kernel module for timer");

#define ENTRY_NAME "timer"
#define PERMS 0644
#define PARENT NULL

static struct proc_dir_entry* timer_entry;
static struct timespec64 ts_start; 
bool firstTime = true;
static int procfs_buf_len;

static ssize_t timer_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{

    struct timespec64 ts_now;
    char buf[256];
    int len = 0;
    //int elapsedTime = 0;
    procfs_buf_len = strlen(buf);
    if(*ppos > 0 || count < procfs_buf_len)
        return 0; 

    if(copy_to_user(ubuf,buf,procfs_buf_len))
        return -EFAULT;

    ktime_get_real_ts64(&ts_now);
    //elapsedTime = ts_now.tv_sec - ts_start.tv_sec;

    //len = snprintf(buf, sizeof(buf), "current time: %lld.%lld\n", (long long)ts_now.tv_sec, (long long)ts_now.tv_nsec);
    if(firstTime==true)
    {
        len = snprintf(buf, sizeof(buf), "Current time: %lld.%lld\n", (long long)ts_now.tv_sec, (long long)ts_now.tv_nsec);
        firstTime = false; 
    }
    else{
        len = snprintf(buf, sizeof(buf), "Current time: %lld.%lld\n", (long long)ts_now.tv_sec, (long long)ts_now.tv_nsec);
        long long difference = (long long)ts_now.tv_nsec- ts_start.tv_nsec;
        
        if (difference < 0)
        {
            //ts_now.tv_sec -= 1;
            //ts_now.tv_nsec += 1000000000; 
            difference *= -1;
            //difference = (long long)ts_now.tv_nsec- ts_start.tv_nsec;
        }
        len += snprintf(buf + len, sizeof(buf) - len, "Elapsed time: %lld.%lld\n", (long long)ts_now.tv_sec- ts_start.tv_sec,difference);

    }
    ts_start.tv_sec = ts_now.tv_sec;
    ts_start.tv_nsec = ts_now.tv_nsec;
    
    
    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}

static const struct proc_ops timer_fops = {
    .proc_read = timer_read,
};

static int __init timer_init(void)
{
    ktime_get_real_ts64(&ts_start);
    timer_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &timer_fops);
    if (!timer_entry) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit timer_exit(void)
{
    proc_remove(timer_entry);
}

module_init(timer_init);
module_exit(timer_exit);