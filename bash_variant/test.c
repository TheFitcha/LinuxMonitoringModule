#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/kmod.h>

#define AUTHOR "Filip Balder"
#define DESCRIPTION "Statux main module."
#define LICENSE "GPL"

#define MAIN_IP "192.168.1.52:5000"

//inotify feature za monitoring promjena na filesystemu?

static int machine_register(void *arg){
	char functionName[20] = "machine_register";

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	struct subprocess_info *scriptInfo;

	//char * argv[] = {"/usr/bin/logger", "help!", NULL };
	char * argv[] = { "/usr/bin/bash", "/home/filip/Documents/Zavrsni/bash_variant/send_request.sh", NULL };

	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC, NULL, NULL, NULL);
	if(scriptInfo == NULL){
		printk(KERN_ERR "Error while creating script (%s)\n", functionName);
		return -ENOMEM;
	}


	int callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_PROC);
	if(callStatus != 0){
		printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
		return callStatus;
	}

	printk(KERN_INFO "Machine register called. Status: %d (%s).\n", callStatus, functionName);

	return 0;
}


int init_routine(void){
	struct task_struct *hello_task;
	int err;

	hello_task = kthread_run(machine_register, NULL, "machine_register_thread");
	if(IS_ERR(hello_task)){
		printk(KERN_ERR "ERROR: Cannot create machine_register_thread!\n");
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
