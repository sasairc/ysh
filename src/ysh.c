/*
 * ysh -  hobby shell
 *
 * ysh.c
 *
 * Copyright (c) 2016 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#include "./config.h"
#include "./ysh.h"
#include "./cmd.h"
#include "./info.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char* argv[])
{
    int     res     = 0,
            index   = 0;

    char    buf[MAXLEN] = {'\0'};

    ysh_t   yt      = {0};

    cmd_t*  cmd     = NULL,
         *  start   = NULL;

    struct  option opts[] = {
        {"command", required_argument,  NULL,   'c'},
        {"help",    no_argument,        NULL,    0 },
        {"version", no_argument,        NULL,    1 },
        {0, 0, 0, 0},
    };

    while ((res = getopt_long(argc, argv, "c:", opts, &index)) != -1) {
        switch (res) {
            case    'c':
                yt.cflag = 1;
                memcpy(buf, optarg, strlen(optarg));
                buf[strlen(optarg)] = '\n';
                break;
            case    0:
                print_usage();
            case    1:
                print_version();
            case    '?':
                return -1;
        }
    }

    /*
     * -c, --comand
     */
    if (yt.cflag == 1) {
        if (parse_cmdline(buf, &cmd, &start) < 0)
            return 1;

        exec_cmd(cmd, 0, STDIN_FILENO);
        release_cmd_t(start);

        return 0;
    }

    /*
     * prompt
     */
    while (1) {
        memset(buf, '\0', MAXLEN);
//      fprintf(stdout, "%s", PROMPT);
        fprintf(stdout, "%s %s", getcwd(NULL, 0), PROMPT);
        fgets(buf, MAXLEN, stdin);
        if (parse_cmdline(buf, &cmd, &start) < 0)
            continue;
        exec_cmd(cmd, 0, STDIN_FILENO);
        release_cmd_t(start);
    }

    return 0;
}
