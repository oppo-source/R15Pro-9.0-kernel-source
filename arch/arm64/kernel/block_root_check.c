/*
 * block_root_check.c - for root action upload to user layer and reboot phone
 *  author by wangzhenhua,Plf.Framework
 */
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#include <linux/oppo_kevent.h>
#include <linux/netlink.h>
#ifdef CONFIG_OPPO_KEVENT_UPLOAD

struct payload
{
	uid_t uid;
}__attribute__((packed));

void oppo_root_check_succ(uid_t call_uid)
{
	struct kernel_packet_info *user_msg;
	struct payload *payload_id;
	char logtag[32] = "kernel_event";
	char eventid[20] = "root_check";
	int size;
	void* buf = NULL;

	size = NLMSG_SPACE(sizeof(struct kernel_packet_info) + sizeof(struct payload));
	buf = kmalloc(size, GFP_ATOMIC);

	memset(buf, 0, size);
	user_msg = (struct kernel_packet_info *)buf;

	user_msg->type = 0;
	user_msg->payload_length = sizeof(struct payload);

	memcpy(user_msg->log_tag, logtag, strlen(logtag) + 1);
	memcpy(user_msg->event_id, eventid, strlen(eventid) + 1);

	payload_id =(struct payload *)(buf + sizeof(struct kernel_packet_info));
	payload_id->uid = call_uid;

	kevent_send_to_user(user_msg);

	kfree(buf);

	msleep(5000);

	return;
}

#endif

#ifdef CONFIG_OPPO_ROOT_CHECK
void oppo_root_reboot(void)
{
	int i=0;
	printk(KERN_INFO "oppo_root_reboot,Rebooting in the phone..");
	for (i = 0; i < 10; i++) {
		panic("ROOT for panic");
		msleep(200);
	}
}
#endif /* CONFIG_OPPO_ROOT_CHECK */

