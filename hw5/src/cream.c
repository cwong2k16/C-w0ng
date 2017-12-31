#include "cream.h"
#include "utils.h"
#include <string.h>
#include "helper.h"
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include "queue.h"
#include "csapp.h"
#include "utils.h"
#include "hashmap.h"

hashmap_t* hashmap;

int main(int argc, char *argv[]) {
    Signal(SIGPIPE, SIG_IGN);
    int help = check_help(argc, argv);
    if(help){
        printf("This is a help menu");
        exit(EXIT_SUCCESS);
    }
    run(argc, argv);
    exit(0);
}

/* argc number of command line args, argv array of command line args, args parsed command line args */
void run(int argc, char *argv[]){
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    queue_t* request_queue = create_queue();


    char *port = argv[2]; // port number, but as a string
    valid_args(argc, argv); // if invalid, this will exit and not move on

    pthread_t tid[atoi(argv[1])];
    hashmap = create_map(atoi(argv[3]), jenkins_one_at_a_time_hash, destroy_function);

    for(int index = 0; index < atoi(argv[1]); index++) {
        if(pthread_create(&tid[index], NULL, thread, request_queue) != 0){
            exit(EXIT_FAILURE);
        }
    }

    listenfd = Open_listenfd(port);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        enqueue(request_queue, &connfd); // add fd (the accepted connection) to queue
    }
}

void destroy_function(map_key_t key, map_val_t val){
    free(key.key_base);
    free(val.val_base);
}

/* this is where you dequeue the connfd and use the worker thread to pull out its data (request_header and all) */
void *thread(void* request_queue){
    Pthread_detach(pthread_self());
    while(1){
        request_header_t request_header;
        uint8_t request_code;
        int *connfdPtr = dequeue(request_queue);
        int connfd = *connfdPtr;
        Rio_readn(connfd, &request_header, sizeof(request_header));

        request_code = request_header.request_code; // check if this is one of the commands above

        if(request_code == PUT){   // read twice, first for the key, and second for the value
            uint32_t key_size = request_header.key_size;
            uint32_t value_size = request_header.value_size;
            response_header_t res = {OK, 0};

            if(key_size < MIN_KEY_SIZE || key_size > MAX_KEY_SIZE){
                res.response_code = BAD_REQUEST;
                Rio_writen(connfd, &res, sizeof(res));

            }
            if(value_size < MIN_VALUE_SIZE || value_size > MAX_VALUE_SIZE){
                res.response_code = BAD_REQUEST;
                Rio_writen(connfd, &res, sizeof(res));
            }
            else{
            void *key = Calloc(1, key_size + 1);        // buffer for key
            void *value = Calloc(1, value_size + 1);    // buffer for val
            Rio_readn(connfd, key, key_size);
            Rio_readn(connfd, value, value_size);

            map_key_t myKey = {key, key_size};
            map_val_t myVal = {value, value_size};

            if(put(hashmap, myKey, myVal, true)){
                res.response_code = OK;
                res.value_size = value_size;
            }
            else{
                res.response_code = BAD_REQUEST;
                res.value_size = 0;
            }



            Rio_writen(connfd, &res, sizeof(res));
            }
        }
        else if(request_code == GET){
            uint32_t key_size = request_header.key_size;
            response_header_t res = {NOT_FOUND, 0};
            if(key_size < MIN_KEY_SIZE || key_size > MAX_KEY_SIZE){
                res.response_code = BAD_REQUEST;
                Rio_writen(connfd, &res, sizeof(res));
            }
            else{
                void *key = Calloc(1, request_header.key_size+1);
                Rio_readn(connfd, key, request_header.key_size);
                map_key_t myKey = {key, request_header.key_size};

                map_val_t myVal = get(hashmap, myKey);
                if(myVal.val_base!= NULL){
                    res.response_code = OK;
                    res.value_size = sizeof(res);
                }
                Rio_writen(connfd, &res, sizeof(res));
                Rio_writen(connfd, myVal.val_base, myVal.val_len);
            }
        }
        else if(request_code == EVICT){
            uint32_t key_size = request_header.key_size;
            response_header_t res = {OK, 0};
            if(key_size < MIN_KEY_SIZE || key_size > MAX_KEY_SIZE){
                res.response_code = OK;
                Rio_writen(connfd, &res, sizeof(res));
            }
            else{
                void *key = Calloc(1, request_header.key_size+1);
                Rio_readn(connfd, key, request_header.key_size);
                map_key_t myKey = {key, key_size};
                delete(hashmap, myKey);
                res.response_code = OK;
                Rio_writen(connfd, &res, sizeof(res));
            }


        }
        else if(request_code == CLEAR){
            response_header_t res = {OK, 0};
            if(clear_map(hashmap)){
                Rio_writen(connfd, &res, sizeof(res));
            }
            else{
                res.response_code = BAD_REQUEST;
                Rio_writen(connfd, &res, sizeof(res));
            }
        }
        else {
            response_header_t res = {UNSUPPORTED, 0};
            Rio_writen(connfd, &res, sizeof(res));
        }
        Close(connfd);
    }

    return NULL;
}

int check_help(int argc, char *args[]){
    for(int i = 0; i < argc; i++){
        if(strcmp(args[i], "-h") == 0){
            return 1;
        }
    }
    return 0;
}

void valid_args(int argc, char *argv[]){
    if(argc != 4){
        printf("Invalid number of args");
        exit(EXIT_FAILURE);
    }
    else{
        if(argv[1] <= 0){   // NUM_WORKERS
            printf("Invalid number of workers");
            exit(EXIT_FAILURE);
        }
        if(argv[3] <= 0){   // MAX_THREADS
            printf("Invalid number of threads");
            exit(EXIT_FAILURE);
        }
    }
}