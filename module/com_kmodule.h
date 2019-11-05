#ifndef COM_KMODULE_H
#define COM_KMODULE_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/mutex.h>
#define NETLINK_USER 31
#define QUEUE_SIZE 3

struct mailbox
{
    //0: unqueued
    //1: queued
    unsigned char type;
    unsigned char msg_data_count;
    struct msg_data *msg_data_head;
    struct msg_data *msg_data_tail;

};
struct msg_data
{
    char buf[256];
    struct msg_data *next;
};
struct mailboxlist
{
    struct mailboxwrapper* head;
    struct mailboxwrapper* tail;
};
struct mailboxwrapper
{
    struct mailboxwrapper* next;
    struct mailbox* box;
    long id;
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Apple pie");
MODULE_DESCRIPTION("A Simple Hello World module");

#endif  //ifndef COM_KMODULE_H
