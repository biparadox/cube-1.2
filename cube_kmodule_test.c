#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <asm/atomic.h>
#include <linux/security.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "./include/errno.h"
#include "./include/data_type.h"
#include "./include/list.h"
#include "./include/alloc.h"
#include "./include/json.h"
#include "./include/string.h"
#include "./include/basefunc.h"
#include "./include/struct_deal.h"
#include "./include/crypto_func.h"

#define NETLINK_USER 31
struct sock *nl_sk = NULL;
int user_process_pid;
static struct task_struct *kthd;

/*
int eey_kthread_task(void *data)
{

	printk("KernelProgram: Entering: %s\n", __FUNCTION__);
//test----------------------------------------------------------------------------------------------

//end-----------------------------------------------------------------------------------------------

	while(!kthread_should_stop()){
		printk(KERN_ALERT "KernelProgram: %s is running!\n", __FUNCTION__);
		msleep(7000);
		//mdelay(100);
		//schedule_timeout(HZ);
	}

	return 0;
}
*/

static void eey_nl_send_msg(char *msg, int pid) {
	struct sk_buff *skb_out;
	struct nlmsghdr *nlh;
	int msg_size;
	int res;

	printk(KERN_ALERT "KernelProgram: Entering: %s\n", __FUNCTION__);

	msg_size = strlen(msg) + 1;
	skb_out = nlmsg_new(msg_size,GFP_ATOMIC);
	if(!skb_out)
	{
		printk(KERN_ALERT "KernelProgram: %s: Failed to allocate new skb\n", __FUNCTION__);
		return;
	}

	nlh = nlmsg_put(skb_out,0,0,0,msg_size,0);	//input some netlink message head
//	NETLINK_CB(skb_out).pid = 0;
	NETLINK_CB(skb_out).dst_group = 0;

	strncpy(nlmsg_data(nlh),msg,msg_size);

	res = nlmsg_unicast(nl_sk,skb_out,pid);
	if(res<0)
		printk(KERN_ALERT "KernelProgram: %s: Error sending to user\n", __FUNCTION__);

}

static void eey_nl_recv_msg(struct sk_buff *skb) {
	struct nlmsghdr *nlh;
	char *payload;
	
	printk(KERN_ALERT "KernelProgram: Entering: %s\n", __FUNCTION__);

	nlh = (struct nlmsghdr*)skb->data;

	payload = nlmsg_data(nlh);	//received msg from user
	user_process_pid = nlh->nlmsg_pid;	//
	
	printk(KERN_ALERT "KernelProgram: %s: Received msg payload:%s %d\n", 
				__FUNCTION__, (char*)(payload+sizeof(int)), (int)*payload);

	eey_nl_send_msg(payload,user_process_pid);

}

static int cube_kmodule_test_init(void)
{
	int err = 0;
	printk("KernelProgram: Entering: %s\n",__FUNCTION__);

	// netlink
	struct netlink_kernel_cfg cfg = {
		.input = eey_nl_recv_msg,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	
	if(!nl_sk) {
		printk(KERN_ALERT "KernelProgram: %s: Error creating socket.\n", __FUNCTION__);
		return -10;
	}

/*	 
	// kthread
	kthd = kthread_create(eey_kthread_task, NULL, "eey_kthread_task_test");
	if(IS_ERR(kthd)){	printk(KERN_ALERT "KernelProgram: %s: Unable to start kernel thread.\n", __FUNCTION__);	err = PTR_ERR(kthd);	kthd = NULL;	return err;	}
	wake_up_process(kthd);
*/

	return 0;
}

static void cube_kmodule_test_exit(void)
{
	printk("KernelProgram: Entering: %s\n",__FUNCTION__);

	//netlink
	sock_release(nl_sk->sk_socket);

/*
	//kthread
	if(kthd){	kthread_stop(kthd);	kthd = NULL;	}
*/

	printk(KERN_ALERT "KernelProgram: %s: cube module test exit.\n", __FUNCTION__);
}

module_init(cube_kmodule_test_init);
module_exit(cube_kmodule_test_exit);

MODULE_AUTHOR("Eeyore");
MODULE_DESCRIPTION("simple cube kernel test module");
//MODULE_LICENSE(GPL);
