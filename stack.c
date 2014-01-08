#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

//unit test
/*int main(int argc, char *argv[])
{
	Stack_t Stack;
	initStack(&Stack, 16);
	
	int i=0;
	while(!(stackIsFull(&Stack)))
	{
		push(&Stack, i);
		i++;
	}	
	while(!(stackIsEmpty(&Stack)))
		printf("%d\n",pop(&Stack));

	return 0;
}*/

Stack_t *NewStack(const int maxSize)
{
	Stack_t *stack;
	stack = malloc(sizeof(Stack_t));
	stack->contents = malloc(sizeof(stackElement_t) * maxSize);

	stack->maxSize = maxSize;
	stack->index = -1;

	for (int i = 0; i < stack->maxSize; i++)
		stack->contents[i] = 0;

	return stack;
}

/*void StackInit(Stack_t *Stack, int maxSize)
{
	Stack->index = -1;
	Stack->maxSize = maxSize;
	Stack->contents = (stackElement_t *)malloc(Stack->maxSize * sizeof(stackElement_t));
	
	int i;
	for (i=0;i<Stack->maxSize;i++)
		Stack->contents[i] = 0;
}*/

void DestroyStack(Stack_t *Stack)
{
	free(Stack->contents);
	free(Stack);
}

int StackPush(Stack_t *Stack, stackElement_t input)
{
	if(!(StackIsFull(Stack)))
	{
		Stack->index++;
		Stack->contents[Stack->index] = input;
		return 1;
	}
	else
		return 0;
		
	//fail
	return -1;
}

stackElement_t StackPop(Stack_t *Stack)
{
	if (!(StackIsEmpty(Stack)))
	{
		stackElement_t popVal;
		popVal = Stack->contents[Stack->index];
		Stack->contents[Stack->index] = 0;
		Stack->index--;
		return popVal;
	}
	else
		return -1;
		
	//fail
	return -1;
}

int StackIsFull(Stack_t *Stack)
{
	if (Stack->index == Stack->maxSize - 1)
		return 1;
	else
		return 0;
		
	//fail
	return -1;
}

int StackIsEmpty(Stack_t *Stack)
{
	if (Stack->index == -1)
		return 1;
	else
		return 0;
		
	//fail
	return -1;
}
