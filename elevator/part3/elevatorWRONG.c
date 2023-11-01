#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mutex.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("Example of kernel module proc file for elevator");

#define ENTRY_NAME "elevator"
#define ENTRY_SIZE 1000
#define PERMS 0644
#define PARENT NULL

static char *message;
static int read_p;
static struct proc_ops fops;

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int,int,int);
extern int (*STUB_stop_elevator)(void);

static struct proc_dir_entry *proc_entry;

/* This function is called when when the start_elevator system call is called */
int start_elevator(void) {
    //buf_offset += snprintf(log_buffer + buf_offset, LOG_BUF_LEN - buf_offset, "start_elevator call is called\n");
    return 0;
}

/* This function is called when when the issue_request system call is called */
int issue_request(int start_floor, int destination_floor, int type) {
    buf_offset += snprintf(log_buffer + buf_offset,
                            LOG_BUF_LEN - buf_offset,
                            "issue_request call with start=%d, destination=%d, type=%d\n",
                            start_floor,
                            destination_floor,
                            type);
    return 0;
}

/* This function is called when when the stop_elevator system call is called */
int stop_elevator(void) {
    buf_offset += snprintf(log_buffer + buf_offset, LOG_BUF_LEN - buf_offset, "stop_elevator call is called\n");
    return 0;
}

/* This function triggers every read! */
static ssize_t procfile_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(ubuf, count, ppos, log_buffer, buf_offset);
}

/* This is where we define our procfile operations */
static struct proc_ops procfile_pops = {
	.proc_read = procfile_read,
};










/*struct {
	//int total_cnt;
	int total_passengers;
	int total_weight;
    int current_floor;
	struct list_head list;
} elevator;*/

#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define NUM_PASSENGER_TYPES 4
#define MAX_PASSENGERS 5
#define MAX_FLOOR 6
#define MIN_FLOOR 1
#define MAX_WEIGHT 750

#define LOG_BUF_LEN 1024

static char log_buffer[LOG_BUF_LEN];
static int buf_offset = 0;

/*typedef struct passengers {
	int start_floor;
	int dest_floor;
	int weight;
	int type;
	//const char *name;
	struct list_head list;
} Passenger; */

struct Passenger {
    int type;
    int dest_floor;
    int start_floor;
    int weight;
}

struct Floor {
    int floor_num;
    int num_passengers;
    struct list_head passenger_list;
}

struct Elevator {
    struct task_struct *kthread;
    int current_floor;
    int total_passengers;
    int state; 
    int total_weight;
    struct list_head passenger_list;
}

static int elevator_thread(void* data)
{
    //elevator movement control code
    return 0; 
}

DEFINE_MUTEX(elevator_mutex);

/*************** LINKED LIST ************************************/
/*void add_passenger(struct Floor* floor, int dest_floor, int type) {
	//int start_floor;
        //int dest_floor;
	int weight;
	Passenger *a;

	if (elevator.total_passengers >= MAX_PASSENGERS)
		//return 0;

	switch (type) {
		case FRESHMAN:
			weight = 100;
			break;
		case SOPHOMORE:
			weight = 150;
			break;
		case JUNIOR:
			weight = 200;
			break;
		case SENIOR:
			weight = 250;
			break;
		default:
			//return -1;
	}

	a = kmalloc(sizeof(Passenger) * 1, __GFP_RECLAIM);
	if (a == NULL)
		return -ENOMEM;

	a->type = type;
	a->start_floor = start_floor;
	a->weight = weight;
	a->dest_floor = dest_floor;
***********************************************


	//list_add(&a->list, &animals.list); // insert at front of list 
	list_add_tail(&a->list, &elevator.list); // insert at back of list 

	elevator.total_passengers += 1;
	//passengers.total_length += length;
	elevator.total_weight += weight;

	//return 0;
}


void remove_passengers(struct Floor* floor, struct Passenger* passenger)
{
    //
}


void control_elevator(struct Elevator* elevator)
{
    //uses kthreads
}

void loading_passengers(struct Elevator* elevator, struct Floor* floor)
{
    //
}
*****************************************/

/*int print_elevator(void) {
	int i;
	Passenger *a;
	struct list_head *temp;

	char *buf = kmalloc(sizeof(char) * 100, __GFP_RECLAIM);
	if (buf == NULL) {
		printk(KERN_WARNING "print_animals");
		return -ENOMEM;
	}

	// init message buffer 
	strcpy(message, "");

	// headers, print to temporary then append to message buffer 
	sprintf(buf, "Total count is: %d\n", elevator.total_passengers);       strcat(message, buf);
	//sprintf(buf, "Total length is: %d\n", animals.total_length);   strcat(message, buf);
	sprintf(buf, "Total weight is: %d\n", elevator.total_weight);   strcat(message, buf);
	sprintf(buf, "Passengers seen:\n");                               strcat(message, buf);

	// print entries 
	i = 0;
	//list_for_each_prev(temp, &animals.list) { // backwards 
	list_for_each(temp, &elevator.list) { // forwards
		a = list_entry(temp, Passenger, list);

		// newline after every 5 entries 
		if (i % 5 == 0 && i > 0)
			strcat(message, "\n");

		sprintf(buf, "%d ", a->type);
		strcat(message, buf);

		i++;
	}

	// trailing newline to separate file from commands 
	strcat(message, "\n");

	kfree(buf);
	return 0;
}
*****************************************************
void delete_passengers(int type) {
	struct list_head move_list;
	struct list_head *temp;
	struct list_head *dummy;
	int i;
	Passenger *a;

	INIT_LIST_HEAD(&move_list);

	// move items to a temporary list to illustrate movement 
	//list_for_each_prev_safe(temp, dummy, &animals.list) { // backwards 
	list_for_each_safe(temp, dummy, &animals.list) { // forwards 
		a = list_entry(temp, Animal, list);

		if (a->id == type) {
			//list_move(temp, &move_list); // move to front of list 
			list_move_tail(temp, &move_list); // move to back of list 
		}

	}

	// print stats of list to syslog, entry version just as example (not needed here) 
	i = 0;
	//list_for_each_entry_reverse(a, &move_list, list) { // backwards 
	list_for_each_entry(a, &move_list, list) { // forwards 
		// can access a directly e.g. a->id 
		i++;
	}
	printk(KERN_NOTICE "animal type %d had %d entries\n", type, i);

	// free up memory allocation of Animals 
	//list_for_each_prev_safe(temp, dummy, &move_list) { // backwards 
	list_for_each_safe(temp, dummy, &move_list) { // forwards 
		a = list_entry(temp, Animal, list);
		list_del(temp);	// removes entry from list 
		kfree(a);
	}
}

********/

/********************************************************************/

static struct proc_dir_entry* elevator_entry;

static ssize_t elevator_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[10000];
    int len = 0;

    len = sprintf(buf, "Elevator state: \n");
    len += sprintf(buf + len, "Current floor: \n");
    len += sprintf(buf + len, "Current load: \n");
    len += sprintf(buf + len, "Elevator status: \n");
    // you can finish the rest.

    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}

static const struct proc_ops elevator_fops = {
    .proc_read = elevator_read,
};

/****************************************************/

int elevator_proc_open(struct inode *sp_inode, struct file *sp_file) {
	read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "elevator_proc_open");
		return -ENOMEM;
	}

	add_passenger(get_random_long() % MAX_FLOOR,get_random_long() % MAX_FLOOR, get_random_long() % NUM_PASSENGER_TYPES);
	return print_elevator();
}

ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
	int len = strlen(message);

	read_p = !read_p;
	if (read_p)
		return 0;

	copy_to_user(buf, message, len);
	return len;
}

int elevator_proc_release(struct inode *sp_inode, struct file *sp_file) {
	kfree(message);
	return 0;
}

//proc_show
static int elevator_proc_show(struct seq_file* m, void* v)
{
    //read operations for /proc/elevator
    //format and print elevator status
    return 0;
}


/*************************************************************/


static int __init elevator_init(void)
{
	/*fops.proc_open = elevator_proc_open;
	fops.proc_read = elevator_proc_read;
	fops.proc_release = elevator_proc_release;*/
    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;

    /*proc_entry = proc_create(                   // this is where we create the proc file!
        ENTRY_NAME,
        PERMS,
        PARENT,
        &procfile_pops
    );*/

    elevator_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &elevator_fops);
    if (!elevator_entry) {
        return -ENOMEM;
    }
	/*elevator.total_passengers = 0;
	elevator.total_weight = 0;
	INIT_LIST_HEAD(&elevator.list);*/



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
