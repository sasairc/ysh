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

#include "./ysh.h"
#include "./info.h"
#include "./buildin.h"
#include "./config.h"
#include "./string.h"
#include "./file.h"
#include "./memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

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
        exec_cmd(cmd, STDIN_FILENO);
        release_cmd_t(start);

        return 0;
    }

    /*
     * prompt
     */
    while (1) {
        memset(buf, '\0', MAXLEN);
        fprintf(stdout, "%s", PROMPT);
//      fprintf(stdout, "%s %s", getcwd(NULL, 0), PROMPT);
        fgets(buf, MAXLEN, stdin);
        if (parse_cmdline(buf, &cmd, &start) < 0)
            continue;
        exec_cmd(cmd, STDIN_FILENO);
        release_cmd_t(start);
    }

    return 0;
}

int set_io_val(char* str, int flag, cmd_t** cmd)
{
    int     msc = 0,
            off = 0,
            len = 0;

    char*   tmp = NULL,
        *   bak = str;

    io_t*   io  = NULL;

    while ( *str != ';'     &&
            *str != '|'     &&
            *str != '&'     &&
            *str != '<'     &&
            *str != '>'     &&
            *str != '\0'    &&
            *str != '\n') {
        str++;
        len++;
    }
    while (*str != '\0' || *str != '\n') {
        if (*str == '&' && *(str + 1) == '&') {
            (*cmd)->type = TAND;
            off++;
            break;
        } else if (*str == '|' && *(str + 1) == '|') {
            (*cmd)->type = TOR;
            off++;
            break;
        }
        str++;
    }

    str = bak;
    if ((tmp = (char*)
                malloc(sizeof(char) * (len + 1))) == NULL)
        return -2;

    memcpy(tmp, str, len);
    tmp[len] = '\0';

    if ((msc = trim(tmp)) > 0)
        msc--;

    if ((io = (io_t*)
                malloc(sizeof(io_t))) == NULL)
        return -2;

    if ((io->io_name = (char*)
                malloc(sizeof(char) * (len + 1))) == NULL)
        return -3;

    memcpy(io->io_name, tmp, strlen(tmp) + 1);
    io->io_flag = flag;
    free(tmp);

    (*cmd)->io = io;

    return len + off + 1;
}

int set_cmd_val(char* str, int type, cmd_t** cmd)
{
    int     len = 0;

    char*   tmp = NULL,
        *   bak = str;

    while ( *str != ';'     &&
            *str != '|'     &&
            *str != '&'     &&
            *str != '<'     &&
            *str != '>'     &&
            *str != '\0'    &&
            *str != '\n') {
        str++;
        len++;
    }

    str = bak;
    if ((tmp = (char*)
                malloc(sizeof(char) * (len + 1))) == NULL)
        return -1;

    memcpy(tmp, str, len);
    tmp[len] = '\0';
    trim(tmp);

    (*cmd)->args = str_to_args(tmp);
    (*cmd)->type = type;
    free(tmp);

    return 0;
}

cmd_t* add_cmdline_t(cmd_t** cmd)
{
    if (((*cmd)->next = (cmd_t*)
                malloc(sizeof(cmd_t))) == NULL)
        return NULL;

    (*cmd)->next->prev = *cmd;
    *cmd = (*cmd)->next;
    (*cmd)->next = NULL;
    (*cmd)->io = NULL;

    return *cmd;
}

int parse_cmdline(char* str, cmd_t** dest_cmd, cmd_t** dest_start)
{
    int     head    = 0,
            tail    = 0;

    short   dqf     = 0;

    cmd_t*  cmd     = NULL,
         *  start   = NULL;

    if ((cmd = (cmd_t*)
                malloc(sizeof(cmd_t))) == NULL) {
        fprintf(stderr, "%s: malloc() failure\n",
                PROGNAME);

        return -1;
    }

    start = cmd;
    cmd->io = NULL;
    cmd->next = NULL;
    cmd->prev = NULL;

    while (1) {
        if (str[head] == ';'        ||
                str[head] == '|'    ||
                str[head] == '&'    ||
                str[head] == '<'    ||
                str[head] == '>'    ||
                str[head] == '\0'   ||
                str[head] == '\n') {
            switch (str[head]) {
                case    ';':
                    if (dqf == 1) {
                        head++;
                        break;
                    }
                    head++;
                    dqf = 1;
                    set_cmd_val(&str[tail], TPAREN, &cmd);
                    break;
                case    '&':
                    if (str[head + 1] == '&') {
                        head += 2;
                        set_cmd_val(&str[tail], TAND, &cmd);
                    }
                    break;
                case    '|':
                    if (str[head + 1] == '|') {
                        head += 2;
                        set_cmd_val(&str[tail], TOR, &cmd);
                    } else {
                        head++;
                        set_cmd_val(&str[tail], TPIPE, &cmd);
                    }
                    break;
                case    '<':
                    if (str[head + 1] == '<') {
                        head += 2;
                        set_cmd_val(&str[tail], TCOM, &cmd);
                        tail = head;
                        head += set_io_val(&str[tail], IOHERE, &cmd);
                    } else {
                        head++;
                        set_cmd_val(&str[tail], TCOM, &cmd);
                        tail = head;
                        head += set_io_val(&str[tail], IOREAD, &cmd);
                    }
                    break;
                case    '>':
                    if (str[head + 1] == '>') {
                        head += 2;
                        set_cmd_val(&str[tail], TCOM, &cmd);
                        tail = head;
                        head += set_io_val(&str[tail], IOCAT, &cmd);
                    } else {
                        head++;
                        set_cmd_val(&str[tail], TCOM, &cmd);
                        tail = head;
                        head += set_io_val(&str[tail], IOWRITE, &cmd);
                    }
                    break;
                case    '\0':
                case    '\n':
                    set_cmd_val(&str[tail], TCOM, &cmd);
                    break;
            }
            if (str[head] == '\n' || str[head] == '\0')
                break;

            cmd = add_cmdline_t(&cmd);
            tail = head;
            head++;
        } else {
            dqf = 0;
            head++;
        }
    }
    if (cmd->args[0][0] == '\0')
        goto NOCMD;

    *dest_cmd = start;
    *dest_start = start;

    return 0;

NOCMD:

    release_cmd_t(start);

    return -2;

}

int file_redirect(cmd_t* cmd)
{
    int fd  = 0;
    
    switch (cmd->io->io_flag) {
        case    IOREAD:
        case    IOHERE:
            if (check_file_stat(cmd, 1,
                        S_IRUSR |
                        S_IRGRP |
                        S_IROTH) < 0)
                return -1;
            if ((fd = open(cmd->io->io_name,
                            O_RDONLY)) < 0) {
                perror("ysh: fopen");
                close(fd);

                return -1;
            }
            close(0);
            dup2(fd, 0);
            break;
        case    IOWRITE:
            if (check_file_stat(cmd, 1,
                        S_IWUSR |
                        S_IWGRP |
                        S_IWOTH) < 0)
                return -1;
            if ((fd = open(cmd->io->io_name,
                            O_WRONLY    |
                            O_CREAT     |
                            O_TRUNC, 0666)) < 0) {
                perror("ysh: fopen");
                close(fd);

                return -1;
            }
            close(1);
            dup2(fd, 1);
            break;
        case    IOCAT:
            if (check_file_stat(cmd, 1,
                        S_IWUSR |
                        S_IWGRP |
                        S_IWOTH) < 0)
                return -1;
            if ((fd = open(cmd->io->io_name,
                            O_WRONLY    |
                            O_CREAT     |
                            O_APPEND, 0666)) < 0) {
                perror("ysh: fopen");
                close(fd);

                return -1;
            }
            close(1);
            dup2(fd, 1);
            break;
    }
    close(fd);

    return 0;
}

int check_file_stat(cmd_t* cmd, int is_redirect, mode_t chk)
{
    char*       path    = NULL;

    struct stat st;

    if (is_redirect == 1)
        path = cmd->io->io_name;
    else
        path = cmd->args[0];

    if (stat(path, &st) < 0) {
        switch (errno) {
            case    ENOENT:
                if (is_redirect == 1)
                    return 0;

                fprintf(stderr, "%s: no such file or directory: %s\n",
                        PROGNAME, path);

                return -1;
            case    EACCES:
                fprintf(stderr, "%s: permission denied: %s\n",
                        PROGNAME, path);

                return -2;
        }
    }

    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        fprintf(stderr, "%s: permission denied: %s\n",
                PROGNAME, path);

        return -3;
    }
    if ((st.st_mode & chk) == 0) {
        fprintf(stderr, "%s: permission denied: %s\n",
                PROGNAME, path);

        return -4;
    }

    return 0;
}

int exec_cmd(cmd_t* cmd, int in_fd)
{
    int     status  = 0,
            fd[2]   = {0};

    pid_t   pid     = 0;

    /*
     * buildin command
     */
    if (strcmp(cmd->args[0], "cd") == 0) {
        ysh_chdir(cmd->args);
        if (cmd->next != NULL)
            exec_cmd(cmd->next, STDIN_FILENO);

        return 0;
    } else if (strcmp(cmd->args[0], "やすなちゃん") == 0) {
        ysh_yasuna();
        if (cmd->next != NULL)
            exec_cmd(cmd->next, STDIN_FILENO);
        
        return 0;
    }
    if (strcmp(cmd->args[0], "exit") == 0)
        ysh_exit(cmd);

    /*
     * pipe (last)
     */
    if (cmd->prev != NULL) {
        if (cmd->prev->type == TPIPE && cmd->type != TPIPE) {
            redirect(in_fd, STDIN_FILENO);
            if (cmd->io != NULL) {
                if (file_redirect(cmd) < 0)
                    exit(1);
            }
            if (cmd->args[0][0] == '.'  &&
                        (cmd->args[0][1] == '/' || cmd->args[0][1] == '.')) {
                if (check_file_stat(cmd, 0, S_IXUSR | S_IXGRP | S_IXOTH) < 0)
                    exit(1);
            }
            execvp(cmd->args[0], cmd->args);
            fprintf(stderr, "%s: command not found: %s\n",
                    PROGNAME, cmd->args[0]);

            exit(1);
        }
    }

    /*
     * command
     */
    if (cmd->type != TPIPE) {
        switch (pid = fork()) {
            case    -1:
                fprintf(stderr, "%s: fork() failure",
                        PROGNAME);

                return errno;
            case    0:
                if (cmd->io != NULL) {
                    if (file_redirect(cmd) < 0)
                        exit(1);
                }
                if (cmd->args[0][0] == '.'  &&
                            (cmd->args[0][1] == '/' || cmd->args[0][1] == '.')) {
                    if (check_file_stat(cmd, 0, S_IXUSR | S_IXGRP | S_IXOTH) < 0)
                        exit(1);
                }
                execvp(cmd->args[0], cmd->args);
                fprintf(stderr, "%s: command not found: %s\n",
                        PROGNAME, cmd->args[0]);

                exit(errno);
            default:
                if (waitpid(pid, &status, 0) < 0)
                    perror("waitpid");
                if (cmd->next != NULL) {
                    /*
                     * 1. &&
                     * 2. ||
                     * 3. ;
                     */
                    if (WEXITSTATUS(status) == 0        &&
                            cmd->type == TAND) {
                        exec_cmd(cmd->next, STDIN_FILENO);
                    } else if (WEXITSTATUS(status) != 0 &&
                            cmd->type == TOR) {
                        exec_cmd(cmd->next, STDIN_FILENO);
                    } else if (cmd->type == TPAREN      ||
                            cmd->type == TCOM) {
                        exec_cmd(cmd->next, STDIN_FILENO);
                    }
                }

                return 0;
        }
    }

    /*
     * pipe
     */
    if (cmd->prev != NULL) {
        if (cmd->prev->type != TPIPE)
            pid = getpid();
    } else {
        pid= getpid();
    }
    switch (fork()) {
        case    -1:
            fprintf(stderr, "%s: fork() failure",
                    PROGNAME);

            return errno;
        case    0:
            if (pipe(fd) < 0) {
                fprintf(stderr, "%s: pipe() failure\n",
                        PROGNAME);

                exit(errno);
            }
            switch (fork()) {
                case    -1:
                    fprintf(stderr, "%s: fork() failure\n",
                            PROGNAME);

                    exit(1);
                case    0:
                    close(fd[0]);
                    redirect(in_fd, STDIN_FILENO);
                    redirect(fd[1], STDOUT_FILENO);
                    if (cmd->args[0][0] == '.'  &&
                                (cmd->args[0][1] == '/' || cmd->args[0][1] == '.')) {
                        if (check_file_stat(cmd, 0, S_IXUSR | S_IXGRP | S_IXOTH) < 0)
                            exit(1);
                    }
                    execvp(cmd->args[0], cmd->args);
                    fprintf(stderr, "%s: command not found: %s\n",
                            PROGNAME, cmd->args[0]);

                    exit(errno);
                default:
                    close(fd[1]);
                    close(in_fd);
                    if (cmd->next != NULL)
                        exec_cmd(cmd->next, fd[0]);

                    exit(1);
            }
        default:
            status = mwait();
            if (pid == getpid()) {
                while (cmd->next != NULL && cmd->type == TPIPE)
                    cmd = cmd->next;
                if (cmd->next != NULL) {
                    /*
                     * 1. &&
                     * 2. ||
                     * 3. ;
                     */
                    if (WEXITSTATUS(status) == 0        &&
                            cmd->type == TAND) {
                        exec_cmd(cmd->next, STDIN_FILENO);
                    } else if (WEXITSTATUS(status) != 0 &&
                            cmd->type == TOR) {
                        exec_cmd(cmd->next, STDIN_FILENO);
                    } else if (cmd->type == TPAREN      ||
                            cmd->type == TCOM) {
                        exec_cmd(cmd->next, STDIN_FILENO);
                    }
                }
            } else {
                exit(0);
            }
    }

    return 0;
}

int mwait(void)
{
    int     status  = 0;

    pid_t   pid     = 0;

    while (1) {
        if ((pid = wait(&status)) == -1) {
            if (errno == ECHILD)
                break;
            else if (errno == EINTR)
                continue;
        }
    }

    return status;
}

void redirect(int oldfd, int newfd)
{
    if (oldfd != newfd)
        if (dup2(oldfd, newfd) == -1)
            close(oldfd);

    return;
}

void release_cmd_t(cmd_t* cmd)
{
    cmd_t*  tmp = NULL;

    while (cmd != NULL) {
        tmp = cmd->next;
        if (cmd->io != NULL) {
            if (cmd->io->io_name != NULL) {
                free(cmd->io->io_name);
            }
            free(cmd->io);
        }
        free2d(cmd->args, p_count_file_lines(cmd->args));
        free(cmd);
        cmd = tmp;
    }

    return;
}
