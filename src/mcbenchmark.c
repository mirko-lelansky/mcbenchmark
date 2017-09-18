#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <libmemcached-1.0/memcached.h>
#include <time.h>
#include <string.h>
#include "cmdline.h"

struct benchmark {
    char *title;
    time_t start_time;
    time_t end_time;
} benchmark;


int main(int argc, char *argv[])
{
    struct gengetopt_args_info mcbenchmark_info;

    if (cmdline_parser(argc, argv, &mcbenchmark_info) != 0)
    {
        exit(1);
    }
    struct benchmark get_method_test;
    get_method_test.title = "GET-METHOD";
    get_method_test.start_time = time(NULL);

    char **server_urls = mcbenchmark_info.url_arg;
    char *config_prefix="--SERVER=";
    char *config_server = NULL;

    if(mcbenchmark_info.url_given)
    {
        //TODO
    }
    else
    {
        unsigned int config_server_size = strlen(server_urls[0]) + strlen(config_prefix) + 1;
        config_server = malloc(config_server_size * sizeof(char));
        strcpy(config_server, config_prefix);
        strcat(config_server, server_urls[0]);
    }

    memcached_st *memc = memcached(config_server, strlen(config_server));
    if(memc == NULL)
    {
        exit(1);
    }

    char *key ="foo_rand0000000000";
    char *value="xxxxxxx";

    memcached_return_t rc = memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

    if(rc != MEMCACHED_SUCCESS)
    {
        //TODO handling
        printf("An error occured.\r\n");
    }
    memcached_free(memc);

    get_method_test.end_time = time(NULL);
    double duration = difftime(get_method_test.end_time, get_method_test.start_time);
    printf("The benchmark test %s need %f seconds.\r\n", get_method_test.title, duration);

    cmdline_parser_free(&mcbenchmark_info);
    exit(0);
}
