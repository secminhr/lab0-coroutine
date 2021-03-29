#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "console.h"
#include "coroutine.h"

static int old_fd_flag;
static int restore(int fd) {
    tcsetattr(fd, TCSAFLUSH, NULL);
    fcntl(fd, F_SETFL, old_fd_flag);
}

#define ENTER 13
#define CTRL_C 3

static int enableRawMode(int fd)
{
    struct termios raw;
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd, TCSAFLUSH, &raw) < 0)
        goto fatal;
    
    old_fd_flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, old_fd_flag|O_NONBLOCK);
    return 0;

fatal:
    return -1;
}

static int inRawMode = 0;
static char buf[10] = {0};
static unsigned int size = 0;

static int read_cmd_raw() {
    cor_func
    if (!inRawMode) {
        enableRawMode(0);
        inRawMode = 1;
    }
    while (1) {
        cor_start
            int count = read(0, buf+size, 1);
            if (count == -1) {
                yield -1;
            }
            if ((*(buf+size)) == CTRL_C) {
                restore(0);
                exit(0);
            } else if ((*(buf+size)) == ENTER) {
                int tmp = size;
                size = 0;
                yield tmp;
            }
            size++;
            if (size == 10) {
                yield 9;
            }
        cor_end_with_value(-1)
    }
}

char *read_cmd() {
    if (read_cmd_raw() == -1) {
        return NULL;
    }
    return buf;
}
