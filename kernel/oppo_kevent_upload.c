/*
 * oppo_kevent_upload.c - for kevent action upload,root action upload to user layer
 *  author by wangzhenhua,Plf.Framework
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <net/net_namespace.h>
#include <linux/proc_fs.h>
#include <net/sock.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#include <linux/oppo_kevent.h>

#ifdef CONFIG_OPPO_KEVENT_UPLOAD

static struct sock *netlink_fd = NULL;
static volatile u32 kevent_pid;

/* send to user space */
int kevent_send_to_user(struct kernel_packet_info *userinfo)
{
	int ret, size;
	unsigned int o_tail;
	struct sk_buff *skbuff;
	struct nlmsghdr *nlh;
	struct kernel_packet_info *packet;

	size = NLMSG_SPACE(sizeof(struct kernel_packet_info) + userinfo->payload_length);
	/* protect payload too long problem */
	if (size > 2048) {
		size = 2048;
	}

	/*allocate new buffer cache */
	skbuff = alloc_skb(size, GFP_ATOMIC);
	if (skbuff == NULL) {
		printk(KERN_ERR "kevent_send_to_user: skbuff alloc_skb failed\n");
		return -1;
	}

	/* fill in the data structure */
	nlh = nlmsg_put(skbuff, 0, 0, 0, size - sizeof(*nlh), 0);
	if (nlh == NULL) {
		printk(KERN_ERR "nlmsg_put failaure\n");
		nlmsg_free(skbuff);
		return -1;
	}

	o_tail = skbuff->tail;

	/* use struct kernel_packet_info for data of nlmsg */
	packet = NLMSG_DATA(nlh);
	memset(packet, 0, size);

	/* copy the payload content */
	memcpy(packet, userinfo, size);

	//compute nlmsg length
	nlh->nlmsg_len = skbuff->tail - o_tail;

	/* set control field,sender's pid */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0))
	NETLINK_CB(skbuff).pid = 0;
#else
	NETLINK_CB(skbuff).portid = 0;
#endif

	NETLINK_CB(skbuff).dst_group = 0;

	/* send data */
	ret = netlink_unicast(netlink_fd, skbuff, kevent_pid, MSG_DONTWAIT);
	if(ret < 0){
		printk(KERN_ERR "[kernel space] netlink_unicast: can not unicast skbuff\n");
		return 1;
	}

	return 0;
}

/* kernel receive message from user space */
void kernel_kevent_receive(struct sk_buff *__skbbr)
{
	struct sk_buff *skbu;
	struct nlmsghdr *nlh = NULL;

	skbu = skb_get(__skbbr);

	if (skbu->len >= sizeof(struct nlmsghdr)) {
		nlh = (struct nlmsghdr *)skbu->data;
		if((nlh->nlmsg_len >= sizeof(struct nlmsghdr))
			&& (__skbbr->len >= nlh->nlmsg_len)){
			kevent_pid = nlh->nlmsg_pid;
		}
	}
	kfree_skb(skbu);
}

int __init netlink_kevent_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.groups = 0,
		.input  = kernel_kevent_receive,
	};

	netlink_fd = netlink_kernel_create(&init_net, NETLINK_OPPO_KEVENT, &cfg);
	if (!netlink_fd) {
		printk(KERN_ERR "Can not create a netlink socket\n");
		return -1;
	}
	return 0;
}

void __exit netlink_kevent_exit(void)
{
	sock_release(netlink_fd->sk_socket);
}

module_init(netlink_kevent_init);
module_exit(netlink_kevent_exit);
MODULE_LICENSE("GPL");

#endif /* CONFIG_OPPO_KEVENT_UPLOAD */

