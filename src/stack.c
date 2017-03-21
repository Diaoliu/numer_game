#include <stdlib.h>
#include "stack.h"

#define OPERATION(operand) \
	do { \
		self->top -= 2; \
		if (self->top >= self->len - 1 || self->top < 0) \
			return -1; \
		self->stack[self->top] operand self->stack[self->top + 1]; \
		self->top++; \
		return 0; \
	} while(0) \

stack_t stack_init(int len) {
	int *p = malloc(sizeof(int) * len);
	stack_t stack = { 0, len, p };
	return stack;
}

int stack_push(stack_t *self, int num) {
	if (self->top >= self->len || self->top < 0)
		return -1;
	self->stack[self->top++] = num;
	return 0;
}

int stack_add(stack_t *self) {
	OPERATION(+=);
}

int stack_sub(stack_t *self) {
	OPERATION(-=);
}

int stack_multi(stack_t *self) {
	OPERATION(*=);
}

int stack_divide (stack_t *self) {
	OPERATION(/=);
}

int stack_get(stack_t *self) {
	if (self->top != 1)
	{
		return -1;
	} else {
		return self->stack[self->top - 1];
	}
}

void free_stack(stack_t *self)
{
	free(self->stack);
}