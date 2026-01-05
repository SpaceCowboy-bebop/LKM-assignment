/**
 * process_lkm.c
 *
 * A loadable kernel module that displays process details for processes
 * with PID greater than a user-specified value.
 * 
 * To compile: make
 * To insert: sudo insmod process_lkm.ko pid_min=500
 * To view output: dmesg | tail -100
 * To remove: sudo rmmod process_lkm
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/list.h>

// Module parameter: minimum PID to display
static int pid_min = 0;
module_param(pid_min, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(pid_min, "Minimum PID to display processes (default: 0)");

/**
 * Converts process state to human-readable string
 */
static const char* get_state_string(long state) {
    switch(state) {
        case TASK_RUNNING:
            return "RUNNING";
        case TASK_INTERRUPTIBLE:
            return "INTERRUPTIBLE";
        case TASK_UNINTERRUPTIBLE:
            return "UNINTERRUPTIBLE";
        case __TASK_STOPPED:
            return "STOPPED";
        case __TASK_TRACED:
            return "TRACED";
        default:
            // Check for EXIT states
            if (state & TASK_DEAD)
                return "DEAD";
            if (state & TASK_WAKEKILL)
                return "WAKEKILL";
            return "UNKNOWN";
    }
}

/**
 * Prints details for a single process
 */
static void print_process_details(struct task_struct *task) {
    printk(KERN_INFO "  Process Name: %s\n", task->comm);
    printk(KERN_INFO "  PID: %d\n", task->pid);
    printk(KERN_INFO "  State: %s (%ld)\n", get_state_string(task->__state), task->__state);
    printk(KERN_INFO "  Priority: %d\n", task->prio);
    printk(KERN_INFO "  Static Priority: %d\n", task->static_prio);
    printk(KERN_INFO "  Normal Priority: %d\n", task->normal_prio);
    printk(KERN_INFO "  Parent PID: %d\n", task->parent ? task->parent->pid : 0);
    printk(KERN_INFO "\n");
}

/**
 * Prints child processes of a given task
 */
static void print_child_processes(struct task_struct *parent_task) {
    struct task_struct *child_task;
    struct list_head *children_list;
    
    printk(KERN_INFO "  --- Child Processes ---\n");
    
    // Iterate through children using the sibling list
    list_for_each(children_list, &parent_task->children) {
        child_task = list_entry(children_list, struct task_struct, sibling);
        print_process_details(child_task);
    }
    
    if (list_empty(&parent_task->children)) {
        printk(KERN_INFO "  No child processes\n\n");
    }
}

/**
 * Module initialization function
 */
static int __init process_lkm_init(void) {
    struct task_struct *task;
    
    printk(KERN_INFO "========================================\n");
    printk(KERN_INFO "Process LKM Loaded\n");
    printk(KERN_INFO "Displaying processes with PID > %d\n", pid_min);
    printk(KERN_INFO "========================================\n\n");
    
    // Iterate through all processes in the system
    for_each_process(task) {
        // Check if PID is greater than user-specified minimum
        if (task->pid > pid_min) {
            printk(KERN_INFO "========================================\n");
            printk(KERN_INFO "MAIN PROCESS:\n");
            print_process_details(task);
            
            // Print parent process if exists
            if (task->parent && task->parent->pid != 0) {
                printk(KERN_INFO "  --- Parent Process ---\n");
                print_process_details(task->parent);
            }
            
            // Print child processes
            print_child_processes(task);
            printk(KERN_INFO "========================================\n\n");
        }
    }
    
    printk(KERN_INFO "Total processes scanned and displayed.\n");
    return 0;
}

/**
 * Module exit function
 */
static void __exit process_lkm_exit(void) {
    printk(KERN_INFO "========================================\n");
    printk(KERN_INFO "Process LKM Unloaded\n");
    printk(KERN_INFO "========================================\n");
}

// Module entry and exit points
module_init(process_lkm_init);
module_exit(process_lkm_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Process Information LKM");
MODULE_AUTHOR("Your Name");
MODULE_VERSION("1.0");