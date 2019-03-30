#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>

#include <linux/sched/signal.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("grup surup");

int p=0;
static int simple_init(void)
{
struct task_struct *task;
struct task_struct *task_child;
struct list_head *list;
	printk(KERN_INFO "%s","LOADING MODULE\n");
    for_each_process( task ){
        printk(KERN_INFO "\nPARENT PID: %d PROCESS: %s ",task->pid, task->comm);
				unsigned long long s_time= 99999999999999999999;
				int oldest_pid =0;
				char comm_name[255];
				memset(comm_name, 0, 255 * sizeof(char));

        list_for_each(list, &task->children){
            task_child = list_entry( list, struct task_struct, sibling);
						if(task_child->start_time<s_time){
							s_time=task_child->start_time;
							oldest_pid =task_child->pid;
							strcpy(comm_name, task_child->comm);
						}
						printk(KERN_INFO "\nCHILD OF %s[%d] PID: %d PROCESS: %s ",task->comm, task->pid,  task_child->pid, task_child->comm);


        }

				if(oldest_pid != 0){
				printk(KERN_INFO "\nOldest child of [%d] is process [%d] PID: with name %s",task->pid,oldest_pid, comm_name);
			}

        printk("-----------------------------------------------------");    /*for aesthetics*/
    }


    return 0;
}

static void simple_cleanup(void)
{
	printk(KERN_WARNING "bye1 ...\n");
}

module_init(simple_init);
module_exit(simple_cleanup);
module_param(p, int, 0);
