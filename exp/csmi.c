/*
 *csim.c-使用C编写一个Cache模拟器，它可以处理来自Valgrind的跟踪和输出统计
 *息，如命中、未命中和逐出的次数。更换政策是LRU。
 * 设计和假设:
 *  1. 每个加载/存储最多可导致一个缓存未命中。（最大请求是8个字节。）
 *  2. 忽略指令负载（I），因为我们有兴趣评估trace.c内容中数据存储性能。
 *  3. 数据修改（M）被视为加载，然后存储到同一地址。因此，M操作可能导致两次缓存命中，或者一次未命中和一次命中，外加一次可能的逐出。
 * 使用函数printSummary() 打印输出，输出hits, misses and evictions 的数，这对结果评估很重要
*/

#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include "cachelab.h"

#define ADDRESS_LENGTH 64

typedef unsigned long long int mem_addr_t;
typedef struct cache_line{
    char valid;
    mem_addr_t tag;
    unsigned long long int lru;
} cache_line_t;

typedef cache_line_t *cache_set_t;
typedef cache_set_t *cache_t;
cache_t cache;

int verbosity = 0; 
int s = 0;
int b = 0;
int E = 0;
char *trace_file = NULL;

int S;
int B;

int miss_count = 0;
int hit_count = 0;
int eviction_count = 0;

void initCache(){
    cache = (cache_set_t *)malloc(sizeof(cache_set_t) * S);
    for (int i = 0; i < S; i++){
        cache[i] = (cache_line_t *)malloc(sizeof(cache_line_t) * E);
        for (int j = 0; j < E; j++){
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].lru = 0;
        }
    }
}

void freeCache(){
    for(int i = 0; i < S; i++)
        free(cache[i]);
    free(cache);
}

void accessData(mem_addr_t addr){
    unsigned long long int eviction_lru = ULONG_MAX;
    unsigned int eviction_line = 0;
    mem_addr_t tag = addr >> (s + b);

    cache_set_t cache_set = cache[(addr >> b) & (s-1)];
    int hit_index = -1;     //命中行号
    int invalid_index = -1; //空闲行号
    for (int i = 0; i < E; i++){
        if (cache_set[i].valid){
            cache_set[i].lru--; //LRU计数-1
            if (cache_set[i].lru < eviction_lru){
                eviction_lru = cache_set[i].lru;
                eviction_line = i; //更新LRU计数最小值
            }
            if (cache_set[i].tag == tag)    /* 命中 */
                hit_index = i;
        }
        else if (invalid_index == -1)
            invalid_index = i; //记录空行
    }
    
    if (hit_index != -1){   /* 命中 */
        if (verbosity){
            printf("Hit! line:%d tag:%lld\n", hit_index, cache_set[hit_index].tag);
        }
        hit_count++;
        cache_set[hit_index].lru = ULONG_MAX; //LRU计数重置为最大值
        hit_index = -1;                       //重置命中行号为-1
    }
    else{   /* 不命中 */
        miss_count++;
        if (invalid_index != -1){   /* 有空位 */
            if (verbosity){
                printf("Miss_Insert! line:%d tag:%lld\n", invalid_index, cache_set[invalid_index].tag);
            }
            cache_set[invalid_index].tag = tag;
            cache_set[invalid_index].lru = ULONG_MAX;
            cache_set[invalid_index].valid = 1;
            invalid_index = -1; //重置空闲行号
        }
        else{   /* 没有空位 */
            if (verbosity){
                printf("Miss_Evict! line:%d tag:%lld\n", eviction_line, cache_set[eviction_line].tag);
            }
            eviction_count++;
            cache_set[eviction_line].tag = tag;
            cache_set[eviction_line].lru = ULONG_MAX;
            eviction_lru = ULONG_MAX; //重置最小LRU计数
        }
    }
}


void replayTrace(char *trace_fn)
{
    char buf[1000];
    mem_addr_t addr = 0;
    unsigned int len = 0;
    FILE *trace_fp = fopen(trace_fn, "r");

    while(fscanf(trace_fp, " %s %llx,%u", buf, &addr, &len) != EOF){
        if(verbosity)
            printf("%s %llx,%u\n", buf, addr, len);
        switch (buf[0]){
            case 'L': /* Load */
                accessData(addr);
                break;
            case 'S': /* Store */
                accessData(addr);
                break;
            case 'M': /* Modify */
                accessData(addr);
                accessData(addr);
                break;
        }
    }
    fclose(trace_fp);
}

void printUsage(char *argv[])
{
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

int main(int argc, char *argv[]){
    char c;
    while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1){
        switch (c){
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                trace_file = optarg;
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

    if (s == 0 || E == 0 || b == 0 || trace_file == NULL){
        printf("%s: Missing required command line argument\n", argv[0]);
        printUsage(argv);
        exit(1);
    }

    S = (unsigned int)pow(2, s);
    B = (unsigned int)pow(2, b);

    initCache();
    replayTrace(trace_file);
    freeCache();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}