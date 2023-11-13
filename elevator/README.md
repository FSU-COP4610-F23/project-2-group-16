# project-2-group-16

[Dorm Elevator]

## Group Members
- **Emily Cleveland**: ec19@fsu.edu
- **Sophia Elliott**: sbe21@fsu.edu
- **Ivan Quinones**: iaq20@fsu.edu
## Division of Labor

### Part 1: System Call Tracing
- **Assigned to**: Sophia, Ivan, Emily

### Part 2: Timer Kernel Module
- **Assigned to**: Sophia, Ivan, Emily

### Part 3a: Adding System Calls
- **Assigned to**: Sophia, Ivan, Emily

### Part 3b: Kernel Compilation
- **Assigned to**: Sophia, Ivan, Emily

### Part 3c: Threads
- **Assigned to**: Sophia, Ivan, Emily

### Part 3d: Linked List
- **Assigned to**: Sophia, Ivan, Emily

### Part 3e: Mutexes
- **Assigned to**: Sophia, Ivan, Emily

### Part 3f: Scheduling Algorithm
- **Assigned to**: Sophia, Ivan, Emily

## File Listing
```
elevator/
├── Makefile
├── part1/
│   ├── empty.c
│   ├── empty.trace
│   ├── part1.c
│   ├── part1.trace
│   └── Makefile
├── part2/
│   ├── src/
│   └── Makefile
├── part3/
│   ├── src/
│   ├── tests/
│   ├── Makefile
│   └── sys_call.c
├── Makefile
└── README.md

```
# How to Compile & Execute

### Requirements
- **Compiler**: gcc
- **Dependencies**: N/A

## Part 1

### Compilation
make

This will build the executable in part 1 directory.
### Execution
N/A

## Part 2

### Compilation
make
cd src
sudo insmod my_timer.ko
lsmod | grep my_timer

This will build the executable in the src file. 
### Execution
cat /proc/timer

This will print the current time. 

sleep #

This will sleep for the # of seconds.

cat /proc/timer

This will now show the current and elapsed time.


## Part 3

### Compilation
make
cd src
sudo insmod elevator.ko

This will build the executable in the src file.
### Execution
In one terminal: 
watch -n1 cat /proc/elevator

This will allow the user to watch elevator operate.

In a separate terminal: 
./producer #
./consumer --start

This will create a # of passengers and start the elevator.

When done:
./consumer --stop

This will stop the elevator.

## Bugs
- **Bug 1**: Elevator direction sometimes moves to unnecessary floors when it should be going in the opposite
direction.
- **Bug 2**: Sometimes it doesn't unload on the first time, but it will unload when it hits the correct floor again.

## Considerations
Elevator drops off passengers in FIFO. 