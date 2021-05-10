
// C program for array implementation of queue from https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <error.h>
#include <limits.h>
#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/HTMLparser.h"

// A structure to represent a queue
typedef struct Queue {
    int front, rear, size;
    int capacity;
    xmlNode** array;
} q_t;
 
// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(int capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (xmlNode**)malloc(
        queue->capacity * sizeof(xmlNode*));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, xmlNode* item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    //printf("%d enqueued to queue\n", item);
}
 
// Function to remove an item from queue.
// It changes front and size
xmlNode* dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return NULL;
    xmlNode* item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// Function to get front of queue
xmlNode* front(struct Queue* queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->array[queue->front];
}
 
// Function to get rear of queue
xmlNode* rear(struct Queue* queue)
{
    if (isEmpty(queue))
        return NULL;
    return queue->array[queue->rear];
}