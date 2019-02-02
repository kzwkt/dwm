#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define FILE_DIR "/sys/class/backlight/intel_backlight/"
#define MAX_FILE FILE_DIR "max_brightness"
#define CUR_FILE FILE_DIR "brightness"
#define INC_STR "inc"
#define DEC_STR "dec"
#define MAX_BYTES (unsigned int) 8
#define BRIGHT_DIFF (unsigned int) 50
#define MIN_VAL (unsigned int) 0

void usage()
{
    printf("Usage: /mod_bright <inc / dec>\n");
    exit(1);
}

int safe_open(char *file, unsigned int flags)
{
    int fd;
    fd = open(file, flags);

    if (fd == -1) {
        printf("[!!] Failed to open %s. Do you have permission?\n", file);
        exit(1);
    }

    return fd;
}

int safe_read(int fd)
{
    char buff[MAX_BYTES];

    if (read(fd, buff, MAX_BYTES) == -1) {
        printf("[!!] Failed to read file with FD:%d\n", fd);
        exit(1);
    }

    return atoi(buff);
}

void safe_write(int fd, int new_val)
{
    char buff[MAX_BYTES];
    sprintf(buff, "%d", new_val);

    if (write(fd, buff, sizeof(int)) == -1) {
        printf("[!!] Failed to write to file with FD:%d\n", fd);
        exit(1);
    }

    return;
}

int main(int argv, char **argc[])
{
    int fd, cur, max, flag = -1;
    char buff[MAX_BYTES];

    if (argv != 2) usage();

    // Is there a more efficient way of doing this using bitwise operations?
    if (!strncmp(DEC_STR, argc[1], 3)) flag = 0;
    if (!strncmp(INC_STR, argc[1], 3)) flag = 1;
    if (flag == -1) usage();

    // TODO: fix usage of safe_open() so that bit flags can be passed via arg
    // TODO: set some kind of program timeout so that it doesn't have to open, read, write, close every single time called
    fd = open(MAX_FILE, O_RDONLY);
    if (fd  == -1) {
        printf("[!!] Failed to open file %s\n", MAX_FILE);
        exit(1);
    }
    max = safe_read(fd);
    close(fd);

    fd = open(CUR_FILE, O_RDWR);
    if (fd  == -1) {
        printf("[!!] Failed to open file %s\n", CUR_FILE);
        exit(1);
    }
    cur = safe_read(fd);
    lseek(fd, 0, 0);

    if (flag) {
        if (cur >= max - BRIGHT_DIFF) safe_write(fd, max);
        else safe_write(fd, cur + BRIGHT_DIFF);
    } else {
        if (cur <= MIN_VAL + BRIGHT_DIFF) safe_write(fd, MIN_VAL);
        else if (cur == max) safe_write(fd, (max / 50) * 50);
        else safe_write(fd, cur - BRIGHT_DIFF);
    }

    close(fd);
    return 0;
}