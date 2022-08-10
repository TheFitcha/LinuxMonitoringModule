#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/uhm.h>

#define AUTHOR "Filip Balder"
#define DESCRIPTION "Statux main module."
#define LICENSE "GPL"

#define MAIN_PORT 5000


static int send_hello(void *arg){
	char functionName[20] = "send_hello";

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);	

	char argv[5][20];
	argv[0] = "/bin/bash";
	argv[1] = "-c";
	argv[2] = "/usr/bin/free";
	argv[3] = NULL;

	char envp[5][20];
	envp[0] = "HOME=/";
	envp[1] = "TERM=linux";
	envp[2] = "PATH=/sbin:/usr/sbin:/bin:/usr/bin";
	envp[3] = NULL;

	call_usermodehelper(argv[0], argv, envp, UHM_WAIT_EXEC);

	printk(KERN_INFO "Successfully sent message %s (%s).\n", helloMsg, functionName);

	return 0;
}


int init_routine(void){
	struct task_struct *hello_task;
	int err;

	hello_task = kthread_run(send_hello, NULL, "send_hello_thread");
	if(IS_ERR(hello_task)){
		printk(KERN_ERR "ERROR: Cannot create send_hello_thread!");
		err = PTR_ERR(hello_task);
		hello_task = NULL;
		return err;
	}

	printk("Main statux module loaded!\n");
	return 0;
}

void exit_routine(void){
	printk("Main statux module unloaded!\n");
}

module_init(init_routine);
module_exit(exit_routine);
MODULE_LICENSE(LICENSE);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);
