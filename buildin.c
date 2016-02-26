/*
 * ysh -  hobby shell
 *
 * buildin.c
 *
 * Copyright (c) 2016 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#include "./ysh.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int ysh_chdir(char** args)
{
    char*   pwd = NULL;

    if (args[1] == NULL)
        pwd = getenv("HOME");
    else
        pwd = args[1];

    if (chdir(pwd) < 0) {
        perror("chdir");

        return errno;
    }

    return 0;
}

int ysh_exit(cmd_t* cmd)
{
    release_cmd_t(cmd);

    exit(0);
}

void ysh_yasuna(void)
{
    fprintf(stdout, "かわいい！\n");
}
