#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/err.h>
#include "queue.h"

struct msg_list{
    struct list_head list;
    char* data;
};
#define USE_MUTEX

#ifdef USE_MUTEX
static struct mutex local_mutex;
#else
DEFINE_SPINLOCK(local_spinlock);
#endif

LIST_HEAD(msg_head);

void queue_init(void)
{
#ifdef USE_MUTEX
    mutex_init(&local_mutex);
#endif
}

void queue_add_node(char *text)
{
    struct msg_list* temp_node = kmalloc(sizeof(struct msg_list), GFP_KERNEL);
    int l = strlen(text);
    temp_node->data = kmalloc(l+1, GFP_KERNEL);

    memcpy(temp_node->data, text, l);
    temp_node->data[l] = 0;
    /*Init the list within the struct*/
    INIT_LIST_HEAD(&temp_node->list);
#ifdef USE_MUTEX
    mutex_lock(&local_mutex);
#else
    spin_lock(&local_spinlock);
#endif
    /*Add Node to Linked List*/
    list_add_tail(&temp_node->list, &msg_head);
#ifdef USE_MUTEX
    mutex_unlock(&local_mutex);
#else
    spin_unlock(&local_spinlock);
#endif
}

char* queue_get_node(void)
{
    struct msg_list *current_node;
#ifdef USE_MUTEX
    mutex_lock(&local_mutex);
#else
    spin_lock(&local_spinlock);
#endif
    list_for_each_entry(current_node, &msg_head, list) {
        list_del(&current_node->list);
        break;
    }
#ifdef USE_MUTEX
    mutex_unlock(&local_mutex);
#else
    spin_unlock(&local_spinlock);
#endif
    return current_node->data;
}

char* stack_get_node(void)
{
    struct msg_list *current_node;
#ifdef USE_MUTEX
    mutex_lock(&local_mutex);
#else
    spin_lock(&local_spinlock);
#endif
    list_for_each_entry_reverse(current_node, &msg_head, list) {
        list_del(&current_node->list);
        break;
    }
#ifdef USE_MUTEX
    mutex_unlock(&local_mutex);
#else
    spin_unlock(&local_spinlock);
#endif
    return current_node->data;
}

int queue_is_empty(void)
{
    int ret;
#ifdef USE_MUTEX
    mutex_lock(&local_mutex);
#else
    spin_lock(&local_spinlock);
#endif
    ret = list_empty(&msg_head);
#ifdef USE_MUTEX
    mutex_unlock(&local_mutex);
#else
    spin_unlock(&local_spinlock);
#endif
    return ret;
}

