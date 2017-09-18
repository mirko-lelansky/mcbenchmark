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

struct benchmark * prepare_get_benchmark()
{
    return prepare_benchmark("GET-TEST");
}

void clean_options(struct gengetopt_args_info *options)
{
    cmdline_parser_free(options);
    free(options);
}

int main(int argc, char *argv[])
{
    struct gengetopt_args_info *options = (struct gengetopt_args_info *) malloc(sizeof(struct gengetopt_args_info));
    const char *server_prefix = "--SERVER=";
    const size_t server_prefix_len = strlen(server_prefix);
    char * connection_string;

    if (cmdline_parser(argc, argv, options) == 0)
    {
        // Prepare get test
        struct benchmark * get_test = prepare_get_benchmark();
        if(!get_test)
        {
            clean_options(options);
            free(get_test);
            exit(EXIT_FAILURE);
        }

        // Prepare the memcached connection
        unsigned int size = options->url_given == 0 ? 1 : options->url_given;
        size_t len = 0;
        for (int i = 0; i < size; i++)
        {
            len += server_prefix_len + strlen(options->url_arg[i]) + 2;
        }
        connection_string = (char *)malloc(len * sizeof(char));
        if(connection_string)
        {
            memcpy(connection_string, "\0", 1);
            for (int i = 0; i < options->url_given; i++)
            {
                char *temp = (char *)malloc((server_prefix_len + strlen(options->url_arg[i]) + 2)*sizeof(char));
                sprintf(temp, "%s%s ", server_prefix, options->url_arg[i]);
                strcat(connection_string, temp);
                free(temp);
            }
        }
        else
        {
            //TODO
        }

        // Open the connection
        memcached_st *memc = memcached(connection_string, strlen(connection_string));
        if(!memc)
        {
            clean_options(options);
            free(get_test);
            free(connection_string);
            printf("The connection couldn't established.\r\n");
            exit(EXIT_FAILURE);
        }

        char *key ="foo_rand0000000000";
        char *value="xxxxxxx";

        // Send data to memcached
        memcached_return_t rc = memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

        if(rc != MEMCACHED_SUCCESS)
        {
            //TODO
            clean_options(options);
            free(get_test);
            free(connection_string);
            memcached_free(memc);
            printf("The operation wasn't successfully.\r\n");
            exit(EXIT_FAILURE);
        }

        // Close connection and free
        memcached_free(memc);

        // Calculate the used time
        get_test->end_time = time(NULL);
        double duration = difftime(get_test->end_time, get_test->start_time);
        printf("The benchmark test %s need %f seconds.\r\n", get_test->title, duration);

        // Free resources
        cmdline_parser_free(options);
        free(options);
        free(get_test);
        free(connection_string);
        exit(EXIT_SUCCESS);
    }
    else
    {
        clean_options(options);
        printf("The commandline options couldn't parsed.\r\n");
        exit(EXIT_FAILURE);
    }
 }
