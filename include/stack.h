#ifndef STACK_H
#define STACK_H

typedef struct
{
	int top;
	int len;
	int *stack;
} stack_t;

stack_t stack_init(int len);
int stack_push(stack_t *self, int num);
int stack_add(stack_t *self);
int stack_sub(stack_t *self);
int stack_multi(stack_t *self);
int stack_divide (stack_t *self);
int stack_get(stack_t *self);
void free_stack(stack_t *self);

#endif