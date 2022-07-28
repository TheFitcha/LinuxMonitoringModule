#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/socket.h>
#include<linux/net.h>
#include<linux/in.h>
#include<net/net_namespace.h>

#define AUTHOR "Filip Balder"
#define DESCRIPTION "Statux main module."
#define LICENSE "GPL"

static int send_hello(void *arg){
	char functionName[20] = "send_hello";

	printk(KERN_INFO "%s thread id: %d\n", functionName, current->pid);
	
	struct socket *sock;

	int sock_err = sock_create_kern(&init_net, PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
	if(sock_err < 0) {
		printk(KERN_ERR "Error while creating (%s) socket!\n", functionName);
		return sock_err;
	}
	printk(KERN_INFO "Successfully created socket in kernel (%s)!\n", functionName);

	struct sockaddr_in sockaddr = {
		.sin_family = AF_INET,
		.sin_port = htons (5000),
		//.sin_addr.s_addr = inet_pton("192.168.1.52"),
		.sin_addr = { htonl("192.168.1.52") }
	};
	/* int ipAddrErr = inet_aton("192.168.1.52", &sockaddr.sin_addr.s_addr);
	if(ipAddrErr == 0){
		printk(KERN_ERR "Invalid IP address!");
		return -1;
	}*/

	struct msghdr msg = {
		.msg_name = &sockaddr,
		.msg_flags = 0,
		.msg_control = NULL,
		.msg_controllen = 0
	};

	struct kvec vec;
	char helloMsg[20] = "Hello from kernel!";
	int msgStatus = kernel_sendmsg(sock, &msg, &vec, 1, sizeof(helloMsg));
	if(msgStatus == -1){
		printk(KERN_ERR "Error while sending message! (%s)\n", functionName);
		return -1;
	}

	printk(KERN_INFO "Successfully sent message %s (%s).\n", helloMsg, functionName);

	sock_release(sock);

	printk(KERN_INFO "Socket released! (%s)\n", functionName);
	
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

	printk("Main statux module loaded!");
	return 0;
}

void exit_routine(void){
	printk("Main statux module unloaded!");
}

module_init(init_routine);
module_exit(exit_routine);
MODULE_LICENSE(LICENSE);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);
