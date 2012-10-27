#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include "hash.h"


struct args {
    hashtable_t *hash;
    int max_values;
    int num_words;    
    char **words;
    int done_adding;
    pthread_mutex_t done_adding_mutex;
    pthread_cond_t done_adding_condv;
};

void *hasher_thread(void *v) {
    struct args *args = (struct args *)v;
    
    fprintf(stderr,"Thread %p running\n", pthread_self());
    int start_value = random() % args->num_words;
    int values_to_add = args->max_values;

    int i = 0;
    for (i = 0; i < values_to_add; i++) {
        hashtable_add(args->hash, args->words[(start_value + i) % args->num_words]);
    }
    
    pthread_mutex_lock(&args->done_adding_mutex);
    // add one to done_adding as a signal to main thread that we're done
    // adding to hashtable.
    args->done_adding += 1;
    // wait for main thread to signal to workers to start removing from table
    while (args->done_adding != -1) {
        pthread_cond_wait(&args->done_adding_condv, &args->done_adding_mutex);
    }
    pthread_mutex_unlock(&args->done_adding_mutex);
    
    for (i = 0; i < values_to_add; i++) {
        hashtable_remove(args->hash, args->words[(start_value + i) % args->num_words]);
    }
    return NULL;
}

void load_words(const char *filename, struct args *threadargs) {
    FILE *infile = fopen(filename, "r");
    if (NULL == infile) {
        fprintf(stderr, "Can't locate %s to open it?\n", filename);
        exit(0);
    }
    
    int num_words = 0;
    int arraysize = 1024;
    char **wordarray = (char **)malloc(sizeof(char *) * arraysize);
    
    char buffer[1024];
    const char *whitespace = " \t\n";
    while (NULL != fgets(buffer, 1024, infile)) {
        char *tmp = strtok(buffer, whitespace);
        while (tmp != NULL) {
            wordarray[num_words] = strdup(tmp);
            num_words += 1;
            if (num_words == arraysize) {
                arraysize *= 2;
                wordarray = (char **)realloc(wordarray, sizeof(char *) * arraysize);
            }
            tmp = strtok(NULL, whitespace);
        }
    }
    
    threadargs->num_words = num_words;
    threadargs->words = wordarray;
    
    fclose(infile);
}

void usage(const char *progname) {
    fprintf(stderr, "usage: %s [-h] [-t threads] [-m max_values_to_add] [-s num_hash_buckets]\n", progname);
    fprintf(stderr, "\t-m: num of values for each thread to add to hashtable\n");
    fprintf(stderr, "\t-s: number of hashtable buckets\n");
    fprintf(stderr, "\t-t: number of threads to start up\n");        
    fprintf(stderr, "\t-h: show this help\n");
    exit(0);
}

int main(int argc, char **argv) {
    int num_threads = 1;
    int max_values_to_add = 10;
    int num_hash_buckets = 13;
    
    int c;
    while ((c = getopt(argc, argv, "t:m:s:h")) != -1) {
        switch(c) {
            case 't':
                num_threads = atoi(optarg);
                if (num_threads < 1 || num_threads > 100) {
                    usage(argv[0]);
                }
                break;
            case 'm':
                max_values_to_add = atoi(optarg);
                if (max_values_to_add < 1) {
                    usage(argv[0]);
                }
                break;
            case 's':
                num_hash_buckets = atoi(optarg);
                if (num_hash_buckets < 1) {
                    usage(argv[0]);
                }
                break;
            case 'h':
            default:
                usage(argv[0]);
                break;
        }
    }

    // see the RNG
    srandom(time(NULL));

    // set up thread arguments
    struct args thread_args;
    thread_args.hash = hashtable_new(num_hash_buckets);
    thread_args.max_values = max_values_to_add;
    thread_args.num_words = 0;
    thread_args.words = NULL;
    thread_args.done_adding = 0;
    pthread_mutex_init(&thread_args.done_adding_mutex, NULL);
    pthread_cond_init(&thread_args.done_adding_condv, NULL);

    // load up words from text file
    load_words("words.txt", &thread_args);

    // here are our threads...
    pthread_t threads[num_threads];
    int i = 0;

    // start up the threads; they'll start adding to the hashtable
    // immediately.
    for (i = 0; i < num_threads; i++) {
        if (0 > pthread_create(&threads[i], NULL, hasher_thread, (void*)&thread_args)) {
            fprintf(stderr, "Error creating thread: %s\n", strerror(errno));
        }
    }

    // do nothing in the main thread while worker
    // threads are adding to hashtable.  
    while (thread_args.done_adding < num_threads) {
        // sleep for half a second
        usleep(500000);
    }

    // threads are done adding - dump the hashtable
    printf("Dump of the hash table (which should be as full as it's gonna get).\n");
    hashtable_print(thread_args.hash);

    // signal worker threads to start removing from hashtable
    pthread_mutex_lock(&thread_args.done_adding_mutex);
    thread_args.done_adding = -1;
    pthread_mutex_unlock(&thread_args.done_adding_mutex);
    pthread_cond_broadcast(&thread_args.done_adding_condv);
    
    // wait for workers to complete
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Dump of the hash table (which should be empty!)\n");
    hashtable_print(thread_args.hash);

    for (i = 0; i < thread_args.num_words; i++) {
        free(thread_args.words[i]);
    }
    free(thread_args.words);
    hashtable_free(thread_args.hash);
    exit(0);
}
