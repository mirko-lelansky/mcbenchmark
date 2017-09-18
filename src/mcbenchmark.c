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

int main(int argc, char *argv[])
{
    struct gengetopt_args_info *options = (struct gengetopt_args_info *) malloc(sizeof(struct gengetopt_args_info));
    struct benchmark *get_test = (struct benchmark *) malloc(sizeof(struct benchmark));
    const char *server_prefix = "--SERVER";
    char * connection_string;

    if (cmdline_parser(argc, argv, options) != 0)
    {
        cmdline_parser_free(options);
        free(get_test);
        printf("The commandline options couldn't parsed.\r\n");
        exit(EXIT_FAILURE);
    }

    // Prepare get test
    get_test->title = "GET-METHOD";
    get_test->start_time = time(NULL);

    // Prepare the memcached connection
    if(options->url_given)
    {
        //TODO
    }
    else
    {
        unsigned int config_server_size = strlen(options->url_arg[0]) + strlen(server_prefix) + 1;
        connection_string = malloc(config_server_size * sizeof(char));
        strcpy(connection_string, server_prefix);
        strcat(connection_string, options->url_arg[0]);
    }

    // Open the connection
    memcached_st *memc = memcached(connection_string, strlen(connection_string));
    if(!memc)
    {
        cmdline_parser_free(options);
        free(get_test);
        printf("The connection couldn't established.\r\n");
        exit(EXIT_FAILURE);
    }

    char *key ="foo_rand0000000000";
    char *value="xxxxxxx";

    // Send data to memcached
    memcached_return_t rc = memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

    if(rc != MEMCACHED_SUCCESS)
    {
        cmdline_parser_free(options);
        free(get_test);
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
    free(get_test);
    exit(EXIT_SUCCESS);
}
