#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <libmemcached-1.0/memcached.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "cmdline.h"

#define MAX_VALUES 5000

struct benchmarkData {
    unsigned long startTimeInMs;
    char *title;
    unsigned short payload;
    unsigned int *times;
    unsigned short clients;
    unsigned int requests;
};

struct taskData {
    unsigned short requests;
    char *connectionUrl;
    unsigned short payload;
    unsigned int *results;
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
    char * value = (char *)malloc((size + 3) * sizeof(char));
    if(value)
    {
        memset(value, 'x', size);
        value[size] = '\r';
        value[size + 1] = '\n';
        value[size + 2] = '\0';
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
        unsigned int * values = (unsigned int *)malloc(MAX_VALUES * sizeof(unsigned int));
        if(values)
        {
            benchmark->times = values;
        }
        else
        {
            // TODO
        }
    }
    else
    {
        //TODO
    }
    return benchmark;
}

void finishBenchmark(struct benchmarkData *benchmark)
{
    // Calculate the used time
    long endTimeInMs = generateTimeInMs();
    long duration = endTimeInMs - benchmark->startTimeInMs;
    unsigned int seen = 0;
    double perc, reqpersec = 0;

    reqpersec = benchmark->requests/(duration/1000);
    // Print the results
    printf("====== %s ======\r\n", benchmark->title);
    printf("The benchmark sends %d requests %ld milli seconds.\r\n", benchmark->requests, duration);
    printf("The test uses %d parallel clients.\r\n", benchmark->clients);
    printf("The test uses %d bytes of payload.\r\n", benchmark->payload);

    for(int i = 0; i < MAX_VALUES; i++)
    {
        if(benchmark->times[i])
        {
            seen += benchmark->times[i];
            perc = (seen*100.0)/benchmark->requests;
            printf("%.2f%% <= %d milli seconds \r\n", perc, i);
        }
    }
    printf("%.2f requests per second\r\n", reqpersec);

    // Free resources
    free(benchmark);
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
    char *connection_string;
    const char *server_prefix = "--SERVER=";
    const size_t server_prefix_len = strlen(server_prefix);
    size_t len = 0;
    for (int i = 0; i < count; i++)
    {
        len += server_prefix_len + strlen(entries[i]) + 1;
    }
    connection_string = (char *)malloc(len * sizeof(char));
    if(connection_string)
    {
        memcpy(connection_string, "\0", 1);
        for (int i = 0; i < count; i++)
        {
            if(count == 1)
            {
                char *temp = (char *)malloc((server_prefix_len + strlen(entries[i]) + 1) * sizeof(char));
                if(temp || i == (count-1))
                {
                    sprintf(temp, "%s%s", server_prefix, entries[i]);
                    strcat(connection_string, temp);
                    free(temp);
                }
                else
                {
                    char *temp = (char *)malloc((server_prefix_len + strlen(entries[i]) + 1)*sizeof(char));
                    if(temp)
                    {
                        sprintf(temp, "%s%s ", server_prefix, entries[i]);
                        strcat(connection_string, temp);
                        free(temp);
                    }
                    else
                    {
                        //TODO
                    }
                }
            }
            else
            {
            }
        }
    }
    else
    {
        //TODO
    }
    return connection_string;
}

memcached_return_t set(memcached_st *memc, const char *key, const char *value, time_t expiration_date, uint32_t flags)
{
    memcached_return_t result = memcached_set(memc, key, strlen(key), value, strlen(value), expiration_date, flags);
    if(result != MEMCACHED_SUCCESS)
    {
        //TODO
    }
    return result;
}

char * get(memcached_st *memc, const char *key, size_t value_len, uint32_t flags, memcached_return_t mem_result)
{
    return memcached_get(memc, key, strlen(key), &value_len, &flags, &mem_result);
}

void executeGetBenchmark(struct gengetopt_args_info *options, char *connection_string)
{
    // Prepare get test
    struct benchmarkData * getData = prepareGetBenchmark(options);
    if(getData)
    {
        memcached_st *memc = memcached(connection_string, strlen(connection_string));
        if(memc)
        {
            const char *key = "foo_124";
            char *value = generateRandomValue(options->datasize_arg);
            memcached_return_t result = set(memc, key, value, (time_t)0, (uint32_t)0);
            if(result == MEMCACHED_SUCCESS)
            {
                getData->startTimeInMs = 0;
                char *mem_value = get(memc, key, strlen(value), (time_t)0, result);
                free(value);
                free(mem_value);
                memcached_free(memc);

                if(result == MEMCACHED_SUCCESS)
                {
                    finishBenchmark(getData);
                }
                else
                {
                    cleanOptions(options);
                    free(connection_string);
                    free(getData);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                cleanOptions(options);
                free(connection_string);
                free(getData);
                memcached_free(memc);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            cleanOptions(options);
            free(connection_string);
            free(getData);
            printf("The memcached connection couldn't be established.");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        cleanOptions(options);
        free(connection_string);
        exit(EXIT_FAILURE);
    }
}

void * sendTask(void *args)
{
    struct taskData *data = (struct taskData *) args;
    memcached_st *memc = memcached(data->connectionUrl, strlen(data->connectionUrl));
    if(memc)
    {
        for (int i = 0; i < data->requests; i++)
        {
            long start = generateTimeInMs();
            const char *key = "foo_123";
            char *value = generateRandomValue(data->payload);
            memcached_return_t result = set(memc, key, value, (time_t)0, (uint32_t)0);
            free(value);
            if(result == MEMCACHED_SUCCESS)
            {
                long end = generateTimeInMs();
                long duration = end - start;
                if(duration > MAX_VALUES)
                {
                    duration = MAX_VALUES;
                }
                data->results[duration]++;
            }
        }
        memcached_free(memc);
    }
    else
    {
        // TODO
        free(data);
        exit(EXIT_FAILURE);
    }
    pthread_exit(data);
}

void executeSetBenchmark(struct gengetopt_args_info *options, char *connection_string)
{
    struct benchmarkData * setData = prepareSetBenchmark(options);
    if(setData)
    {
        pthread_t workers[options->clients_arg];
        struct taskData * task_data[options->clients_arg];
        for (int i = 0; i < options->clients_arg; i++)
        {
            struct taskData *data = (struct taskData *) malloc(sizeof(struct taskData));
            if(data)
            {
                data->connectionUrl = connection_string;
                data->payload = options->datasize_arg;
                data->requests = options->requests_arg;
                unsigned int * results = (unsigned int *)malloc(MAX_VALUES * sizeof(unsigned int));
                data->results = results;
                task_data[i] = data;
                int res = pthread_create(&workers[i], NULL, sendTask, (void *) data);
                if(res != 0)
                {
                    free(connection_string);
                    cleanOptions(options);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                free(connection_string);
                cleanOptions(options);
                exit(EXIT_FAILURE);
            }
        }
        for (int i = 0; i < options->clients_arg; i++)
        {
            int res = pthread_join(workers[i], (void **)task_data[i]);
            if(res == 0)
            {
                unsigned int * result = setData->times;
                unsigned int * tmp = task_data[i]->results;
                for(int i = 0; i < MAX_VALUES; i++)
                {
                    result[i] += tmp[i];
                }
                free(task_data[i]);
            }
            else
            {
                free(connection_string);
                cleanOptions(options);
                exit(EXIT_FAILURE);
            }
        }

        finishBenchmark(setData);
    }
}

int main(int argc, char *argv[])
{
    struct gengetopt_args_info *options = (struct gengetopt_args_info *) malloc(sizeof(struct gengetopt_args_info));
    char *connection_string;

    if (cmdline_parser(argc, argv, options) == 0)
    {
        unsigned int count = options->url_given ? options->url_given : 1;
        options->datasize_arg = (options->datasize_arg < 0 || options->datasize_arg > 30000) ? 3 : options->datasize_arg;
        connection_string = generateConnectionUrl(options->url_arg, count);

        if(connection_string)
        {
            for (int i = 0; i < options->test_given; i++) {
                enum enum_test test_method = options->test_arg[i];

                switch (test_method) {
                    case test_arg_get:
                    {
                        executeGetBenchmark(options, connection_string);
                        break;
                    }
                    case test_arg_set:
                    {
                        executeSetBenchmark(options, connection_string);
                        break;
                    }
                    default:
                    {
                        break;
                    }

                }
            }
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
    cleanOptions(options);
    free(connection_string);
    exit(EXIT_SUCCESS);
 }
