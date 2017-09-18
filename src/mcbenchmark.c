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
        test->start_time = time(NULL);
        test->title = title;
    }
    return test;
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
    const char *server_prefix = "--SERVER";
    const size_t server_prefix_len = strlen(server_prefix);
    size_t len = 0;
    for (int i = 0; i < count; i++)
    {
        len += server_prefix_len + strlen(entries[i]) + 2;
    }
    connection_string = (char *)malloc(len * sizeof(char));
    if(connection_string)
    {
        memcpy(connection_string, "\0", 1);
        for (int i = 0; i < count; i++)
        {
            char *temp = (char *)malloc((server_prefix_len + strlen(entries[i]) + 2)*sizeof(char));
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
        //TODO
    }
    return connection_string;
}

memcached_return_t set(memcached_st *memc, const char *key, const char *value, time_t t1, uint32_t t2)
{
    memcached_return_t result = memcached_set(memc, key, strlen(key), value, strlen(value), t1, t2);
    if(result != MEMCACHED_SUCCESS)
    {
        //TODO
    }
    return result;
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
            // Prepare get test
            struct benchmark * set_test = prepare_set_benchmark();
            if(set_test)
            {
                memcached_st *memc = memcached(connection_string, strlen(connection_string));
                if(memc)
                {
                    const char *key = "foo_123";
                    const char *value = "foo_123";
                    memcached_return_t result = set(memc, key, value, (time_t)0, (uint32_t)0);
                    memcached_free(memc);
                    if(result == MEMCACHED_SUCCESS)
                    {
                        // Calculate the used time
                        set_test->end_time = time(NULL);
                        double duration = difftime(set_test->end_time, set_test->start_time);
                        printf("The benchmark test %s need %f seconds.\r\n", set_test->title, duration);

                        // Free resources
                        clean_options(options);
                        free(connection_string);
                        free(set_test);
                        exit(EXIT_SUCCESS);
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
 }
