#include "com_app.h"
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

const char* registery(char* id,char* type)
{
    char *t= malloc (strlen ("Registration. id=")+strlen (", type=") + strlen (id) + strlen (type) + 1);
    strcat(t,"Registration. id=");
    strcat(t,id);
    strcat(t,", type=");
    strcat(t,type);
    //printf("%s\n",t);
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
    {
        return "Fail";
    }
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), t);
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    sendmsg(sock_fd, &msg, 0);
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    close(sock_fd);
    free(t);
    return NLMSG_DATA(nlh);
}

const char* recvmesg(char* id)
{
    char *t= malloc(strlen("Recv ")+strlen(id)+1);
    strcat(t,"Recv ");
    strcat(t,id);

    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
    {
        return "Fail";
    }
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), t);
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    sendmsg(sock_fd, &msg, 0);
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    close(sock_fd);
    free(t);
    return NLMSG_DATA(nlh);
}

const char* sendmesg(char* mesg)
{
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
    {
        return "Fail";
    }
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), mesg);
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    sendmsg(sock_fd, &msg, 0);
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    close(sock_fd);
    return NLMSG_DATA(nlh);
}

int main(int argc, char *argv[])
{
    char* id=argv[1];
    char* type;
    if(strcmp(argv[2],"queued")==0)
    {
        type="queued";
    }
    else
    {
        type="unqueued";
    }
    char* result=(char*)registery(id,type);
    printf("%s\n",result);
    if(strcmp(result,"Fail")==0)
    {
        exit(-1);
    }
    while(1)
    {
        char cmd[1024];
        gets(cmd);

        if(strcmp(cmd,"Recv")==0)
        {
            printf("%s\n",recvmesg(id));
        }
        else if(strstr(cmd,"Send")!=NULL)
        {
            printf("%s\n",sendmesg(cmd));
        }
        else
        {
            printf("Fail\n");
        }
    }


}

