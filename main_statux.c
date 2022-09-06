#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/kmod.h>
#include<linux/delay.h>
#include<linux/moduleparam.h>

#include<linux/rcupdate.h>
#include<linux/fdtable.h>
#include<linux/fs.h>
#include<linux/fs_struct.h>
#include<linux/dcache.h>
#include<linux/slab.h>

#define AUTHOR "Filip Balder"
#define DESCRIPTION "Statux main module."
#define LICENSE "GPL"

#define SIZEOF(arr) sizeof(arr)/sizeof(*arr)

#define MAIN_IP "192.168.137.161:5000"
#define SIZE_PARAMS 10
#define SEND_FREQ 5000
#define SEND_FREQ_MEM 10000
#define SCRIPT_NAME "send_request.sh"

struct task_struct *process_update_task;
struct task_struct *memory_update_task;

char *cwd, *abs_script_path;

static int pids[100];
static int processIdsCount;
module_param_array(pids, int, &processIdsCount, 0660);
MODULE_PARM_DESC(pids, "User filled array with process ID (int) targeted for monitoring.");

static int freq = 0;
module_param(freq, int, 0660);
MODULE_PARM_DESC(freq, "Custom sending frequency in ms (default: 5000)");

static int freq_mem = 0;
module_param(freq_mem, int, 0660);
MODULE_PARM_DESC(freq_mem, "Custom sending freqency in ms for memory update (default: 10000)");

static int keep = 0;
module_param(keep, int, 0660);
MODULE_PARM_DESC(keep, "Set this parameter to 1 to keep data in database after unloading module.");

static void machine_register_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "machine_register_cleanup\n");
}

static void process_register_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "process_register_cleanup\n");
}

static void process_update_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "process_update_cleanup\n");
}

static void memory_update_cleanup(struct subprocess_info *info){
	printk(KERN_INFO "memory_update_cleanup\n");
}

static int machine_register(void *arg){
	char functionName[20] = "machine_register";
	struct subprocess_info *scriptInfo;
	int callStatus;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	char * argv[] = { "/usr/bin/bash", abs_script_path, MAIN_IP, "machineRegister", NULL };

	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, &machine_register_cleanup, NULL);
	if(scriptInfo == NULL){
		printk(KERN_ERR "Error while creating script (%s)\n", functionName);
		return -ENOMEM;
	}

	printk(KERN_INFO "%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
	callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

	if(callStatus != 0){
		printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", -callStatus, functionName);
		return -callStatus;
	}

	printk(KERN_INFO "Machine register called. Status: %d (%s).\n", callStatus, functionName);

	return 0;
}


static int process_register(void *arg){
	char functionName[20] = "process_register";
	int * processIds = arg;
	struct subprocess_info *scriptInfo;
	char processIdString[10];
	int callStatus, i;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	for (i = 0; i<SIZEOF(processIds); i++) {
		if(processIds[i] == 0) continue;

		sprintf(processIdString, "%d", processIds[i]);
		printk(KERN_INFO "Process ID: %s\n", processIdString);

		char * argv[] = { "/usr/bin/bash", abs_script_path, MAIN_IP, "processRegister", processIdString, NULL };

		char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

		scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, &process_register_cleanup, NULL);
		if(scriptInfo == NULL){
			printk(KERN_ERR "Error while creating script (%s)\n", functionName);
			return -ENOMEM;
		}

		printk(KERN_INFO "%s %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);

		callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

		if(callStatus != 0){
			printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
			return -callStatus;
		}
	}

	printk(KERN_INFO "Process register called. Status: %d (%s).\n", callStatus, functionName);

	return 0;
}

static int process_update(void *arg){
	char functionName[20] = "process_update";
	int * processIds = arg;
	struct subprocess_info *scriptInfo;
	char processIdString[10];
	int callStatus, i;

	int sending_frequency = (freq > 0) ? freq : SEND_FREQ;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	while (!kthread_should_stop()){
		for (i = 0; i<SIZEOF(processIds); i++) {
			if(processIds[i] == 0) continue;

			sprintf(processIdString, "%d", processIds[i]);
			printk(KERN_INFO "Process ID: %s\n", processIdString);

			char * argv[] = { "/usr/bin/bash", abs_script_path, MAIN_IP, "processUpdate", processIdString, NULL };

			char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

			scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, &process_update_cleanup, NULL);
			if(scriptInfo == NULL){
				printk(KERN_ERR "Error while creating script (%s)\n", functionName);
				return -ENOMEM;
			}

			printk(KERN_INFO "%s %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);

			callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

			if(callStatus != 0){
				printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
				return -callStatus;
			}

			printk(KERN_INFO "Process update called. Status: %d (%s).\n", callStatus, functionName);
			msleep_interruptible(sending_frequency);
		}
	}

	return 0;
}


static int memory_update(void *arg){
	char functionName[20] = "memory_update";
	int callStatus;
	struct subprocess_info *scriptInfo;
	int sending_frequency = (freq_mem > 0) ? freq_mem : SEND_FREQ_MEM;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	while(!kthread_should_stop()){

		char * argv[] = { "/usr/bin/bash", abs_script_path, MAIN_IP, "memoryUpdate", NULL };

		char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

		scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, &memory_update_cleanup, NULL);
		if(scriptInfo == NULL){
			printk(KERN_ERR "Error while creating script (%s)\n", functionName);
			return -ENOMEM;
		}

		printk(KERN_INFO "%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
		callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

		if(callStatus != 0){
			printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
			return -callStatus;
		}

		printk(KERN_INFO "Memory update called. Status: %d (%s).\n", callStatus, functionName);
		msleep_interruptible(sending_frequency);
	}
	return 0;
}

static int remove_machine(void *arg){
	char functionName[20] = "machine_delete";
	int callStatus;
	struct subprocess_info *scriptInfo;

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);

	char * argv[] = { "/usr/bin/bash", abs_script_path, MAIN_IP, "machineDelete", NULL };

	char * envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	scriptInfo = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, &machine_register_cleanup, NULL);
	if(scriptInfo == NULL){
		printk(KERN_ERR "Error while creating script (%s)\n", functionName);
		return -ENOMEM;
	}

	printk(KERN_INFO "%s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
	callStatus = call_usermodehelper_exec(scriptInfo, UMH_WAIT_EXEC);

	if(callStatus != 0){
		printk(KERN_ERR "Error while calling script (code: %d) (%s)\n", callStatus >> 8, functionName);
		return -callStatus;
	}

	printk(KERN_INFO "Machine delete called. Status: %d (%s).\n", callStatus, functionName);

	return 0;
}

static void get_curr_path(void){
	struct path current_path;
	char *buffer;
	int len_read = sizeof(char)*100;

	current_path = current->fs->pwd;
	path_get(&current_path);
	buffer = (char*) kmalloc (GFP_KERNEL, len_read);
	cwd = d_path(&current_path, buffer, len_read);
}

static void set_abs_script_path(void){
	strcat(cwd, "/");
	abs_script_path = (char*) kmalloc (GFP_KERNEL, sizeof(cwd)+sizeof(SCRIPT_NAME));
	strcpy(abs_script_path, cwd);
	strcat(abs_script_path, SCRIPT_NAME);
}


int init_routine(void){
	int err;

	get_curr_path();
	printk(KERN_ALERT "working dir: %s\n", cwd);

	set_abs_script_path();
	printk(KERN_ALERT "abs script path: %s\n", abs_script_path);

	printk("Initializing statux...\n");

	if(processIdsCount > SIZE_PARAMS){
		printk(KERN_WARNING "Too many arguments passed into statux_main. Currently max: %d\n", SIZE_PARAMS);
		return -EINVAL;
	}

	int machine_register_status = machine_register(NULL);

	if(machine_register_status != 0){
		printk(KERN_ERR "Failed to register machine to server! Status: %d\n", machine_register_status >> 8);
		remove_machine(NULL);
		return machine_register_status;
	};

	msleep(500);

	if(processIdsCount != 0){
		process_register(pids);
		process_update_task = kthread_run(process_update, pids, "process_update_thread");
		if(IS_ERR(process_update_task)){
			printk(KERN_ERR "ERROR: Cannot create process_update_thread!\n");
			err = PTR_ERR(process_update_task);
			process_update_task = NULL;
			return err;
		}
	}
	else{
		printk(KERN_INFO "No PiDs provided. Process registration and update skipped!\n");
	}

	memory_update_task = kthread_run(memory_update, NULL, "memory_update_thread");
	if(IS_ERR(memory_update_task)){
		printk(KERN_ERR "ERROR: Cannot create memory_update_thread!\n");
		err = PTR_ERR(memory_update_task);
		memory_update_task = NULL;
		return err;
	}

	printk("Main statux module loaded!\n");
	return 0;
}

void exit_routine(void){
	if(processIdsCount != 0)
		kthread_stop(process_update_task);
	kthread_stop(memory_update_task);
	if(keep != 1)
		remove_machine(NULL);
	printk("Main statux module unloaded!\n");
}

module_init(init_routine);
module_exit(exit_routine);
MODULE_LICENSE(LICENSE);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);
