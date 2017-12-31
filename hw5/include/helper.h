#ifndef HELPER_H
#define HELPER_H

void run(int argc, char *argv[]);
int check_help(int argc, char* argv[]);
void valid_args(int argc, char* argv[]);
void *thread(void* request_queue);
void destroy_function();

#endif