#ifndef STACK_H
#define STACK_H

typedef int stackElement_t;

typedef struct Stack_s {
	stackElement_t *contents;
	int index;
	int maxSize;
} Stack_t;

Stack_t *NewStack(const int maxSize);
void DestroyStack(Stack_t *Stack);
int StackPush(Stack_t *Stack, stackElement_t input);
stackElement_t StackPop(Stack_t *Stack);
int StackIsFull(Stack_t *Stack);
int StackIsEmpty(Stack_t *Stack);

#endif