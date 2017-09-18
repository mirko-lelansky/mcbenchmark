#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <libmemcached-1.0/memcached.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "cmdline.h"

struct benchmark {
    unsigned int active_clients;
    struct client *clients;
    time_t end_time;
    time_t start_time;
    char *title;
};

struct client {
    memcached_st conn;
};

struct benchmark * prepare_benchmark(char *title)
{
    struct benchmark *test = (struct benchmark *)malloc(sizeof(struct benchmark));
    if(test)
    {
        test->active_clients = 0;
        test->start_time = time(NULL);
        test->title = title;
    }
    return test;
}

void finish_benchmark(struct benchmark *test)
{
    // Calculate the used time
    test->end_time = time(NULL);
    double duration = difftime(test->end_time, test->start_time);
    printf("The benchmark test %s need %f seconds.\r\n", test->title, duration);

    // Free resources
    free(test);
}

struct benchmark * prepare_get_benchmark()
{
    return prepare_benchmark("GET-TEST");
}

struct benchmark * prepare_set_benchmark()
{
    return prepare_benchmark("SET-TEST");
}

void clean_options(struct gengetopt_args_info *options)
{
    cmdline_parser_free(options);
    free(options);
}

char *build_connection_string(char **entries, unsigned int count)
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

void run_get_test(struct gengetopt_args_info *options, char *connection_string)
{
    // Prepare get test
    struct benchmark * get_test = prepare_get_benchmark();
    if(get_test)
    {
        memcached_st *memc = memcached(connection_string, strlen(connection_string));
        if(memc)
        {
            const char *key = "foo_124";
            const char *value = "foo_124";
            memcached_return_t result = set(memc, key, value, (time_t)0, (uint32_t)0);
            if(result == MEMCACHED_SUCCESS)
            {
                get_test->start_time = time(NULL);
                char *mem_value = get(memc, key, strlen(value), (time_t)0, result);
                free(mem_value);
                memcached_free(memc);

                if(result == MEMCACHED_SUCCESS)
                {
                    finish_benchmark(get_test);
                }
                else
                {
                    clean_options(options);
                    free(connection_string);
                    free(get_test);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                clean_options(options);
                free(connection_string);
                free(get_test);
                memcached_free(memc);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            clean_options(options);
            free(connection_string);
            free(get_test);
            printf("The memcached connection couldn't be established.");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        clean_options(options);
        free(connection_string);
        exit(EXIT_FAILURE);
    }
}

void run_set_test(struct gengetopt_args_info *options, char *connection_string)
{
    // Prepare set test
    struct benchmark * set_test = prepare_set_benchmark();
    if(set_test)
    {
        printf("The connection is: %s .\r\n", connection_string);
        memcached_st *memc = memcached(connection_string, strlen(connection_string));
        if(memc)
        {
            const char *key = "foo_123";
            const char *value = "foo_123";
            memcached_return_t result = set(memc, key, value, (time_t)0, (uint32_t)0);
            memcached_free(memc);
            if(result == MEMCACHED_SUCCESS)
            {
                finish_benchmark(set_test);
            }
            else
            {
                clean_options(options);
                free(connection_string);
                free(set_test);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            clean_options(options);
            free(connection_string);
            free(set_test);
            printf("The memcached connection couldn't be established.");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        clean_options(options);
        free(connection_string);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    struct gengetopt_args_info *options = (struct gengetopt_args_info *) malloc(sizeof(struct gengetopt_args_info));
    char *connection_string;

    if (cmdline_parser(argc, argv, options) == 0)
    {
        unsigned int count = options->url_given ? options->url_given : 1;
        connection_string = build_connection_string(options->url_arg, count);

        if(connection_string)
        {
            for (int i = 0; i < options->test_given; i++) {
                enum enum_test test_method = options->test_arg[i];

                switch (test_method) {
                    case test_arg_get:
                    {
                        run_get_test(options, connection_string);
                        break;
                    }
                    case test_arg_set:
                    {
                        run_set_test(options, connection_string);
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
            clean_options(options);
            printf("The connection string couldn't be build.");
            exit(EXIT_FAILURE);
        }

    }
    else
    {
        clean_options(options);
        printf("The commandline options couldn't parsed.\r\n");
        exit(EXIT_FAILURE);
    }
    clean_options(options);
    free(connection_string);
    exit(EXIT_SUCCESS);
 }
