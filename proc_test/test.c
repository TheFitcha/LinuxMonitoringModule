#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<asm/uaccess.h>

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "buffer1k"

#define BUFSIZE 1024

static ssize_t p_write(struct file *file, const char *ubuf, size_t count, loff_t *ppos){
/*	char buf[BUFSIZE];
	if(*ppos > 0 || count > BUFSIZE){
		printk(KERN_WARNING "ppos or count problem\n");
		return -EFAULT;
	}

	if(copy_from_user(buf, ubuf, count)){
		printk(KERN_WARNING "copy_from_user error!\n");
		return -EFAULT;
	}

	int c = strlen(buf);
	*ppos = c;*/

	printk("WRITE HANDLER! Count: %zu\n", count);
	return count;
}


static ssize_t p_read(struct file *file, char *ubuf, size_t count, loff_t *ppos){
	/*char buf[BUFSIZE];

	if(*ppos > 0 || count < BUFSIZE)
		return 0;

	if(copy_to_user(ubuf, buf, count))
		return -EFAULT;*/

	printk("READ HANDLER! Count: %zu\n", count);
	*ppos = 6;
	return 6;
}

static struct proc_dir_entry *proc;

static struct proc_ops proc_operations = {
	proc_read: &p_read,
	proc_write: &p_write
};


int init(void){
	proc = proc_create(PROCFS_NAME, 0666, NULL, &proc_operations);
	if (proc == NULL){
		proc_remove(proc);
		printk(KERN_ERR "Error: could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}

	return 0;
}

void cleanup(void){
	proc_remove(proc);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

module_init(init);
module_exit(cleanup);
MODULE_LICENSE("GPL");

