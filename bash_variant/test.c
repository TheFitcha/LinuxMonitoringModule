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

static void machine_register_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "machine_register_cleanup\n");
}

static void process_register_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "process_register_cleanup\n");
}

static void process_update_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "process_update_cleanup\n");
}

static int machine_register(void *arg){
	char functionName[20] = "machine_register";

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	struct subprocess_info *scriptInfo;

	char * argv[] = { "/usr/bin/bash", "/home/filip/Documents/Zavrsni/bash_variant/send_request.sh", MAIN_IP, "machineRegister", NULL };

	char * envp[] = { "HOME=/home/filip/Documents/Zavrsni/bash_variant", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC, NULL, &machine_register_cleanup, NULL);
	if(scriptInfo == NULL){
		printk(KERN_ERR "Error while creating script (%s)\n", functionName);
		return -ENOMEM;
	}

	printk(KERN_INFO "%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
	int callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

	if(callStatus != 0){
		printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
		return callStatus;
	}

	printk(KERN_INFO "Machine register called. Status: %d (%s).\n", callStatus, functionName);

	return 0;
}


//treba handlati argument id procesa
static int process_register(void *arg){
	char functionName[20] = "process_register";
	int processId = 1;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	struct subprocess_info *scriptInfo;

	char * argv[] = { "/usr/bin/bash", "/home/filip/Documents/Zavrsni/bash_variant/send_request.sh", MAIN_IP, "processRegister", processId, NULL };

	char * envp[] = { "HOME=/home/filip/Documents/Zavrsni/bash_variant", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC, NULL, &process_register_cleanup, NULL);
	if(scriptInfo == NULL){
		printk(KERN_ERR "Error while creating script (%s)\n", functionName);
		return -ENOMEM;
	}

	printk(KERN_INFO "%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
	int callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

	if(callStatus != 0){
		printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
		return callStatus;
	}

	printk(KERN_INFO "Process register called. Status: %d (%s).\n", callStatus, functionName);

	return 0;
}

//treba handlati parametar za process id
static int process_update(void *arg){
	char functionName[20] = "process_update";
	int processId = 1;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	struct subprocess_info *scriptInfo;

	char * argv[] = { "/usr/bin/bash", "/home/filip/Documents/Zavrsni/bash_variant/send_request.sh", MAIN_IP, "processUpdate", processId, NULL };

	char * envp[] = { "HOME=/home/filip/Documents/Zavrsni/bash_variant", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC, NULL, &process_update_cleanup, NULL);
	if(scriptInfo == NULL){
		printk(KERN_ERR "Error while creating script (%s)\n", functionName);
		return -ENOMEM;
	}

	printk(KERN_INFO "%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
	int callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

	if(callStatus != 0){
		printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
		return callStatus;
	}

	printk(KERN_INFO "Process update called. Status: %d (%s).\n", callStatus, functionName);

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
