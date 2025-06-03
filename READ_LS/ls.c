#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <dirent.h>

#define BUF_SIZE 8192
#define INPUT_SIZE 256

struct linux_dirent64 {
    uint64_t        d_ino;
    int64_t         d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char            d_name[];
};

const char *file_type(unsigned char d_type) {
    switch (d_type) {
        case DT_REG: return "-";
        case DT_DIR: return "d";
        case DT_LNK: return "l";
        case DT_FIFO: return "p";
        case DT_SOCK: return "s";
        case DT_CHR: return "c";
        case DT_BLK: return "b";
        default: return "?";
    }
}

void run_ls(const char *dir, int show_all, int long_format) {
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent64 *d;
    int bpos;

    fd = syscall(SYS_open, dir, O_RDONLY | O_DIRECTORY);
    if (fd == -1) {
        perror("open");
        return;
    }

    for (;;) {
        nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
        if (nread == -1) {
            perror("getdents64");
            close(fd);
            return;
        }
        if (nread == 0) break;

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent64 *) (buf + bpos);

            if (!show_all && d->d_name[0] == '.') {
                bpos += d->d_reclen;
                continue;
            }

            if (long_format) {
                char fullpath[PATH_MAX];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, d->d_name);

                struct stat st;
                if (stat(fullpath, &st) == -1) {
                    perror("stat");
                    bpos += d->d_reclen;
                    continue;
                }

                printf("%s %10ld ", file_type(d->d_type), st.st_size);

                char timebuf[64];
                struct tm *tm_info = localtime(&st.st_mtime);
                strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M", tm_info);
                printf("%s %s\n", timebuf, d->d_name);
            } else {
                printf("%s  ", d->d_name);
            }

            bpos += d->d_reclen;
        }

        if (!long_format) printf("\n");
    }

    close(fd);
}

int main() {
    char input[INPUT_SIZE];
    char cwd[PATH_MAX];
    char prev_dir[PATH_MAX] = "";

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("\n[%s] $ ", cwd);
        } else {
            printf("\n$ ");
        }

        if (fgets(input, INPUT_SIZE, stdin) == NULL)
            break;

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0)
            continue;

        // parse command
        char *args[4] = {NULL};
        int i = 0;
        char *token = strtok(input, " ");
        while (token && i < 4) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        if (strcmp(args[0], "ls") == 0) {
            int show_all = 0, long_format = 0;
            const char *path = ".";

            for (int j = 1; j < i; j++) {
                if (args[j][0] == '-') {
                    if (strchr(args[j], 'a')) show_all = 1;
                    if (strchr(args[j], 'l')) long_format = 1;
                } else {
                    path = args[j];
                }
            }

            run_ls(path, show_all, long_format);
        }
        else if (strcmp(args[0], "cd") == 0) {
            const char *target = NULL;
            if (i == 1) {
                target = getenv("HOME");
            } else if (strcmp(args[1], "-") == 0) {
                if (strlen(prev_dir) == 0) {
                    printf("cd: không có thư mục trước\n");
                    continue;
                }
                target = prev_dir;
            } else {
                target = args[1];
            }

            if (target) {
                char tmp[PATH_MAX];
                if (getcwd(tmp, sizeof(tmp)) != NULL)
                    strcpy(prev_dir, tmp);

                if (chdir(target) == -1)
                    perror("cd");
            }
        }
        else if (strcmp(args[0], "exit") == 0) {
            break;
        }
        else {
            printf("Lệnh không hỗ trợ: %s\n", args[0]);
        }
    }

    return 0;
}

