#ifndef QUEUE_H
#define QUEUE_H


void queue_init(void);
void queue_add_node(char *text);
char* queue_get_node(void);
char* stack_get_node(void);
int queue_is_empty(void);
void queue_clean(void);

#endif