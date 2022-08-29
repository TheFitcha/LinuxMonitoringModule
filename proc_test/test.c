#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<asm/uaccess.h>
#include<linux/slab.h>

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "statux_info"

#define BUFSIZE 1024

char buf[BUFSIZE];

static ssize_t p_write(struct file *file, const char *ubuf, size_t count, loff_t *ppos){
	if(*ppos > 0 || count > BUFSIZE){
		printk(KERN_WARNING "ppos or count problem\n");
		return -EFAULT;
	}

	char *temp_buf = kmalloc(strlen(ubuf), GFP_USER);

	if(copy_from_user(temp_buf, ubuf, count)){
		printk(KERN_WARNING "copy_from_user error!\n");
		return -EFAULT;
	}

	strcat(buf, temp_buf);

	int buf_len = strlen(buf);
	*ppos = buf_len;

	printk("WRITE HANDLER! Count: %d\n", buf_len);
	return buf_len;
}


static ssize_t p_read(struct file *file, char *ubuf, size_t count, loff_t *ppos){
	/*if(*ppos > 0 || count < BUFSIZE)
		return 0;*/
	if(*ppos > 0){
		*ppos = 0;
		return 0;
	}

	int buf_len = strlen(buf);

	if(copy_to_user(ubuf, buf, buf_len))
		return -EFAULT;


	printk("READ HANDLER! Count: %d ppos: %lld\n ", buf_len, *ppos);

	*ppos = buf_len;
	return buf_len;
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

