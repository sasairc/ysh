/*
 * ysh -  hobby shell
 *
 * buildin.h
 *
 * Copyright (c) 2016 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#ifndef BUILDIN_H
#define BUILDIN_H

#include "./cmd.h"

extern int ysh_chdir(char** args);
extern int ysh_exit(cmd_t* cmd);
extern int ysh_ret(int ret);
extern int ysh_yasuna(void);

/* BUILDIN_H */
#endif
