#ifndef HELPER_H
#define HELPER_H

int get_list_index(int size1);
int * find_fit(int index, int size);
void relink(int index, int traverse);
void append(int index, sf_free_header*);
int * location_in_sfl(void *ptr);
void coalesce(int size, int allocated, void* ptr, sf_free_header* sf, sf_header* header, sf_footer* footer, int i, int originalSize);

#endif