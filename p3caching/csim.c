/*
 * CS154 Project 3: Caching
 *
 * If working solo (not with a partner), your repository
 * itself identifies who you are.
 * 
 * If you are working as a pair, please identify yourselves
 * with your "CNetID: Name", by editing the following two lines,
 * filling your CNetIDs and Names: */
PP0 = "fangning: Fangning Gu";
PP1 = "jcma: Jennifer Chunduo Ma";
 /*  We will be grading the work handed in by the FIRST ("PP0") person
 * in the pair above.
 * The SECOND ("PP1") person should hand in a csim.c with the EXACT SAME
 * TWO LINES (with "PP0" and "PP1") above, so that we can be sure the
 * pair of people really intends to be a pair. Other than this, we will
 * not be looking at the files handed in by the SECOND person.

 * csim.c - Cache simulator for replaying Valgrind traces, with
 *     output statistics like # hits, misses, and evictions.
 *     Replacement policy is LRU.
 *
 * Implementation and assumptions:
 *  1. Each load/store can cause at most one cache miss.
 *  2. Instruction loads (I) are ignored: we are only interested in
 *  optimizing data cache performance for this project
 *  3. Data modify (M) is equivalent to a load and then a store to the same
 *  address. An M operation can thus cause two cache hits, or a miss and a
 *  hit plus (possibly) an eviction.
 *
 * The function printSummary() (in caching.c) is given to print output.
 * Use this function to print the number of hits, misses and evictions,
 * which is how the driver while evaluate your work.
 */

#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "caching.h"

typedef struct cache cache;

struct cache {
    int valid; // valid bit for the line.
    int tag; // tag bit for the line.
    int timer; // time when the cache was updated.
};

/*****Global Counters*****/
int missCount = 0; 
int hitCount = 0;
int evictionCount = 0;
int counter = 0;
/* Globals set from command line args */
int verbosity = 0; /* print trace if set */
int s_bits = 0; /* "s": set index bits */
int b_bits = 0; /* "b": block offset bits */
int EE = 0; /* "E": associativity */
char* traceFilename = NULL;
int SS; /* "S": number of sets */
int BB; /* "B": block size (bytes) */
/* Global counters to record cache simulation events */

/*****cache_init*****/
/*initializes cache by looping through the number of sets by setting
 valid, tag and timer to 0. */
cache** cache_init(int S, int E) // S = # of set index bits. E = lines/set
{
    cache** new = (cache**) malloc(sizeof (cache) * E * S);
    // for how many number of sets, we create a cache
    int i = 0;
    while (i < S)
    {
        new[i] = (cache*) malloc(sizeof (cache) * E);
        new[i]->valid = 0;
        new[i]->tag = 0;
        new[i]->timer = 0;
        i++;
    }
    return new;
}

/*****c_simulate*****/
/*takes input of address, s, b, E and a cache and gets the set, tag bits. 
 then loop over every cache line, taking the valid and tag ints from each struct
 if the cache is empty, then empty is set to 0. else keeps updating time to the
 current time and updates hits if hit occurs.
 if miss, updates miss counter by 1 and sets the timer to the first cache struct's
 line. we then find the min time and index to update the min time and index
 accordingly. if a cache line is empty, then set valid to 1 and sets
 current time and tag. */
void c_simulate(int address, int s, int b, int E, cache** c)
{ 
    int t = ((unsigned int) address >>(b+s));
    int s_bit = (address>>b)&(((~0 << (s-1)) << 1)^(~0<<0)) & (~0<<0);
    int empty = 1; // flag to check if cache is full. full by default.
    int i = 0;
    while (i < E)
    {
        counter++;
        int current_tag = (c[s_bit][i]).tag;
        int current_valid = (c[s_bit][i]).valid; 
        if (current_valid == 0)
        {
            empty = 0;
        }
        if (current_valid == 1 && current_tag == t)
        {
            c[s_bit][i].timer = counter; 
            hitCount++;
            return;
        }
        i++;
    }
    // else a miss
    missCount++; 
    if(empty)
    {   
        int timer_m = c[s_bit][0].timer; 
        int index_m = 0;
        evictionCount++;
        i = 0;
        while (i < E)
        {
            if (c[s_bit][i].timer < timer_m)
            {
                timer_m = c[s_bit][i].timer;
                index_m = i;
            }
            i++;
        }
        c[s_bit][index_m].timer = counter; 
        c[s_bit][index_m].tag = t;
    }
    else 
    {
    	i = 0;
        while (i < E)
        {
            if (c[s_bit][i].valid == 0)
            {
                c[s_bit][i].valid = 1;
                c[s_bit][i].timer = counter;
                c[s_bit][i].tag = t;
                return;
            }
            i++;
        }
    }
}

/******simulate******/
/*extracts the information from inputted instructions. if load or store, 
 we call the c_simulate function once. if modify (load and store) we call
 c_simulate twice. */
void simulate (char *trace_fn) {
    FILE *trace_fp = fopen(trace_fn, "r");
    if (!trace_fp) {
        fprintf(stderr, "%s: %s\n", trace_fn, strerror(errno));
        exit(1);
    }
    cache** cache = cache_init(SS, EE);
    //counter *c = count_init();
    int address;
    char operation;
    int size;
    while(1) {
        (fscanf(trace_fp, " %c %x, %d", &operation, &address, &size));
    if (feof(trace_fp))
        break;
        switch (operation)
        {
            case 'L':
                c_simulate(address, s_bits, b_bits, EE, cache);
                break;
            case 'S':
                c_simulate(address, s_bits, b_bits, EE, cache);
                break;
            case 'M':
                c_simulate(address, s_bits, b_bits, EE, cache);
                c_simulate(address, s_bits, b_bits, EE, cache);
                break;
            default: 
                break;
        }
    }
    fclose(trace_fp);
}
/*
 * printUsage - Print usage info
 */
void printUsage(char* argv[]) {
    printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this help message.\n");
    printf("  -v         Optional verbose flag.\n");
    printf("  -s <num>   Number of set index bits.\n");
    printf("  -E <num>   Number of lines per set.\n");
    printf("  -b <num>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExamples:\n");
    printf("  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n", argv[0]);
    printf("  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0]);
    exit(0);
}

/*
 * main - Main routine
 */
int main(int argc, char* argv[]) {
    char c;
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
        switch (c) {
            case 's':
                s_bits = atoi(optarg);
                break;
            case 'E':
                EE = atoi(optarg);
                break;
            case 'b':
                b_bits = atoi(optarg);
                break;
            case 't':
                traceFilename = optarg; 
                break;
            case 'v':
                verbosity = 1;
                break;
            case 'h':
                printUsage(argv);
                exit(0);
            default:
                printUsage(argv);
                exit(1);
        }
    }
    /* Make sure that all required command line args were specified */
    if (s_bits == 0 || EE == 0 || b_bits == 0 || traceFilename == NULL) {
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }
    /* Compute S, E and B from command line args */
    SS = 1 << s_bits;
    BB = 1 << b_bits;
    simulate(traceFilename);
    /* Output the hit and miss statistics for the autograder */
    printSummary(hitCount, missCount, evictionCount);
    return 0;
}