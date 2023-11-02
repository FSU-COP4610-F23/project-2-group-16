#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/random.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("Example of kernel module proc file for elevator");

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL
#define LOG_BUF_LEN 1024
#define NUM_FLOORS 6

static char log_buffer[LOG_BUF_LEN];
static int buf_offset = 0;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int,int,int);
extern int (*STUB_stop_elevator)(void);

enum currentState { OFFLINE, IDLE, LOADING, UP, DOWN };

struct Passenger {
    int type;
    int dest_floor;
    int start_floor;
    int weight;
} ;

struct Floor {
    int floor_num;
    int num_passengers;
    struct list_head passenger_list;
};

struct Elevator {
   
    int current_floor;
    int total_passengers;
    int state; 
    int total_weight;
    int passengers_serviced; 
    //struct list_head passenger_list;
    enum currentState status;
    char * status1; 
    struct task_struct *kthread;
    //struct mutex mutex;
};

struct FloorStatus {
    int floors[6]; 
};

struct Elevator elevator;
static struct Elevator Floor; 
static struct FloorStatus Floor_status; 


static struct proc_dir_entry* elevator_entry;


/* This function is called when when the start_elevator system call is called */
int start_elevator(void) {
    //buf_offset += snprintf(log_buffer + buf_offset, LOG_BUF_LEN - buf_offset, "start_elevator call is called\n");
    return 0;
}

/* This function is called when when the issue_request system call is called */
int issue_request(int start_floor, int destination_floor, int type) {
    printk(KERN_INFO "start: %d, dest: %d,type: %d ", start_floor, destination_floor, type);
    return 0;
}


/* This function is called when when the stop_elevator system call is called */
int stop_elevator(void) {
    //buf_offset += snprintf(log_buffer + buf_offset, LOG_BUF_LEN - buf_offset, "stop_elevator call is called\n");
    return 0;
}

int move_to_next_floor(int floor)
{
   return (floor + 1) % NUM_FLOORS;
}


int process_elevator_state(struct Elevator * e_thread) {
    int status = e_thread->status;
    switch(status) {
        case 0: 
            elevator.status1 = "OFFLINE"; 
            printk(KERN_INFO "Elevator is OFFLINE\n");
            break;
        case 1:
            elevator.status1 = "IDLE"; 
            printk(KERN_INFO "Elevator is IDLE\n");
            break;
        case 2:
            elevator.status1 = "LOADING"; 
            printk(KERN_INFO "Elevator is LOADING\n");
            break;              // changed states!
        case 3: 
            elevator.status1 = "UP"; 
            printk(KERN_INFO "Elevator is going UP\n");
            break;
        case 4:
            elevator.status1 = "DOWN"; 
            printk(KERN_INFO "Elevator is going DOWN\n");
            break;
        default:
            break;
    }
    return status;
}


static ssize_t elevator_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[10000];
    int len = 0;

    len = sprintf(buf, "Elevator state: %s\n", elevator.status1);
    len += sprintf(buf + len, "Current floor: %d\n", elevator.current_floor);
    len += sprintf(buf + len, "Current load: %d\n", elevator.total_weight);
    len += sprintf(buf + len, "Elevator status: \n\n");
    // you can finish the rest.
    len += sprintf(buf + len, "[ ] Floor 6: \n");
    len += sprintf(buf + len, "[ ] Floor 5: \n");
    len += sprintf(buf + len, "[ ] Floor 4: \n");
    len += sprintf(buf + len, "[ ] Floor 3: \n");
    len += sprintf(buf + len, "[ ] Floor 2: \n");
    len += sprintf(buf + len, "[*] Floor 1: \n");

    len += sprintf(buf + len, "Number of Passengers: %d\n", elevator.total_passengers);
    len += sprintf(buf + len, "Number of Passengers waiting: \n");
    len += sprintf(buf + len, "Number of Passengers serviced: %d\n", elevator.passengers_serviced);
    
    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

static int __init elevator_init(void)
{
    // This is where we link our system calls to our stubs
    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;

    //struct Elevator elevator; 
    elevator.current_floor = 1;
    elevator.total_passengers = 0;
    elevator.state=0; 
    elevator.total_weight = 0;
    elevator.passengers_serviced = 0; 
    elevator.status1 = "OFFLINE";         //CHECK THIS AGAIN LATER

    elevator_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &elevator_fops);
    if (!elevator_entry) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit elevator_exit(void)
{

    // This is where we unlink our system calls from our stubs
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;
    
    proc_remove(elevator_entry);
}

module_init(elevator_init);
module_exit(elevator_exit);