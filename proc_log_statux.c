#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/proc_fs.h>
#include<asm/uaccess.h>
#include<linux/slab.h>

#define AUTHOR "Filip Balder"
#define DESCRIPTION "Module that ensures entry in /proc virtual memory"
#define LICENSE "GPL"

#define LOG_NAME "statux_log"

#define BUFSIZE 1048576

char buf[BUFSIZE];

static ssize_t p_write_log(struct file *file, const char *ubuf, size_t count, loff_t *ppos){
	int buf_len;
	char *temp_buf;
	char erase_keyword[] = "clear_proc_contents";
	char erase_keyword_n[] = "clear_proc_contents\n";

	if(*ppos > 0 || count > BUFSIZE){
		printk(KERN_WARNING "ppos or count problem\n");
		return -EFAULT;
	}

	temp_buf = kmalloc(strlen(ubuf), GFP_USER);

	if(copy_from_user(temp_buf, ubuf, count)){
		printk(KERN_WARNING "copy_from_user error!\n");
		return -EFAULT;
	}

	if(strcmp(temp_buf, erase_keyword) == 0 || strcmp(temp_buf, erase_keyword_n) == 0){
		printk(KERN_INFO "Clearing statux_log contents!\n");
		strcpy(buf, "");
		buf_len = count;
		*ppos = 0;
	}
	else{
		strcat(buf, temp_buf);
		buf_len = strlen(buf);
		*ppos = buf_len;

	}
	printk("WRITE HANDLER STATUX LOG! Count: %d\n", buf_len);

	return buf_len;
}


static ssize_t p_read_log(struct file *file, char *ubuf, size_t count, loff_t *ppos){
	int buf_len;

	if(*ppos > 0){
		*ppos = 0;
		return 0;
	}

	buf_len = strlen(buf);

	if(copy_to_user(ubuf, buf, buf_len)){
		return -EFAULT;
	}

	printk("READ HANDLER STATUX LOG! Count: %d ppos: %lld\n ", buf_len, *ppos);

	*ppos = buf_len;
	return buf_len;
}


static struct proc_dir_entry *log;

static struct proc_ops proc_operations_log = {
	proc_read: &p_read_log,
	proc_write: &p_write_log
};


int init(void){
	log = proc_create(LOG_NAME, 0666, NULL, &proc_operations_log);
	if (log == NULL){
		proc_remove(log);
		printk(KERN_ERR "Error: could not initialize /proc/%s!\n", LOG_NAME);
		return -ENOMEM;
	}

	printk(KERN_INFO "Proc statux entries created (/proc/%s)\n", LOG_NAME);
	return 0;
}

void cleanup(void){
	proc_remove(log);
	printk(KERN_INFO "/proc/%s removed\n", LOG_NAME);
}

module_init(init);
module_exit(cleanup);
MODULE_LICENSE(LICENSE);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_AUTHOR(AUTHOR);

