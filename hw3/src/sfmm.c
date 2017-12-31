/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <debug.h>
#include <string.h>
/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;

int arr[3]; // for maintaining free list after splitting a chunk from one of the free blocks
int arr2[2]; // for maintaining free list after coalescing 2 free blocks in memory

void *sf_malloc(size_t size) {
    void *ptr;
    if(size == 0 || size > PAGE_SZ * 4){ // if user requests size that is equal to 0 or greater than 4 pages, set sf_errno to EINVAL + return null
        sf_errno = EINVAL;
        return NULL;
    }

    int padValue;
    if(size%16 == 0){
        padValue = 0;
    }
    else{
        padValue = 16 - size%16;
    }

    size_t size1 = size + 8 + 8 + padValue; // this would be the total block size after adding header/footer size and padding

    if(size1 > PAGE_SZ * 4){
        sf_errno = ENOMEM;
        return NULL;
    }

    int seg_list_index = get_list_index(size1); // size1 could be 32, which fits into the first list, so this value can be 0

    // sf_free_header *x = seg_free_list[0].head;
    int * found = find_fit(seg_list_index, size1); // if found is -1, then does no exist anywhere in any of the list, so call sf_sbrk

    /* if found is -1 that means there isn't a free block any of the free lists, so call sf_sbrk*/
    while(found[0] == -1){
        ptr = sf_sbrk();
        sf_footer* newFooter = (sf_footer*)(ptr-8);

        /* This means we will need to do some backwards coalescing */
        if((newFooter->allocated == 0) && (newFooter->block_size<<4)>0){
            int blocksize = ((sf_footer *)(ptr-8))->block_size<<4; // block size is located in the footer
            sf_header* header = ((sf_header*)(ptr-blocksize));
            header->block_size = (blocksize + PAGE_SZ)>>4; // just changed the block_size for header
            sf_footer* footer = ((sf_footer*)(ptr+PAGE_SZ-8));
            footer->block_size = (blocksize + PAGE_SZ)>>4; // just changed block_size for footer
            ptr = ptr-blocksize; // point to the header;

            int *found2 = location_in_sfl(ptr);
            relink(found2[0], found2[1]);
            sf_free_header* fheader = ptr;
            fheader->next = NULL;
            fheader->prev = NULL;
            fheader->header.allocated = 0;
            fheader->header.padded = 0;

            fheader->header = *header;
            int index = get_list_index(footer->block_size << 4);

            append(index, fheader);

            found = find_fit(seg_list_index, size1);
        }
        /* This ends the backwards coalescing */

        /* Backwards coalescing is not necessary here... Because last block in memory was not a free one. */
        else{
            sf_free_header* f_header = ptr;     // this points to the return pointer of sf_sbrk() aka the heap/page
            f_header->header.allocated = 0;
            f_header->header.padded = 0;
            f_header->header.block_size = PAGE_SZ >> 4;
            sf_footer* footer = ptr + PAGE_SZ - 8;
            footer->allocated = 0;
            footer->padded = 0;
            footer->block_size = PAGE_SZ >> 4;
            int index = get_list_index(f_header->header.block_size<<4);

            append(index, f_header);

            found = find_fit(seg_list_index, size1);

        }
    }

        /* To splinter or not to splinter*/
        /* if found[2] == 0, then that means there is no splinter*/
        /* if found[2] == 1, then that means there is a splinter*/
        // if(!found[2]){
        void* ptr2 = seg_free_list[found[0]].head; // it's like int i = 0
        // this takes the appropriate list from the seg_free_list explicit list, and traverses through to the appropriate block in the list
        int traverse = 0;
        while(traverse < found[1]){
            ptr2 = seg_free_list[found[0]].head->next;
            traverse++;
        }
        sf_header* chunkHeader = ptr2;
        int originalBlockSize = chunkHeader->block_size<<4;
        /* Here we create the new block that the user requested */
        sf_header* myHeader = ptr2;  // now this new block we're trying to make points to the appropriate pointer
        myHeader->allocated = 1;
        if(found[2]){
            myHeader->block_size = (originalBlockSize >> 4); // We're going to use the whole free block for allocation if there is a splinter.
            size1 = myHeader->block_size<<4;
            if(size1 > PAGE_SZ * 4){
                sf_errno = ENOMEM;
                return NULL;
            }
        }
        else{
            myHeader->block_size = (size1 >> 4);
        }
        if(size1 != size+16){
            myHeader->padded = 1;
        }
        sf_footer* footer = ptr2 + size1 - 8;
        footer->allocated = 1;
        footer->padded = myHeader->padded;
        footer->requested_size = size;
        footer->block_size = myHeader->block_size;
        /* End of the new block creation that user requested + set to appropriate pointer and returned later */

        /* Relink the seg_list*/
        relink(found[0], found[1]);
        /* */

        /* Rebuild the free-block that we just carved an allocated chunk out of and set it to appropriate list
        *  If there were no splinters found, rebuild free block. If splinter was found, don't do anything and just return header+1
        */
        if(!found[2]){   // since we took the entire free block, we don't need to recreate a free block and append it to the list
            sf_free_header* fheader = ptr2 + size1;
            fheader->next = NULL;
            fheader->prev = NULL;
            fheader->header.allocated = 0;
            fheader->header.padded = 0;

            fheader->header.block_size = ((originalBlockSize) - size1) >> 4;
            sf_footer* footer2 = ptr2 + (originalBlockSize) - 8;
            footer2->allocated = 0;
            footer2->block_size = (originalBlockSize - size1)>>4;
            int index = get_list_index(footer2->block_size << 4);
            append(index, fheader);
        }

        // printf( " FIRST PRINT OF POINTER: %p ", myHeader + 1);
        return myHeader + 1;
}

void append(int index, sf_free_header* fheader){
    if(seg_free_list[index].head != NULL){
        seg_free_list[index].head->prev = fheader;
        fheader->next = seg_free_list[index].head;
        seg_free_list[index].head = fheader;
    }
    else{
        seg_free_list[index].head = fheader;
    }
}

void relink(int index, int traverse){
    void* ptr = seg_free_list[index].head;

    // this takes the appropriate list from the seg_free_list explicit list, and traverses through to the appropriate block in the list
    int traversing = 0;
    while(traversing < traverse){
        ptr = seg_free_list[index].head->next;
        traversing++;
    }

    /* if ptr.next isn't null */
    sf_free_header* newPtr = ptr;
    if(traversing == 0){ // if traversing == 0, this means newPtr.head is the head of the explicit list
        if(newPtr->next == NULL){ // this means the explicit list literally only has one element, and that's the head
            seg_free_list[index].head = NULL; // set the head of that list to NULL, now it's an empty list
        }
        else{ // else head element has a node after it
            newPtr->next->prev = NULL; // make the second to last head no longer point to the current head
            seg_free_list[index].head = newPtr->next; // make second to last head the new head of the list
            newPtr->next = NULL;
        }

    }
    else{ // newPtr has to have a prev, since it traversed its list more than 0 times, but not necessarily a next
        if(newPtr->next != NULL){ // that means newPtr does have a next
            newPtr->prev->next = newPtr->next;  // relink newPtr prev pointer to point to newPtr's next
            newPtr->next->prev = newPtr->prev;  // relink newPtr next pointer to point back to newPtr's prev
            newPtr->prev = NULL;
            newPtr->next = NULL;
        }
        else{   // newPtr has a prev, but not a next
            newPtr->prev->next = NULL;
            newPtr->prev = NULL;
        }
    }
}

int get_list_index(int size1){
    int i = 0;
    int found = 0;
    while(!found && i < FREE_LIST_COUNT){
        if((size1 >= seg_free_list[i].min && size1 <= seg_free_list[i].max)){ // if block size is 32 then it fits in seg_free_list[0]
            found = 1;
        }
        else{
            i++;
        }
    }
    return i;
}
/*
*   LOGIC: loop through current list in seg_free_list[index] to find a block that fits the block_size
*   IMPORTANT: seg_free_list[index].head->header.block_size << 4
*/
int * find_fit(int index, int size){
    sf_free_header *x = seg_free_list[index].head;
    int myIndex = index;
    int innerIndex = 0;
    while(myIndex < 4){
        while(x != NULL){
            int curr_block_size = x->header.block_size << 4;
            if(((curr_block_size) >= size) && (curr_block_size) - size > 31){
                arr[0] = myIndex;
                arr[1] = innerIndex;
                arr[2] = 0;
                return arr;
            }
            else if (((curr_block_size) >= size) && (curr_block_size)-size<32){
                arr[0] = myIndex;
                arr[1] = innerIndex;
                arr[2] = 1;
                return arr;
            }
            else{
                innerIndex++;
                x = x->next;
            }
        }
        innerIndex = 0;
        myIndex++;
        x = seg_free_list[myIndex].head;
    }
    arr[0] = -1;
    arr[1] = -1;
    arr[2] = -1;
    return arr;
}

/*
*   PURPOSE: loop through the lists in the segmented_free_lists, find s_f_l[i].head.prev == the coalesced block and remove that from
*   the free list
*/

int * location_in_sfl(void *ptr){
    sf_free_header *x = seg_free_list[0].head;
    int index = 0;
    int innerIndex = 0;
    while(index < 4){
        while(x != NULL){
            if(ptr == x){
                arr2[0] = index;
                arr2[1] = innerIndex;
                return arr2;
            }
            else{
                innerIndex++;
                x = x->next;
            }
        }
        innerIndex = 0;
        index++;
        x = seg_free_list[index].head;
    }
    arr2[0] = -1;
    arr2[1] = -1;
    return arr2;
}

void *sf_realloc(void *ptr, size_t size) {
    /* THIS BEGINS ALL THE BORING VALIDATION CHECKS */

    /* IF SIZE IS 0, FREE THE POINTER AND RETURN NULL */
    if(size == 0){
        sf_free(ptr);
        return NULL;
    }

    if(size > PAGE_SZ * 4){ // if user requests size that is equal to 0 or greater than 4 pages, set sf_errno to EINVAL + return null
        sf_errno = EINVAL;
        return NULL;
    }
       /*  if pointer is null, then abort... */

    if(ptr == NULL){
        abort();
    }

    /* Since ptr is not null, let's cast it to sf_header* and see what it's inside it */
    // sf_free_header* sf = ptr-8;
    sf_header* header = ptr-8;
    sf_footer* footer = ((ptr-8) + (header->block_size<<4)) - 8;

    /* if ptr starts before the heap start or if ptr ends after heap_end, return null */
    if(ptr-8 < get_heap_start() || (((header->block_size<<4)+(ptr-8)) > get_heap_end())){
        abort();
    }

    /* if header/footer allocated bit is 0... already freed, abort */
    if((header->allocated == 0) || (footer->allocated == 0)){
        abort();
    }

    /* if requested_size + 16 != block_size and padded = 0, this is invalid so abort */
    if(((footer->requested_size+16) != header->block_size<<4) && footer->padded == 0){
        abort();
    }

    /* if requested_size + 16 == block_size and padded = 1, this is invalid so abort */
    if(((footer->requested_size+16) == header->block_size<<4) && footer->padded == 1){
        abort();
    }

    /* if padded bits are inconsistent, abort() */
    if(footer->padded != header->padded){
        abort();
    }

    /* if allocated bits are inconsistent, abort() */
    if(footer->allocated != header->allocated){
        abort();
    }

    /* THIS ENDS ALL OF THE VALIDATION CHECKS */


    /* Check if the new size is smaller or larger than the block we're trying to realloc */
    int padding = 0;
    if(size%16 != 0){
        padding = 16 - size%16;
    }
    size_t size1 = size + 16 + padding;

    /* This begins the realloc stuff when reallocing to a larger size */
    if(size1 > (header->block_size<<4)){
          // debug(" %s ", "LINE 3");
        void *ptr2 = sf_malloc(size);
        sf_header* header = ptr2-8;
        if(size1 != size+16){
            // debug(" %s ", "LINE 1");
            header->padded = 1;
        }
        else{
            // debug(" %s ", "LINE 2");
            header->padded = 0;
        }
        memcpy(ptr2, ptr, size);
        sf_free(ptr);
        return ptr2;
    }
    /* This ends the realloc stuff when reallocing to a larger size */

    /* This begins the realloc stuff when reallocing to a smaller size */
    else if(size1 < (header->block_size<<4)){

        /* Check for splinter */
        if(((header->block_size<<4) - size1) < 32){ // splinter found
            footer->requested_size = size;
            return ptr;
        }
        else{   // no splinter found
            /* Here we create the new header for the reallocated block */
            int original_size = header->block_size<<4;
            header-> block_size = (original_size - (original_size - size1))>>4;
            if(size1 != size+16){
                header->padded = 1;
            }
            /* New header ends here*/

            /* Here we create the new footer for the reallocated block */
            sf_footer* footer = (sf_footer*)((header) + ((header->block_size<<4)/8) - 1);
            footer->block_size = header->block_size;
            footer->allocated = 1;
            footer->padded = header->padded;
            footer->requested_size = size;
            /* New footer implementation ends here */

            /* Check if next block in memory is free */
            int size = ((sf_header*)(ptr-8+original_size))->block_size<<4;          // size of next block in memory
            int allocated = ((sf_header*)(ptr-8 + (original_size)))->allocated;     // if next block in memory is allocated or not
            void *myPtr = (((sf_free_header*)(ptr-8+original_size)))  ;             // pointer to block immediately after current ptr;
            myPtr = myPtr + 8;
            sf_free_header* mySf = myPtr-8;                                           // initialize free block for next block in memory

            coalesce(size, allocated, myPtr, mySf, header, footer, (int)header->block_size<<4, original_size);
            return ptr;
        /* Chickens here */
        }
    }
    /* This ends the realloc stuff when reallocing to a smaller size */

    /* This is when the two blocks are of equal size, in terms of payload */
    else{
        if(footer->requested_size == size){
            return ptr;
        }
        if(footer->requested_size != size){
            footer->requested_size = size;
            if(size1 == size+16){
                footer->padded = 0;
                ((sf_header*)(ptr-8))->padded = 0;
            }
            else{
                footer->padded = 1;
                ((sf_header*)(ptr-8))->padded = 1;
            }
            return ptr;
        }
   }
    /* end of reallocing */

    return NULL;
}

void sf_free(void *ptr) {
    /*  if pointer is null, then abort... */
    if(ptr == NULL){
        abort();
    }

    /* Since ptr is not null, let's cast it to sf_header* and see what it's inside it */
    sf_free_header* sf = ptr-8;
    sf_header* header = ptr-8;
    sf_footer* footer = ((ptr-8) + (header->block_size<<4)) - 8;


    /* if ptr starts before the heap start or if ptr ends after heap_end, return null */
    if(ptr-8 < get_heap_start() || (((header->block_size<<4)+(ptr-8)) > get_heap_end())){
        abort();
    }

    /* if header/footer allocated bit is 0... already freed, abort */
    if((header->allocated == 0) || (footer->allocated == 0)){
        abort();
    }

    /* if requested_size + 16 != block_size and padded = 0, this is invalid so abort */
    if(((footer->requested_size+16) != header->block_size<<4) && footer->padded == 0){
        abort();
    }

    /* if requested_size + 16 == block_size and padded = 1, this is invalid so abort */
    if(((footer->requested_size+16) == header->block_size<<4) && footer->padded == 1){
        abort();
    }

    /* if padded bits are inconsistent, abort() */
    if(footer->padded != header->padded){
        abort();
    }

    /* if allocated bits are inconsistent, abort() */
    if(footer->allocated != header->allocated){
        abort();
    }

    int size = ((sf_header*)((ptr-8) + (header->block_size<<4)))->block_size<<4;
    int allocated = ((sf_header*)((ptr-8) + (header->block_size<<4)))->allocated;

    coalesce(size, allocated, ptr, sf, header, footer, 0, 0);

    return;
}

/* This checks if coalescing is possible... checks if next block in memory size exists, and checks if it's allocated or not
* If next block in memory has a size, and it's not allocated, then we will need to do some coalescing...
* If next block in memory has a size, and it's allocated, then no coalescing is needed, just free requested block pointer
* If next block in memory doesn't have a size, allocation is irrelevant, and we just free the requested block pointer
*/
void coalesce(int size, int allocated, void *ptr, sf_free_header* sf, sf_header* header, sf_footer* footer, int i, int originalSize){
    if(size && !allocated){
    // header2 is the header of freeBlock2, aka header of the free block that's immediately next to current block in memory
        sf_header* header2 = (ptr-8) + ((header->block_size<<4) - i);

        // footer2 is the footer of the freeBlock2
        sf_footer* footer2 = (((ptr-8) + (header->block_size<<4) - i) + (header2->block_size<<4)) - 8;

        // set header and footer2 block_size to the blocksize of current block's block_size to the free block block_size
        if(i > 0){
            (header+(i/8))->block_size = ((originalSize - ((header->block_size<<4))) + (header2->block_size<<4)) >> 4;
        }
        else{
            header->block_size = (((header->block_size<<4)) + (header2->block_size<<4))>> 4;
        }
        footer2->block_size = (header+(i/8))->block_size;
        // --------------------------------------------------------------------------------------------------------------

        int * found = location_in_sfl(header2);
        relink(found[0], found[1]);

        sf = (sf_free_header*)(header+(i/8));
        header = header+(i/8);
        sf->next = NULL;
        sf->prev = NULL;
        sf->header.block_size=footer2->block_size;
        sf->header.allocated = 0;
        sf->header.padded = 0;

        int index = get_list_index(footer2->block_size << 4);

        append(index, sf);
    }
    /*
    *   Free only, no coalescing cases:
    *   Case 1: Next block in memory exists but is allocated
    *   Case 2: Next block in memory does not exist
    */
    else if((size && allocated) || (!size)){
        sf->header.padded = 0;
        sf->header.allocated = 0;
        footer->padded = 0;
        footer->allocated = 0;
        footer->requested_size = 0;
        int index = get_list_index(footer->block_size << 4);
        append(index, sf);
    }
}
