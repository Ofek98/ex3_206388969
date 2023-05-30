#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>   
#include <linux/module.h>   
#include <linux/fs.h>       
#include <linux/uaccess.h> 
#include <linux/string.h> 
#include <linux/slab.h> 
#include "message_slot.h"


MODULE_LICENSE("GPL");

static SLOT *HEAD;
void slot_insert(SLOT *new){
    if (HEAD == NULL){
        HEAD = new;
    }
    else{
        HEAD->prev = new;
        new->next = HEAD;
        HEAD = new;
    }
}

void channel_insert(SLOT *slot, CHANNEL *channel){
    if (slot->head == NULL){
        slot->head = channel;
    }
    else{
        (slot->head)->prev = channel;
        channel->next = slot->head;
        slot->head = channel;
    }
}

SLOT* minor_lookup(int minor){
    SLOT* curr;
    curr = HEAD;
    while (curr != NULL){
        if (curr->minor == minor){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

CHANNEL* set_channel(SLOT* slot, unsigned long channel_id){
    CHANNEL *curr, *new;
    curr = slot->head;
    while(curr != NULL){
        if (curr->id == channel_id){
            return curr;
        }
        curr = curr->next;
    }
    new = (CHANNEL*) kmalloc(sizeof(CHANNEL),GFP_KERNEL);
    if (new == NULL){
        return new;
    }
    new -> id = channel_id;
    new -> active_msg_len = 0;
    channel_insert(slot,new);
    return new;
}

void free_channel_list(CHANNEL *head){//code from a software project lecture
    if (head != NULL){
        free_channel_list(head->next);
        kfree(head->buf);
        kfree(head);
    }  
}

void free_slots_list(SLOT *slot){
    if (slot != NULL){
        free_slots_list(slot->next);
        free_channel_list(slot->head);
        kfree(slot);
    }
}

static int device_open( struct inode* inode,
                        struct file*  file){
    int minor;
    SLOT *new;
    minor = iminor(inode);
    if (minor_lookup(minor) == NULL){
        new = (SLOT*)kmalloc(sizeof(SLOT),GFP_KERNEL);
        if (new == NULL){
            return -ENOMEM; 
    }
        new->minor = minor;
        new->head = NULL;
        slot_insert(new);
    }
    file->private_data = NULL;
    return 0;
}


static ssize_t device_read( struct file* file, char __user* buffer, size_t length, loff_t* offset){
    CHANNEL *channel; 
    int i;
    char *new;

    channel = (CHANNEL*) file->private_data;
    
    if (channel == NULL){
        return -EINVAL;
    }
    if (channel->active_msg_len == 0){
        return -EWOULDBLOCK;
    }
    if (length < channel->active_msg_len){
        return -ENOSPC;
    }
    new = (char*) kmalloc(BUF_LEN*sizeof(char), GFP_KERNEL);
    if (new == NULL){
        return -ENOMEM; 
    }
    for (i=0; i<BUF_LEN; i++){
        if (get_user(new[i], &buffer[i])<0){
            return -EFAULT;
        }
    }
    kfree(new);
    for (i=0; i < channel->active_msg_len; i++){
        if(put_user((channel->buf)[i], &buffer[i])<0){
            return -EFAULT;
        }
    }  
    return i;
}

static ssize_t device_write( struct file* file, const char __user* buffer, size_t length,loff_t* offset) {
    char *tmp, *new;
    int i;
    CHANNEL *channel;
    channel = (CHANNEL*) file->private_data;
    if (channel == NULL){
        return -EINVAL;
    }
    if ((length <= 0) || (length > BUF_LEN)){
        return -EMSGSIZE;
    }
    tmp = channel -> buf;
    new = (char*) kmalloc(length*sizeof(char), GFP_KERNEL);
    if (new == NULL){
        return -ENOMEM; 
    }
    for (i=0; i<length; i++){
        if (get_user(new[i], &buffer[i])<0){
            return -EFAULT;
        }
    }
    channel->buf = new;
    channel->active_msg_len = length;
    kfree(tmp);
    return length;
}

static long device_ioctl( struct   file* file, unsigned int   ioctl_command_id, unsigned long  ioctl_param ){
    CHANNEL *active_channel;
    SLOT *slot;
    if (ioctl_command_id != MSG_SLOT_CHANNEL){
        return -EINVAL;
    }
    if (ioctl_param == 0){
        return -EINVAL;
    }
    slot = minor_lookup(iminor(file->f_inode));
    if (slot == NULL){
        return -EFAULT;
    }

    active_channel = set_channel(slot, ioctl_param);
    if (active_channel == NULL){
        return -ENOMEM; 
    }
    file->private_data = active_channel;
    return 0;
}


struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
};

static int __init simple_init(void){
    int rc;
    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
    if( rc < 0 ) {
        printk(KERN_ALERT "%s registraion failed for  %d\n",DEVICE_FILE_NAME, MAJOR_NUM );
        return rc;
    }
    HEAD = NULL;
    return 0;
}

static void __exit simple_cleanup(void)
{
    free_slots_list(HEAD);
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);