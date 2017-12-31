#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if(hash_function==NULL || destroy_function==NULL || capacity <= 0){
        errno = EINVAL;
        return NULL;
    }
    hashmap_t* myHashmap_t = (hashmap_t*)calloc(4, sizeof(hashmap_t));
    if(myHashmap_t==NULL){
        return NULL;
    }
    myHashmap_t->capacity = capacity;
    myHashmap_t->size = 0;
    myHashmap_t->num_readers = 0;
    myHashmap_t->hash_function = hash_function;
    myHashmap_t->destroy_function = destroy_function;
    myHashmap_t->invalid = false;
    myHashmap_t->nodes = (map_node_t*)calloc(capacity, sizeof(map_node_t));

    pthread_mutex_init(&myHashmap_t->write_lock, NULL);
    pthread_mutex_init(&myHashmap_t->fields_lock, NULL);
    return myHashmap_t;
}

// this modifies the underlying map, so lock the writers lock
bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    if(self==NULL || self->invalid || (&key.key_base == NULL) || (key.key_len == 0) ||
      (val.val_base == NULL) || ((val.val_len == 0))){
        errno = EINVAL;
        return false;
    }
    if((self->size == self->capacity) && (!force)){
        errno = ENOMEM;
        return false;
    }

    pthread_mutex_lock(&self->write_lock);

    int index = get_index(self, key);
    int i = index;
    /* if node[index] is empty, put in your values*/

    if((self->nodes[i].key.key_base == NULL) || self->nodes[i].tombstone){
        self->nodes[i] = MAP_NODE(MAP_KEY(key.key_base, key.key_len), MAP_VAL(val.val_base, val.val_len), false);
        self->size++;
        pthread_mutex_unlock(&self->write_lock);
        return true;
    }
    /* if force is true, and map is full, evict current node at computed index and replace the key/value*/
    else if((self->size == self->capacity) && force){
        if(!((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0))){
            self->destroy_function(self->nodes[i].key, self->nodes[i].val); // evict node
            self->nodes[i] = MAP_NODE(MAP_KEY(key.key_base, key.key_len), MAP_VAL(val.val_base, val.val_len), false);
        }
        else{
            self->nodes[i].val = MAP_VAL(val.val_base, val.val_len);
        }
        pthread_mutex_unlock(&self->write_lock);
        return true;
    }

    // /* the annoying part, loop through until you find an empty cell, tombstone or same keybase, and build on top of it or update val*/
    else{
        while(i < self->capacity){
            /* if current cell of computed index is dead, then do this */
            /* if current cell of computed index is empty, then do this */
            if((self->nodes[i].key.key_base == NULL) || self->nodes[i].tombstone){
                self->nodes[i] = MAP_NODE(MAP_KEY(key.key_base, key.key_len), MAP_VAL(val.val_base, val.val_len), false);
                self->size++;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
                /* if keybase @ computed index of map == keybase, update the node value with given value */

            else if((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0)){
                self->nodes[i].val.val_base = val.val_base;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
            i++;
        }
        i = 0;
        while(i < index){
            /* empty */
            if((self->nodes[i].key.key_base == NULL) || self->nodes[i].tombstone){
                self->nodes[i] = MAP_NODE(MAP_KEY(key.key_base, key.key_len), MAP_VAL(val.val_base, val.val_len), false);
                self->size++;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
            /* if keybase @ computed index of map == keybase, update the node value with given value */
            else if((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0)){
                self->nodes[i].val.val_base = val.val_base;
                pthread_mutex_unlock(&self->write_lock);
                return true;
            }
            i++;
        }
    }

    return false;
}

/* this doesn't modify the underlying map so use the reader lock */
map_val_t get(hashmap_t *self, map_key_t key) {
    if(self==NULL || self->invalid || (&key.key_base == NULL) || (key.key_len == 0)){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    int index = get_index(self, key);
    if(index >= self->capacity){
        return MAP_VAL(NULL, 0);
    }

    int i = index;
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers++;
    if(self->num_readers == 1){
        pthread_mutex_lock(&self->write_lock);
    }

    pthread_mutex_unlock(&self->fields_lock);
    while(i < self->capacity){
        if((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0) && (!self->nodes[i].tombstone)){
            pthread_mutex_lock(&self->fields_lock);
            self->num_readers--;
            if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
            }
            pthread_mutex_unlock(&self->fields_lock);
            return MAP_VAL(self->nodes[i].val.val_base, self->nodes[i].val.val_len);
        }
        else if((&self->nodes[i].key.key_base == NULL) && (!&self->nodes[i].tombstone)){ // empty and tombstone doesnt exist
            pthread_mutex_lock(&self->fields_lock);
            self->num_readers--;
            if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
            }
            pthread_mutex_unlock(&self->fields_lock);
            return MAP_VAL(NULL, 0);
        }
        i++;
    }

    i = 0;
    while(i < index){
        if((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0) && (!self->nodes[i].tombstone)){
            pthread_mutex_lock(&self->fields_lock);
            self->num_readers--;
            if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
            }
            pthread_mutex_unlock(&self->fields_lock);
            return MAP_VAL(&self->nodes[index].val.val_base, self->nodes[index].val.val_len);
        }
        else if((&self->nodes[i].key.key_base == NULL) && (!self->nodes[i].tombstone)){ // empty and tombstone doesnt exist
            pthread_mutex_lock(&self->fields_lock);
            self->num_readers--;
            if(self->num_readers == 0){
                pthread_mutex_unlock(&self->write_lock);
            }
            pthread_mutex_unlock(&self->fields_lock);
            return MAP_VAL(NULL, 0);
        }
        i++;
    }
    pthread_mutex_lock(&self->fields_lock);
    self->num_readers--;
    if(self->num_readers == 0){
        pthread_mutex_unlock(&self->write_lock);
    }
    pthread_mutex_unlock(&self->fields_lock);
    return MAP_VAL(NULL, 0);
}


/* writer lock */
map_node_t delete(hashmap_t *self, map_key_t key) {
    if(self==NULL || self->invalid || (key.key_base == NULL) || (key.key_len == 0)){
        errno = EINVAL;
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    pthread_mutex_lock(&self->write_lock);
    int index = get_index(self, key);
    if(index >= self->capacity){
        pthread_mutex_unlock(&self->write_lock);
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }

    int i = index;
    while(i < self->capacity){
        if((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0) && (!self->nodes[i].tombstone)){
            self->nodes[i].tombstone = true;
            self->size = self->size - 1;
            pthread_mutex_unlock(&self->write_lock);
            return MAP_NODE(MAP_KEY(&self->nodes[i].key.key_base, i), MAP_VAL(self->nodes[i].val.val_base, i), false);
        }
        else if((self->nodes[i].key.key_base == NULL) && (!self->nodes[i].tombstone)){ // empty and tombstone doesnt exist
            pthread_mutex_unlock(&self->write_lock);
            return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
        }
        i++;
    }
    i = 0;
    while(i < self->size){
        if((self->nodes[i].key.key_len == key.key_len) && (memcmp(self->nodes[i].key.key_base, key.key_base, key.key_len) == 0) && (!&self->nodes[i].tombstone)){
            self->nodes[i].tombstone = true;
            self->size = self->size - 1;
            pthread_mutex_unlock(&self->write_lock);
            return MAP_NODE(MAP_KEY(self->nodes[i].key.key_base, i), MAP_VAL(self->nodes[i].val.val_base, i), false);
        }
        else if((self->nodes[i].key.key_base == NULL) && (!self->nodes[i].tombstone)){ // empty and tombstone doesnt exist
            pthread_mutex_unlock(&self->write_lock);
            return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
        }
        i++;
    }
    pthread_mutex_unlock(&self->write_lock);
    return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
}

bool clear_map(hashmap_t *self) {
    if(self==NULL || self->invalid){
        errno = EINVAL;
        return false;
    }
    pthread_mutex_lock(&self->write_lock);
    for(int i = 0; i < self->capacity; i++){
        if(!self->nodes[i].tombstone){
            self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            self->nodes[i].tombstone = true;
        }
    }
    self->size = 0;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}

bool invalidate_map(hashmap_t *self) {
    clear_map(self);
    pthread_mutex_lock(&self->write_lock);
    free(self->nodes);
    self->invalid = true;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}