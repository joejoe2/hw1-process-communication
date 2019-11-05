#include "com_kmodule.h"

struct sock *nl_sk = NULL;

struct mutex my_mutex;

struct mailboxlist list=
{
    NULL,NULL
};


static char* registery(char* id, char* type)
{
    long *uid=vmalloc(sizeof(long));
    if(uid!=NULL)
        kstrtol(id,10,uid);
    else
        return "Fail";

    if(list.head==NULL&&list.tail==NULL) //empty
    {
        struct mailbox *box=vmalloc(sizeof(struct mailbox));
        struct mailboxwrapper* wrap=vmalloc(sizeof(struct mailboxwrapper));

        wrap->id=*uid;
        if(strcmp(type,"queued")==0)
        {
            box->type=1;
        }
        else
        {
            box->type=0;
        }
        box->msg_data_count=0;
        box->msg_data_head=NULL;
        box->msg_data_tail=NULL;

        wrap->box=box;
        wrap->next=NULL;

        list.head=wrap;
        list.tail=list.head;
    }
    else //not empty
    {
        struct mailboxwrapper* wrap=list.head;
        struct mailbox *box;
        //test exist
        while(wrap!=NULL)
        {
            if(wrap->id==*uid) //if exist
            {
                vfree(uid);
                return "Fail";
            }
            wrap=wrap->next;
        }

        //not exist => new
        wrap=vmalloc(sizeof(struct mailboxwrapper));
        if(wrap==NULL)
        {
            vfree(uid);
            return "Fail";
        }


        wrap->id=*uid;
        box=vmalloc(sizeof(struct mailbox));
        if(strcmp(type,"queued")==0)
        {
            box->type=1;
        }
        else
        {
            box->type=0;
        }
        box->msg_data_count=0;
        box->msg_data_head=NULL;
        box->msg_data_tail=NULL;

        wrap->box=box;
        wrap->next=NULL;
        list.tail->next=wrap;
        list.tail=wrap;
    }

    vfree(uid);
    return "Success";
}

static char* getmsg(char* id)
{
    long *uid=vmalloc(sizeof(long));
    kstrtol(id,10,uid);
    struct mailboxwrapper* wrap=list.head;
    struct mailbox *target=NULL;
    //test exist
    while(wrap!=NULL)
    {
        if(wrap->id==*uid) //if exist
        {
            target=wrap->box;
            break;
        }
        wrap=wrap->next;
    }
    if(target!=NULL) //exist
    {
        if(target->type==1)
        {
            //queued need pop
            if(target->msg_data_count==0)
            {
                //empty
                vfree(uid);
                return "Fail";
            }
            else if(target->msg_data_count==1)
            {
                target->msg_data_count--;
                char* result=vmalloc(sizeof(target->msg_data_head->buf)+1);
                strcpy(result,target->msg_data_head->buf);
                vfree(target->msg_data_head);
                target->msg_data_head=NULL;
                target->msg_data_tail=target->msg_data_head;

                vfree(uid);
                return result;
            }
            else if(target->msg_data_count==2)
            {
                target->msg_data_count--;
                char* result=vmalloc(sizeof(target->msg_data_tail->buf)+1);
                strcpy(result,target->msg_data_tail->buf);
                vfree(target->msg_data_tail);
                target->msg_data_tail=target->msg_data_head;
                target->msg_data_head->next=NULL;

                vfree(uid);
                return result;
            }
            else if(target->msg_data_count==QUEUE_SIZE)
            {
                target->msg_data_count--;
                char* result=vmalloc(sizeof(target->msg_data_tail->buf)+1);
                strcpy(result,target->msg_data_tail->buf);
                vfree(target->msg_data_tail);
                target->msg_data_head->next->next=NULL;
                target->msg_data_tail=target->msg_data_head->next;

                vfree(uid);
                return result;
            }
        }
        else
        {
            //unqueued not consume
            if(target->msg_data_count==0)
            {
                //empty
                vfree(uid);
                return "Fail";
            }
            else
            {
                //not empty
                char* result=vmalloc(sizeof(target->msg_data_head->buf)+1);
                strcpy(result,target->msg_data_head->buf);

                vfree(uid);
                return result;
            }
        }
    }
    else
    {
        //not exist
        vfree(uid);
        return "Fail";
    }

    vfree(uid);
    return "Success";
}

static char* storemsg(char* id, char* msg)
{
    struct msg_data *data;
    long *uid=vmalloc(sizeof(long));
    struct mailboxwrapper *wrap=list.head;
    struct mailbox *target=NULL;
    kstrtol(id,10,uid);
    int len=strlen(msg);
    len=len>256?256:len;

    //test exist
    while(wrap!=NULL)
    {
        if(wrap->id==*uid) //if exist
        {
            target=wrap->box;
            break;
        }
        wrap=wrap->next;
    }
    if(target!=NULL) //exist
    {
        if(target->type==1) //queued
        {
            if(target->msg_data_count<QUEUE_SIZE)
            {
                data=vmalloc(sizeof(struct msg_data));
                memset(data->buf, 0, sizeof(data->buf));//clear
                strncpy(data->buf,msg,len);//put

                if(target->msg_data_count==0)//push to empty queue
                {
                    data->next=NULL;
                    target->msg_data_head=data;
                    target->msg_data_tail=target->msg_data_head;
                }
                else//push to not empty queue
                {
                    data->next=target->msg_data_head;
                    target->msg_data_head=data;
                }
                target->msg_data_count++;

                vfree(uid);
                return "Success";
            }
            else
            {
                vfree(uid);
                return "Fail";
            }
        }
        else  //unqueued
        {

            if(target->msg_data_count==0)//empty
            {
                data=vmalloc(sizeof(struct msg_data));
                target->msg_data_count=1;
                data->next=NULL;
                target->msg_data_head=data;
                target->msg_data_tail=data;
            }
            else//not empty
            {
                data=target->msg_data_head;
            }

            memset(data->buf, 0, sizeof(data->buf));//clear
            strncpy(data->buf,msg,len);//put


            vfree(uid);
            return "Success";
        }
    }
    else  //not exist
    {
        vfree(uid);
        return "Fail";
    }

}

static char* getid(char* in)
{
    bool f=false;
    bool fi=false;
    char *id=vmalloc(sizeof("1000")+1);

    int i=0;
    int c=0;
    for(i=in[2]=='g'?16:0; i<strlen(in); i++)
    {
        if(in[i]=='='||in[i]==' '&&!f)
        {
            f=!f;
            continue;
        }
        if(in[i]==','||in[i]==' '||in[i]=='\n'&&f)
        {
            fi=!fi;
            break;
        }
        if(f)
        {
            id[c]=in[i];
            c++;
        }

    }
    id[c]='\0';
    return id;
}

static char* gettxt(char* in)
{
    int c=0;
    char *txt=vmalloc(strlen(in)+1);
    int i=0;
    int j=0;
    for(i=0; i<strlen(in); i++)
    {
        if(in[i]==' '&&c<2)
        {
            c++;
            continue;
        }

        if(c==2)
        {
            txt[j]=in[i];
            j++;
        }
    }
    txt[j]='\0';
    return txt;
}

static char* getty(char* in)
{
    int c=0;
    char *txt=vmalloc(sizeof("unqueued")+1);
    int i=0;
    int j=0;
    for(i=0; i<strlen(in); i++)
    {
        if(in[i]=='='&&c<2)
        {
            c++;
            continue;
        }

        if(c==2)
        {
            txt[j]=in[i];
            j++;
        }
    }
    txt[j]='\0';
    return txt;
}

static void nl_recv_msg(struct sk_buff *skb)
{
    mutex_lock(&my_mutex);//lock

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char* cmd;
    char *msg;
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);


    nlh = (struct nlmsghdr *)skb->data;

    cmd=(char *)nlmsg_data(nlh);

    //process cmd...
    if(strstr(cmd,"Send")!=NULL)
    {
        msg=storemsg(getid(cmd),gettxt(cmd));
    }
    else if(strstr(cmd,"Recv")!=NULL)
    {
        msg=getmsg(getid(cmd));
    }
    else if(strstr(cmd,"Registration.")!=NULL)
    {
        msg=registery(getid(cmd),getty(cmd));
    }
    else
    {
        msg="Fail";
    }
    //
    msg_size = strlen(msg)+1;
    pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */

    strncpy(nlmsg_data(nlh), msg, msg_size);
    vfree(msg);
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending back to user\n");

    //

    mutex_unlock(&my_mutex);//unlock

}

static int __init com_kmodule_init(void)
{
    struct netlink_kernel_cfg cfg =
    {
        .input = nl_recv_msg,
    };

    printk(KERN_INFO "Enter module. Hello world!\n");


    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    mutex_init(&my_mutex);
    printk("init mutex ok\n");

    return 0;
}

static void __exit com_kmodule_exit(void)
{
    printk(KERN_INFO "Exit module. Bye~\n");
    netlink_kernel_release(nl_sk);
}

module_init(com_kmodule_init);
module_exit(com_kmodule_exit);
