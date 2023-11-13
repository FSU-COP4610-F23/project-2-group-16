#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("A elevator that demonstrates kernel threads with proc entry");

#define NUM_FLOORS 6

//define types
#define FRESHMAN 0
#define SOPHOMORE 1
#define JUNIOR 2
#define SENIOR 3

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL

#define MAX_PASSENGERS 5
#define MAX_LOAD 750

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int,int,int);
extern int (*STUB_stop_elevator)(void);

enum state {OFFLINE, UP, DOWN, LOADING, IDLE};      
// you should note that enums are just integers.

struct elevator {
    int current_floor;              // an example of information to store in your elevator.
    int total_serviced;
    int total_passengers;
    int total_waiting; 
    enum state status;
    enum state direction;           //remember the intended direction after loading
    int load_weight;
    struct task_struct *kthread;    // this is the struct to make a kthread.
    //struct list_head list;
    struct list_head passengers;
     struct mutex elevator_mutex;
};

struct floor {
    int num_passengers;
    struct list_head list;  // this is how we set up a list of a struct
    struct mutex floor_mutex;
};

//passenger struct
struct Passenger {       //change to passenger
    char *name;
    struct list_head list; // this is how we set up a list of a struct
    int start_floor;  
    int dest_floor;
    int type;
    int weight;
};

//floors struct
struct Floors {
    int Floor[6];
    struct floor floors[6];     //change to 6
};

static struct mutex floors_mutex;
static struct proc_dir_entry *proc_entry;

/* Here is our global variables */
static struct elevator elevator_thread;
static struct Floors Floors;


/* Move to the next floor */
int move_to_next_floor(int floor) {
    return (floor + 1) % NUM_FLOORS;
}

int move_down_floor(int floor) {
    return (floor + NUM_FLOORS - 1) % NUM_FLOORS;
}

void process_elevator_state(struct elevator * w_thread) {
    mutex_lock(&w_thread->elevator_mutex);
    struct floor *current_floor = &Floors.floors[w_thread->current_floor];
    struct list_head *temp;
    struct list_head *dummy;
    struct Passenger *passenger = list_first_entry(&w_thread->passengers, struct Passenger, list);

    switch(w_thread->status) {
        case UP:
            ssleep(1);  //sleeping for 1 second before processing next stuff
    
            if (w_thread->load_weight > MAX_LOAD || w_thread->total_passengers > MAX_PASSENGERS){
                //if at max weight then continue movement
                
                w_thread->current_floor = move_to_next_floor(w_thread->current_floor);
                w_thread->direction = UP; 
            }

            if (w_thread->current_floor == NUM_FLOORS - 1) {
                //if at the top floor, start moving DOWN after loading
                if (!list_empty(&current_floor->list) 
                && w_thread->total_passengers <= MAX_PASSENGERS 
                && w_thread->load_weight <= MAX_LOAD) {
                w_thread->status = LOADING;
                } else {
                w_thread->direction = DOWN;
                w_thread->status = DOWN;
                }
            }
            
            else {
                if (!list_empty(&current_floor->list) 
                && w_thread->total_passengers <= MAX_PASSENGERS 
                && w_thread->load_weight <= MAX_LOAD) {
                w_thread->status = LOADING;
                } else{
                //else , move to the next floor and then load
                w_thread->current_floor = move_to_next_floor(w_thread->current_floor);
                w_thread->direction = UP; 
                }
            }
            if (passenger->dest_floor == w_thread->current_floor+1) {
                    w_thread->status = LOADING;
                    
            }
            if (list_empty(&w_thread->passengers) && w_thread->total_waiting == 0) {
                w_thread->status = IDLE;
            }
            break;
        case DOWN:
            ssleep(1);  // Sleeps for 1 second before processing next stuff!
             
             if (w_thread->load_weight > MAX_LOAD || w_thread->total_passengers > MAX_PASSENGERS){
                // If at the max start moving DOWN after loading
                
                w_thread->current_floor = move_down_floor(w_thread->current_floor);
                w_thread->direction = DOWN; 
            }

            if (w_thread->current_floor == 0) {
                if (!list_empty(&current_floor->list) 
                && w_thread->total_passengers <= MAX_PASSENGERS 
                && w_thread->load_weight <= MAX_LOAD) {
                w_thread->status = LOADING;
                } else {
                w_thread->direction = UP;
                w_thread->status = UP;
                }
                } 
            
            
            else {
                if (!list_empty(&current_floor->list) 
                && w_thread->total_passengers <= MAX_PASSENGERS 
                && w_thread->load_weight <= MAX_LOAD) {
                w_thread->status = LOADING;
                } else{

                //else, move to the previous floor and then load
                w_thread->current_floor = move_down_floor(w_thread->current_floor);
            
                w_thread->direction = DOWN; 
                }
            }

            if (passenger->dest_floor == w_thread->current_floor+1) {
                    w_thread->status = LOADING;                        
                }
            

            if (list_empty(&w_thread->passengers) && w_thread->total_waiting == 0) {
                w_thread->status = IDLE;
            }
            break;

        case LOADING:
            ssleep(2);  // Sleep for 2 seconds for loading

            // Unloading passengers
            mutex_lock(&current_floor->floor_mutex);
            list_for_each_safe(temp, dummy, &w_thread->passengers) {
                passenger = list_entry(temp, struct Passenger, list);
                if (passenger->dest_floor == w_thread->current_floor+1) {
                    list_del(temp);  // Remove passenger from the elevator
                    w_thread->load_weight -= passenger->weight;  // Adjust load weight
                    kfree(passenger);  // Free the memory allocated for the passenger
                    w_thread->total_serviced++;
                w_thread->total_passengers--;
            }
            }

            bool can_add_more = true;  // Flag to check if more passengers can be added
            // Loading passengers
            list_for_each_safe(temp, dummy, &current_floor->list) {
            if (!can_add_more || w_thread->total_passengers >= MAX_PASSENGERS) break;

            passenger = list_entry(temp, struct Passenger, list);

            if (w_thread->load_weight + passenger->weight <= MAX_LOAD 
            && w_thread->total_passengers < MAX_PASSENGERS) {
                list_move_tail(temp, &w_thread->passengers); // Move passenger to elevator
                w_thread->load_weight += passenger->weight; // Update load weight
                current_floor->num_passengers--; // Decrease the num of passengers on curr floor
                w_thread->total_passengers++; 
                w_thread->total_waiting--;
            } else {
                can_add_more = false;//checks load_weight is at max_load
            }
            }
            mutex_unlock(&current_floor->floor_mutex); // Unlock mutex for the current floor

        // Decide next state based on the destinations of passengers inside
        if (!list_empty(&w_thread->passengers)) {
            struct Passenger *first_passenger = 
            list_first_entry(&w_thread->passengers, struct Passenger, list);
            if (first_passenger->dest_floor > w_thread->current_floor) {
                w_thread->direction = UP;
                w_thread->current_floor = move_to_next_floor(w_thread->current_floor);
                
            } else {
                w_thread->direction = DOWN;
                w_thread->current_floor = move_down_floor(w_thread->current_floor);
            
            }
            w_thread->status = w_thread->direction;
        } else if (w_thread->total_waiting > 0) {
            // If there are passengers waiting, but none on the elevator, decide next direction
            w_thread->status = (w_thread->current_floor == 0) ? UP : DOWN;
        } 
    
        else if (w_thread->load_weight > MAX_LOAD || w_thread->total_passengers > MAX_PASSENGERS){
            // If there are passengers waiting, but none on the elevator, decide next direction
            w_thread->status = (w_thread->current_floor == 0) ? UP : DOWN;
            break; }
        
        else {
            w_thread->status = IDLE;
        }
        break;
        case IDLE:
            // In IDLE state, the elevator does nothing but waits for new passengers
            ssleep(1); // Sleep to prevent busy waiting

            // Transition out of IDLE if there are passengers waiting
            if (w_thread->total_waiting > 0) {
                w_thread->status = (w_thread->current_floor == 0) ? UP : DOWN;
            }
            break;

        default:
            break;
    }
    mutex_unlock(&w_thread->elevator_mutex);
}

/* LIST  */

int add_passengers(int start, int dest, int type) {
    int weight;
    char *name;
    struct Passenger *a;

    switch (type) {
        case FRESHMAN:
            weight = 100;
            name = "F";
            break;
        case SOPHOMORE:
            weight = 150;
            name = "O";
            break;
        case JUNIOR:
            weight = 200;
            name = "J";
            break;
        case SENIOR:
            weight = 250;
            name = "S";
            break;
        default:
            return -1;
    }

    a = kmalloc(sizeof(struct Passenger) * 1, __GFP_RECLAIM);
    if (a == NULL)
        return -ENOMEM;

    a->type = type;
    a->start_floor = start;
    a->dest_floor = dest; 
    a->weight = weight;
    a->name = name;

    mutex_lock(&floors_mutex);
    list_add_tail(&a->list, &Floors.floors[start-1].list); // Add to list of start floor
    Floors.floors[start-1].num_passengers++; 
    elevator_thread.total_waiting++; 
    mutex_unlock(&floors_mutex);

    return 0;
}


void delete_passengerss(int floor) {
    struct list_head move_list;
    struct list_head *temp;
    struct list_head *dummy;
    int i;
    struct Passenger *a;

    INIT_LIST_HEAD(&move_list);

    /* move items to a temporary list to illustrate movement */
    /* backwards */
    list_for_each_safe(temp, dummy, &Floors.floors[floor].list) { 
        /* forwards */
        a = list_entry(temp, struct Passenger, list);

        list_move_tail(temp, &move_list); /* move to back of list */
    }
    /* print stats of list to syslog, entry version just as example (not needed here) */
    i = 0;
    list_for_each_entry(a, &move_list, list) { /* forwards */
        /* can access a directly e.g. a->id */
        i++;
    }

    /* free up memory allocation of struct Passengers */
    list_for_each_safe(temp, dummy, &move_list) { /* forwards */
        a = list_entry(temp, struct Passenger, list);
        list_del(temp); /* removes entry from list */
        kfree(a);
    }
}

void delete_elevator(void) {
    struct list_head move_list;
    struct list_head *temp;
    struct list_head *dummy;
    int i;
    struct Passenger *a;

    INIT_LIST_HEAD(&move_list);

    /* move items to a temporary list to illustrate movement */
    list_for_each_safe(temp, dummy, &elevator_thread.passengers) { /* forwards */
        a = list_entry(temp, struct Passenger, list);


        list_move_tail(temp, &move_list); /* move to back of list */
        
    }
    /* print stats of list to syslog, entry version just as example (not needed here) */
    i = 0;
    list_for_each_entry(a, &move_list, list) { /* forwards */
        /* can access a directly e.g. a->id */
        i++;
    }

    /* free up memory allocation of struct Passengers */
    list_for_each_safe(temp, dummy, &move_list) { /* forwards */
        a = list_entry(temp, struct Passenger, list);
        list_del(temp); /* removes entry from list */
        kfree(a);
    }
}

/* elevator */
int elevator_active(void * _elevator) {
    struct elevator * w_thread = (struct elevator *) _elevator;
    printk(KERN_INFO "elevator thread has started running \n");
    while(!kthread_should_stop()) {
        process_elevator_state(w_thread);
    }
    return 0;
}

/* This is where we spawn our elevator thread */
int spawn_elevator(struct elevator * w_thread) {
    static int current_floor = 0;

    w_thread->current_floor = current_floor;
    w_thread->kthread =
        kthread_run(elevator_active, w_thread, "thread elevator\n"); 
        // thread actually spawns here

    w_thread->status = UP; 
    return 0;
}

// function to print to proc file the Floors state using elevator state
int print_Floors_state(char * buf) {
    int i;
    int j; 
    int len = 0;
    struct Passenger *a;
    struct Passenger *passenger;
    struct list_head *temp;

    // convert enums (integers) to strings
    const char * states[5] = {"OFFLINE","UP", "DOWN","LOADING", "IDLE"};

    len += sprintf(buf + len, "Elevator State: %s\n", states[elevator_thread.status]);
    len += sprintf(buf + len, "Current floor: %d\n", elevator_thread.current_floor+1);
    len += sprintf(buf + len, "Current load: %d\n", elevator_thread.load_weight);
    len += sprintf(buf + len, "Elevator status: ");
    
    //chat
    list_for_each(temp, &elevator_thread.passengers) {
        passenger = list_entry(temp, struct Passenger, list);
        len += sprintf(buf + len, "%s%d ", passenger->name, passenger->dest_floor);
    }
    //chat
    len += sprintf(buf + len, "\n\n"); 
    for(i = 5; i >= 0; i--) {
        int floor = i + 1;

        // ternary operators equivalent to the bottom if statement.
        len += (i != elevator_thread.current_floor)
            ? sprintf(buf + len, "[ ] Floor %d: ", floor)
            : sprintf(buf + len, "[*] Floor %d: ", floor);

        /* print entries */
        len += sprintf(buf + len, "%d ", Floors.floors[i].num_passengers);
        j = 0;
    //list_for_each_prev(temp, &passengerss.list) { /* backwards */
        list_for_each(temp, &Floors.floors[i].list) { /* forwards*/
        a = list_entry(temp, struct Passenger, list);
        len += sprintf(buf + len, "%s", a->name);
        len += sprintf(buf + len, "%d ", a->dest_floor);
        //strcat(message, buf);
        j++;
        }
        len += sprintf(buf + len, " \n"); 
    }
    len += sprintf(buf + len,"\n\nNumber of Passengers: %d\n",elevator_thread.total_passengers);
    len += sprintf(buf + len,"Number of Passengers waiting: %d\n",elevator_thread.total_waiting);
   len += sprintf(buf + len,"Number of Passengers serviced: %d\n",elevator_thread.total_serviced);
   
    return len;
}

/* triggers every read */
static ssize_t procfile_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[512];
    int len = 0;

    if (*ppos > 0 || count < 512)
        return 0;

    // recall that this is triggered every second if we do watch -n1 cat /proc/elevator
    len = print_Floors_state(buf);   //write to the file

    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}


/**************** SYSTEM CALLS *************************/

int start_elevator(void) {
    //mutex??
    spawn_elevator(&elevator_thread); 
    return 0;
}

int issue_request(int start_floor, int destination_floor, int type) {
    printk(KERN_INFO "start: %d, dest: %d,type: %d ", start_floor, destination_floor, type);
    //use linked lists to put in right floors
    add_passengers(start_floor, destination_floor, type); 
    return 0;
}

int stop_elevator(void) {
    
    kthread_stop(elevator_thread.kthread);  
    elevator_thread.status = OFFLINE;
    return 0;
}

/***************** END OF SYSCALLS *********************/

/* This is where we define our procfile operations */
static struct proc_ops procfile_pops = {
    .proc_read = procfile_read,
};

/* Treat this like a main function, this is where your kernel module will
   start from, as it gets loaded. */
static int __init elevator_init(void) {

    //chat
    elevator_thread.load_weight = 0;
    elevator_thread.total_serviced = 0;
    elevator_thread.total_passengers = 0;
    elevator_thread.total_waiting = 0;
    //chat

    elevator_thread.load_weight = 0; // Initialize the load weight
    INIT_LIST_HEAD(&elevator_thread.passengers);

    for (int i = 0; i < NUM_FLOORS; i++) {
        Floors.floors[i].num_passengers = 0;
        INIT_LIST_HEAD(&Floors.floors[i].list);
        mutex_init(&Floors.floors[i].floor_mutex);
    }
    //chat
    mutex_init(&floors_mutex);
    mutex_init(&elevator_thread.elevator_mutex);
    
    // This is where we link our system calls to our stubs
    STUB_start_elevator = start_elevator;
    STUB_issue_request = issue_request;
    STUB_stop_elevator = stop_elevator;

    if(IS_ERR(elevator_thread.kthread)) {
        printk(KERN_WARNING "error creating thread");
        remove_proc_entry(ENTRY_NAME, PARENT);
        return PTR_ERR(elevator_thread.kthread);
    }

    proc_entry = proc_create(                   //create the proc file
        ENTRY_NAME,
        PERMS,
        PARENT,
        &procfile_pops
    );

    return 0;
}

/* exiting our kernel module, when we unload it */
static void __exit elevator_exit(void) {

   //unlinking our system calls from our stubss
    STUB_start_elevator = NULL;
    STUB_issue_request = NULL;
    STUB_stop_elevator = NULL;

    for (int i = 0; i < 6; i++)
    {
        Floors.floors[i].num_passengers = 0;
        delete_passengerss(i);
    }
    delete_elevator();

    elevator_thread.load_weight = 0;
    elevator_thread.total_passengers = 0;
    elevator_thread.total_waiting = 0;
    
    for (int i = 0; i < NUM_FLOORS; i++) {
        mutex_destroy(&Floors.floors[i].floor_mutex); //destroy the mutex for each floor
    }

    mutex_destroy(&elevator_thread.elevator_mutex); //destroy the mutex for the elevator
    mutex_destroy(&floors_mutex);

    //kthread_stop(elevator_thread.kthread);                //stop the thread.
    remove_proc_entry(ENTRY_NAME, PARENT);              //remove the proc file
    
}

module_init(elevator_init); //initiates kernel module
module_exit(elevator_exit); //exits kernel module