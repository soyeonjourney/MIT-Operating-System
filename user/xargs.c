#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

int readline(int xargvi, char *xargv[MAXARG]) {
    int bufsize = 512;
    char buf[bufsize];
    int bufi = 0;
    while (read(0, buf + bufi, 1) > 0) {
        if (buf[bufi] == '\n') {
            break;
        }
        bufi++;
        if (bufi == bufsize) {
            fprintf(2, "xargs: input line too long");
            exit(1);
        }
    }
    // no input
    if (bufi == 0) {
        return 0;
    }

    int bufj = 0;
    while (bufj < bufi) {
        if (xargvi > MAXARG) {
            fprintf(2, "xargs: too many arguments\n");
            exit(1);
        }

        // skip space
        while ((bufj < bufi) && (buf[bufj] == ' ')) {
            bufj++;
        }
        int start = bufj;
        // find end
        while ((bufj < bufi) && (buf[bufj] != ' ')) {
            bufj++;
        }
        int end = bufj;
        buf[bufj++] = '\0';

        xargv[xargvi] = (char *)malloc((end - start + 1) * sizeof(char));
        strcpy(xargv[xargvi], buf + start);
        xargvi++;
    }

    return xargvi;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [arg...]\n");
        exit(1);
    }

    char *cmd = (char *)malloc((strlen(argv[1]) + 1) * sizeof(char));
    strcpy(cmd, argv[1]);

    char *xargv[MAXARG];
    for (int i = 0; i < argc - 1; ++i) {
        xargv[i] = (char *)malloc((strlen(argv[i + 1]) + 1) * sizeof(char));
        strcpy(xargv[i], argv[i + 1]);
        xargv[i][strlen(argv[i + 1])] = '\0';
    }
    int xargvi;
    while ((xargvi = readline(argc - 1, xargv)) > 0) {
        xargv[xargvi] = '\0';  // null terminate
        if (fork() == 0) {
            exec(cmd, xargv);
            // exec failed
            fprintf(2, "xargs: exec %s failed\n", cmd);
            exit(1);
        }
        wait(0);
    }

    exit(0);
}
