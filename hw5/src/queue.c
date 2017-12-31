#include "queue.h"
#include <errno.h>
#include <stdio.h>

queue_t *create_queue(void) {
    queue_t* myQueue_t = (queue_t*)calloc(4, sizeof(queue_t));
    int ret;

    if(myQueue_t == NULL){ // calloc failed
        return NULL;
    }

    ret = sem_init(&myQueue_t->items, 0, 0);

    if(ret < 0){        // failed to initialize semaphore
        return NULL;
    }

    ret = pthread_mutex_init(&myQueue_t->lock, NULL);

    if(ret < 0){       // failed to initialize mutex
        return NULL;
    }

    return myQueue_t;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) { // not fully implemented yet
    if(self == NULL || destroy_function == NULL){
        errno = EINVAL;
        return false;
    }
    if(self != NULL && self->invalid){
        errno = EINVAL;
        return false;
    }
     /* call destroy on all the items in the queue */
    pthread_mutex_lock(&self->lock);
    queue_node_t *currNode = self->front;
    while(currNode!=NULL){
        if(currNode->item!=NULL){
           destroy_function(currNode->item);
        }
        currNode = currNode->next;
    }

    currNode = self->front;
    while(currNode!=NULL){
        free(currNode);
        currNode = currNode->next;
    }

    self->invalid = true;
    pthread_mutex_unlock(&self->lock);
    return true;
}

bool enqueue(queue_t *self, void *item) {
    if(self == NULL || item == NULL){
        errno = EINVAL;
        return false;
    }
    else if(self != NULL && self->invalid){
        errno = EINVAL;
        return false;
    }
    queue_node_t* myQueue_node_t = (queue_node_t*)calloc(4, sizeof(queue_node_t));
    if(myQueue_node_t == NULL){
        return false;
    }
    /* Checks for invalid params done; now lock thread, doStuff(), unlock thread, and return */
    pthread_mutex_lock(&self->lock);
    myQueue_node_t->item = item;
    if(self->front == NULL){         // if rear is null, then that means there is not a single element in the queue
        self->rear = myQueue_node_t; // one queue node in the queue means it points to both rear and front
        self->front = myQueue_node_t; // as said above
    }
    else{
        self->rear->next = myQueue_node_t;
        self->rear = myQueue_node_t;
    }
    pthread_mutex_unlock(&self->lock);
    sem_post(&self->items);
    return true; // if self is not null and calloc did not fail, then other initializations should be valid (return true)
}

void *dequeue(queue_t *self) {
    if(self == NULL || (self!= NULL && self->invalid)){
        errno = EINVAL;
        return NULL;
    }
    sem_wait(&self->items);
    pthread_mutex_lock(&self->lock);
    queue_node_t *holdFront = NULL;;
    if(self->front != NULL){
        holdFront = self->front;
        self->front = self->front->next;
        free(self->front);
    }
    pthread_mutex_unlock(&self->lock);
    if(holdFront != NULL){
        return holdFront->item;
    }
    else{
        return NULL;
    }
}