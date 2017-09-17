#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include "cmdline.h"


int main(int argc, char *argv[])
{
    struct gengetopt_args_info mcbenchmark_info;

    if (cmdline_parser(argc, argv, &mcbenchmark_info) != 0)
    {
        cmdline_parser_free(&mcbenchmark_info);
        exit(1);
    }

    cmdline_parser_free(&mcbenchmark_info);
    exit(0);
}
