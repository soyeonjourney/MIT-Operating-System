#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *getfname(char *path) {
    char *p;

    // find first character after last slash
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // return filename (w.o. padding)
    return p;
}

void find(char *path, char const *target) {
    char buf[512], *p;
    int fd;
    struct stat st;
    struct dirent de;  // sizeof(de.name) == 14

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
        // case file: judge if it is the target file
        case T_FILE:
            if (strcmp(getfname(path), target) == 0) {
                printf("%s\n", path);
            }
            break;

        // case dir: recursively call find
        case T_DIR:
            // if path is longer than buf (512), print error
            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
                printf("find: path too long\n");
                break;
            }

            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
                if (de.inum == 0 || strcmp(de.name, ".") == 0 ||
                    strcmp(de.name, "..") == 0) {
                    continue;
                }
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = '\0';
                find(buf, target);
            }
            break;

        default:
            break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <target>\n");
        exit(1);
    }

    char *path = argv[1];
    char const *target = argv[2];

    int fd;
    struct stat st;
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }
    if (st.type != T_DIR) {
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        exit(1);
    }
    find(path, target);

    exit(0);
}
