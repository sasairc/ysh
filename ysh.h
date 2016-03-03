/*
 * ysh -  hobby shell
 *
 * ysh.h
 *
 * Copyright (c) 2016 sasairc
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar.HocevarHocevar See the COPYING file or http://www.wtfpl.net/
 * for more details.
 */

#ifndef YSH_H
#define YSH_H

#define TCOM    1   /* command */
#define TPAREN  2   /* a ; b */
#define TPIPE   3   /* a | b */
#define TAND    4   /* a && b */
#define TOR     5   /* a || b*/

#define IOREAD  1   /* < */
#define IOHERE  2   /* << */
#define IOWRITE 3   /* > */
#define IOCAT   4   /* >> */

#include <sys/stat.h>

typedef struct {
    int     cflag;
} ysh_t;

typedef struct IO_T {
    short   io_unit;
    short   io_flag;
    char*   io_name;
} io_t;

typedef struct CMD_T {
    int             type;
    char**          args;
    struct IO_T*    io;
    struct CMD_T*   next;
    struct CMD_T*   prev;
} cmd_t;

extern cmd_t* add_cmdline_t(cmd_t** cmd);
extern int set_io_val(char* str, int flag, cmd_t** cmd);
extern int set_cmd_val(char* str, int type, cmd_t** cmd);
extern int parse_cmdline(char* str, cmd_t** dest_cmd, cmd_t** dest_start);
extern int file_redirect(cmd_t* cmd);
extern int check_file_stat(cmd_t* cmd, int is_redirect, mode_t chk);
extern int exec_cmd(cmd_t* cmd, int in_fd);
extern void redirect(int oldfd, int newfd);
extern void mwait(void);
extern void release_cmd_t(cmd_t* cmd);

/* YSH_H */
#endif
