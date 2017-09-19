#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <libmemcached-1.0/memcached.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "cmdline.h"

#define MAX_LATENCY 50

struct benchmarkData {
    unsigned long startTimeInMs;
    char * title;
    unsigned short payload;
    unsigned int * times;
    unsigned short clients;
    unsigned int requests;
};

struct taskData {
    char * key;
    unsigned short requests;
    memcached_st * memc;
    unsigned short payload;
    unsigned int times[MAX_LATENCY];
};

unsigned long generateTimeInMs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long milli = ((unsigned long)ts.tv_sec)*1000;
    milli += ts.tv_nsec * 10e-6;
    return milli;
}

char * generateRandomValue(unsigned short size)
{
    const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char * value = (char *)malloc((size + 1)* sizeof(char));
    if(value != NULL)
    {
        for (unsigned short i = 0; i < size; i++)
        {
            value[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        value[size] = 0;
    }
    else
    {
        value = "";
    }
    return value;
}

struct benchmarkData * prepareBenchmark(char *title, struct gengetopt_args_info *options)
{
    struct benchmarkData *benchmark = (struct benchmarkData *)malloc(sizeof(struct benchmarkData));
    if(benchmark)
    {
        benchmark->startTimeInMs = generateTimeInMs();
        benchmark->title = title;
        benchmark->payload = options->datasize_arg;
        benchmark->clients = options->clients_arg;
        benchmark->requests = benchmark->clients * options->requests_arg;
        unsigned int * values = (unsigned int *) malloc(MAX_LATENCY * sizeof(unsigned int));
        if(values != NULL)
        {
            for(int i = 0; i < MAX_LATENCY; i++)
            {
                values[i] = 0;
            }
            benchmark->times = values;
            return benchmark;
        }
        else {
            free(benchmark);
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

void cleanBenchmark(struct benchmarkData *benchmark)
{
    free(benchmark->times);
    free(benchmark);
}

void finishBenchmark(struct benchmarkData *benchmark)
{
    // Calculate the used time
    unsigned long endTimeInMs = generateTimeInMs();
    unsigned long duration = endTimeInMs - benchmark->startTimeInMs;
    unsigned int seen = 0;
    double perc, reqpermillisec = 0;

    reqpermillisec = benchmark->requests/duration;

    // Print the results
    printf("====== %s ======\r\n", benchmark->title);
    printf("The benchmark sends %d requests %ld milli seconds.\r\n", benchmark->requests, duration);
    printf("The test uses %d parallel clients.\r\n", benchmark->clients);
    printf("The test uses %d bytes of payload.\r\n", benchmark->payload);

    for(int i = 0; i < MAX_LATENCY; i++)
    {
        seen += benchmark->times[i];
        perc = (seen*100.0)/benchmark->requests;
        printf("%.2f%% <= %d milli seconds \r\n", perc, i);
    }
    printf("%.2f requests per milli second\r\n", reqpermillisec);

    // Free resources
    cleanBenchmark(benchmark);
}

struct benchmarkData * prepareGetBenchmark(struct gengetopt_args_info *options)
{
    return prepareBenchmark("GET", options);
}

struct benchmarkData * prepareSetBenchmark(struct gengetopt_args_info *options)
{
    return prepareBenchmark("SET", options);
}

void cleanOptions(struct gengetopt_args_info *options)
{
    cmdline_parser_free(options);
    free(options);
}

char * generateConnectionUrl(char **entries, unsigned int count)
{
    char *connectionUrl;
    const char *server_prefix = "--SERVER=";
    const unsigned int server_prefix_len = strlen(server_prefix);
    unsigned int len = 0;
    for (int i = 0; i < count; i++)
    {
        len += server_prefix_len + strlen(entries[i]) + 1;
    }
    connectionUrl = (char *)malloc(len * sizeof(char));
    if(connectionUrl)
    {
        memcpy(connectionUrl, "\0", 1);
        for (int i = 0; i < count; i++)
        {
            char * temp = (char*)malloc((server_prefix_len + strlen(entries[i]) + 1 )*sizeof(char));
            if(temp)
            {
                if(count == 1 || i == 0)
                {
                    sprintf(temp, "%s%s", server_prefix, entries[i]);
                }
                else
                {
                    sprintf(temp, " %s%s", server_prefix, entries[i]);
                }
                strcat(connectionUrl, temp);
                free(temp);
            }
        }
        return connectionUrl;
    }
    else
    {
        return NULL;
    }
}

void * getTask(void *args)
{
    struct taskData *data = (struct taskData *)args;
    for (int i = 0; i < MAX_LATENCY; i++)
    {
        data->times[i] = 0;
    }
    for (int i = 0; i < data->requests; i++)
    {
        unsigned long start = generateTimeInMs();
        memcached_return result;
        size_t valuelength;
        uint32_t flags;
        char * tmp = memcached_get(data->memc, data->key, strlen(data->key), &valuelength, &flags, &result);
        if(tmp != NULL)
        {
            unsigned long end = generateTimeInMs();
            unsigned long duration = end - start;
            if(duration > MAX_LATENCY)
            {
                duration = MAX_LATENCY;
            }
            data->times[duration]++;
            free(tmp);
        }
    }
    pthread_exit(0);
}

void executeGetBenchmark(struct gengetopt_args_info *options, char *connectionUrl)
{
    struct benchmarkData * getData = prepareGetBenchmark(options);
    if(getData != NULL)
    {
        memcached_st * tmp = memcached(connectionUrl, strlen(connectionUrl));
        if(tmp != NULL)
        {
            char * key = generateRandomValue(options->datasize_arg);
            char * value = generateRandomValue(options->datasize_arg);
            if(key != NULL && value != NULL)
            {
                memcached_return_t result = memcached_set(tmp, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);
                if(result == MEMCACHED_SUCCESS)
                {
                    free(value);

                    pthread_t * workers = (pthread_t *) malloc(options->clients_arg * sizeof(pthread_t));
                    if(workers != NULL)
                    {
                        struct taskData ** taskDatas = (struct taskData **)malloc(options->clients_arg * sizeof(struct taskData *));
                        if(taskDatas != NULL)
                        {
                            for (int i = 0; i < options->clients_arg; i++)
                            {
                                taskDatas[i] = NULL;
                            }

                            // setup the parameter structure for the threads
                            for (int i = 0; i < options->clients_arg; i++)
                            {
                                struct taskData * data = (struct taskData *)malloc(sizeof(struct taskData));
                                if(data != NULL)
                                {
                                    data->memc = memcached(connectionUrl, strlen(connectionUrl));
                                    data->payload = options->datasize_arg;
                                    data->key = key;
                                    data->requests = options->requests_arg;
                                    taskDatas[i] = data;
                                }
                            }

                            // create the threads
                            for (int i = 0; i < options->clients_arg; i++)
                            {
                                if(taskDatas[i] != NULL)
                                {
                                    pthread_create(&workers[i], NULL, getTask, (void *)taskDatas[i]);
                                }
                            }

                            // join the threads
                            for (int i = 0; i < options->clients_arg; i++)
                            {
                                pthread_join(workers[i], NULL);
                            }

                            // combine the results
                            for (int i = 0; i < options->clients_arg; i++)
                            {
                                unsigned int * times = taskDatas[i]->times;
                                for (int j = 0; j < MAX_LATENCY; j++)
                                {
                                    getData->times[j] += times[j];
                                }
                                
                            }

                            free(key);
                            
                            // cleaning
                            for (int i = 0; i < options->clients_arg; i++)
                            {
                                if(taskDatas[i] != NULL)
                                {
                                    memcached_free(taskDatas[i]->memc);
                                    free(taskDatas[i]);
                                }
                            }
                            free(taskDatas);
                            free(workers);
                        }
                        else
                        {
                            free(workers);
                            free(key);
                        }
                    }
                    else
                    {
                        free(key);
                    }
                }
                else
                {
                    free(key);
                    free(value);
                }
            }
            memcached_free(tmp);
        }
        finishBenchmark(getData);
    }
}

void * sendTask(void *args)
{
    struct taskData *data = (struct taskData *) args;
    for(int i = 0; i < MAX_LATENCY; i++)
    {
        data->times[i] = 0;
    }
    for (int i = 0; i < data->requests; i++)
    {
        char *key = generateRandomValue(data->payload);
        char *value = generateRandomValue(data->payload);
        unsigned long start = generateTimeInMs();
        if(key != NULL && value != NULL)
        {
            memcached_return_t result = memcached_set(data->memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);
            free(key);
            free(value);
            if(result == MEMCACHED_SUCCESS)
            {
                unsigned long end = generateTimeInMs();
                unsigned long duration = (size_t)end - start;
                if(duration > MAX_LATENCY)
                {
                    duration = MAX_LATENCY;
                }
                data->times[duration]++;
            }
        }
    }
    pthread_exit(0);
}

void executeSetBenchmark(struct gengetopt_args_info *options, char *connectionUrl)
{
    struct benchmarkData * setData = prepareSetBenchmark(options);
    if(setData != NULL)
    {
        //define the thread array and result array
        pthread_t * workers = (pthread_t *) malloc(options->clients_arg * sizeof(pthread_t));
        if(workers != NULL)
        {
            struct taskData ** taskDatas = (struct taskData **)malloc(options->clients_arg * sizeof(struct taskData *));
            if(taskDatas != NULL)
            {
                for(int i = 0; i < options->clients_arg; i++)
                {
                    taskDatas[i] = NULL;
                }

                // setup the parameter structure for the threads
                for(int i = 0; i < options->clients_arg; i++)
                {
                    struct taskData * data = (struct taskData *)malloc(sizeof(struct taskData));
                    if(data != NULL)
                    {
                        data->memc = memcached(connectionUrl, strlen(connectionUrl));
                        data->payload = options->datasize_arg;
                        data->requests = options->requests_arg;
                        taskDatas[i] = data;
                    }
                }

                // create the threads
                for(int i = 0; i < options->clients_arg; i++)
                {
                    if(taskDatas[i] != NULL)
                    {
                        pthread_create(&workers[i], NULL, sendTask, (void *)taskDatas[i]);
                    }
                }

                // join the threads
                for(int i = 0; i < options->clients_arg; i++)
                {
                    pthread_join(workers[i], NULL);
                }

                // combine the results
                for(int i = 0; i < options->clients_arg; i++)
                {
                    unsigned int * times = taskDatas[i]->times;
                    for(int j = 0; j < MAX_LATENCY; j++)
                    {
                        setData->times[j] += times[j];
                    }
                }

                for(int i = 0; i < options->clients_arg; i++)
                {
                    if(taskDatas[i] != NULL)
                    {
                        memcached_free(taskDatas[i]->memc);
                        free(taskDatas[i]);
                    }
                }
                free(taskDatas);
                free(workers);
            }
            else
            {
                free(workers);
            }
        }
        finishBenchmark(setData);
    }
}

int main(int argc, char *argv[])
{
    struct gengetopt_args_info *options = (struct gengetopt_args_info *) malloc(sizeof(struct gengetopt_args_info));
    char *connectionUrl;

    if(options)
    {
        if (cmdline_parser(argc, argv, options) == 0)
        {
            unsigned int count = options->url_given ? options->url_given : 1;
            options->datasize_arg = (options->datasize_arg < 0 || options->datasize_arg > 30000) ? 3 : options->datasize_arg;
            connectionUrl = generateConnectionUrl(options->url_arg, count);

            if(connectionUrl != NULL)
            {
                for (int i = 0; i < options->test_given; i++) {
                    enum enum_test test_method = options->test_arg[i];

                    switch (test_method) {
                        case test_arg_get:
                        {
                            executeGetBenchmark(options, connectionUrl);
                            break;
                        }
                        case test_arg_set:
                        {
                            executeSetBenchmark(options, connectionUrl);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
                free(connectionUrl);
                cleanOptions(options);
                exit(EXIT_SUCCESS);
            }
            else
            {
                cleanOptions(options);
                printf("The connection string couldn't be build.\r\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            cleanOptions(options);
            printf("The commandline options couldn't parsed.\r\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("The options struct couldn't be created.\r\n");
        exit(EXIT_FAILURE);
    }
 }
