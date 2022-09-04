#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<asm/uaccess.h>
#include<linux/slab.h>

#define AUTHOR "Filip Balder"
#define DESCRIPTION "Module that ensures entry in /proc virtual memory"
#define LICENSE "GPL"

#define PROCFS_NAME "statux_info"

#define BUFSIZE 1048576

char buf[BUFSIZE];

static ssize_t p_write_info(struct file *file, const char *ubuf, size_t count, loff_t *ppos){
	int buf_len;
	char *temp_buf;
	char erase_keyword[] = "clear_proc_contents";
	char erase_keyword_n[] = "clear_proc_contents\n";

	if(count > BUFSIZE){
		printk(KERN_WARNING "buffer overload\n");
		return -EFAULT;
	}

	temp_buf = kmalloc(strlen(ubuf), GFP_USER);

	if(copy_from_user(temp_buf, ubuf, count)){
		printk(KERN_WARNING "copy_from_user error!\n");
		return -EFAULT;
	}

	if(strcmp(temp_buf, erase_keyword) == 0 || strcmp(temp_buf, erase_keyword_n) == 0){
		printk(KERN_INFO "Clearing statux_info contents!\n");
		strcpy(buf, "");
		buf_len = count;
		*ppos = 0;
	}
	else{
		strcat(buf, temp_buf);
		buf_len = strlen(buf);
		*ppos = buf_len;
	}

	printk("WRITE HANDLER STATUX INFO! Count: %d\n", buf_len);
	return buf_len;
}


static ssize_t p_read_info(struct file *file, char *ubuf, size_t count, loff_t *ppos){
	int buf_len;

	if(*ppos > 0){
		*ppos = 0;
		return 0;
	}

	buf_len = strlen(buf);

	if(copy_to_user(ubuf, buf, buf_len)){
		return -EFAULT;
	}

	printk("READ HANDLER STATUX INFO! Count: %d ppos: %lld\n ", buf_len, *ppos);

	*ppos = buf_len;
	return buf_len;
}


static struct proc_dir_entry *proc;

static struct proc_ops proc_operations_info = {
	proc_read: &p_read_info,
	proc_write: &p_write_info
};


int init(void){
	proc = proc_create(PROCFS_NAME, 0666, NULL, &proc_operations_info);
	if (proc == NULL){
		proc_remove(proc);
		printk(KERN_ERR "Error: could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}

	printk(KERN_INFO "Proc statux entries created (/proc/%s)\n", PROCFS_NAME);
	return 0;
}

void cleanup(void){
	proc_remove(proc);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

module_init(init);
module_exit(cleanup);
MODULE_LICENSE(LICENSE);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);

