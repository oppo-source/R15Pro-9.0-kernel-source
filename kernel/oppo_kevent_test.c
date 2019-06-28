/*
 * oppo_kevent_test.c - for kevent action upload test
 *  author by wangzhenhua,Plf.Framework
 */
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#include <linux/oppo_kevent.h>
#include <linux/netlink.h>

#ifdef CONFIG_OPPO_KEVENT_TEST
#define MAX_PAYLOAD_LENGTH			2048
static char *str = NULL;
/*
struct payload
{
	uid_t uid;
}__attribute__((packed));

int kevent_proc_show(struct seq_file *m, void *v)
{
	struct kernel_packet_info *user_msg_info;
	struct payload *payload_uid;
	char log_tag[32] = "2100001";
	char event_id[32] = "user_location";
	void* buffer = NULL;
	int size;

	size = NLMSG_SPACE(sizeof(struct kernel_packet_info) + sizeof(struct payload));
	printk(KERN_INFO "kevent_send_to_user, kevent_proc_show:size=%d\n", size);
	buffer = kmalloc(size, GFP_ATOMIC);
	memset(buffer, 0, size);
	user_msg_info = (struct kernel_packet_info *)buffer;
	user_msg_info->type = 0;

	memcpy(user_msg_info->log_tag, log_tag, strlen(log_tag) + 1);
	memcpy(user_msg_info->event_id, event_id, strlen(event_id) + 1);

	user_msg_info->payload_length = sizeof(struct payload);

	payload_uid = (struct payload *)(buffer + sizeof(struct kernel_packet_info));
	payload_uid->uid = 2000;

	kevent_send_to_user(user_msg_info);
	msleep(20);
	kfree(buffer);
	return 0;
}
*/

static int kevent_proc_show(struct seq_file *m, void *v)
{
	struct kernel_packet_info *user_msg_info;
	char log_tag[32] = "2100001";
	char event_id[32] = "user_location";
	void* buffer = NULL;
	int len, size;

	seq_printf(m, "%s", str);

	len = strlen(str);

	size = NLMSG_SPACE(sizeof(struct kernel_packet_info) + len + 1);
	printk(KERN_INFO "kevent_send_to_user, kevent_proc_show:size=%d\n", size);

	buffer = kmalloc(size, GFP_ATOMIC);
	memset(buffer, 0, size);
	user_msg_info = (struct kernel_packet_info *)buffer;
	user_msg_info->type = 1;

	memcpy(user_msg_info->log_tag, log_tag, strlen(log_tag) + 1);
	memcpy(user_msg_info->event_id, event_id, strlen(event_id) + 1);

	user_msg_info->payload_length = len + 1;
	memcpy(user_msg_info->payload, str, len + 1);

	kevent_send_to_user(user_msg_info);
	msleep(20);
	kfree(buffer);
	return 0;
}

/*
* file_operations->open
*/
static int kevent_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, kevent_proc_show, NULL);
}

/*
* file_operations->write
*/
static ssize_t kevent_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
	char *tmp = kzalloc((count + 1), GFP_KERNEL);
	if (!tmp) {
		return -ENOMEM;
	}

	if (copy_from_user(tmp, buffer, count)) {
		kfree(tmp);
		return -EFAULT;
	}

	str = tmp;

	return count;
}

static struct file_operations kevent_fops = {
	.open = kevent_proc_open,
	.release = single_release,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = kevent_proc_write,
};

static int __init kevent_init(void)
{
	struct proc_dir_entry* file;

	/* create proc and file_operations */
	file = proc_create("kevent", 0666, NULL, &kevent_fops);
	if (!file) {
		return -ENOMEM;
	}

	str = kzalloc((MAX_PAYLOAD_LENGTH + 1), GFP_KERNEL);
	memset(str, 0, MAX_PAYLOAD_LENGTH);
	strcpy(str, "kevent");

	return 0;
}

static void __exit kevent_exit(void)
{
	remove_proc_entry("kevent", NULL);
	kfree(str);
}

module_init(kevent_init);
module_exit(kevent_exit);

MODULE_LICENSE("GPL");
#endif /* CONFIG_OPPO_KEVENT_TEST */

