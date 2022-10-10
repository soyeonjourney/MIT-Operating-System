#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // pipe from parent/child to child/parent, 0/1 for read/write
    int p2c[2], c2p[2];
    pipe(p2c);
    pipe(c2p);
    char buf[4];
    char *pmsg = "ping";  // constant string ends with implicit '\0'
    char *cmsg = "pong";

    int pid = fork();
    if (pid == 0) {
        // child
        close(p2c[1]);
        close(c2p[0]);

        if (read(p2c[0], buf, 4) != 4) {
            fprintf(2, "child: read error\n");
            exit(1);
        }
        printf("// child received: %s\n", buf);
        printf("%d: received ping\n", getpid());

        if (write(c2p[1], cmsg, 4) != 4) {
            fprintf(2, "child: write error\n");
            exit(1);
        }

        close(p2c[0]);
        close(c2p[1]);
        exit(0);
    } else if (pid > 0) {
        // parent
        close(p2c[0]);
        close(c2p[1]);

        if (write(p2c[1], pmsg, 4) != 4) {
            fprintf(2, "parent: write error\n");
            exit(1);
        }

        if (read(c2p[0], buf, 4) != 4) {
            fprintf(2, "parent: read error\n");
            exit(1);
        }
        printf("// parent received: %s\n", buf);
        printf("%d: received pong\n", getpid());

        close(p2c[1]);
        close(c2p[0]);
        // wait for child to exit
        wait(0);
        exit(0);
    } else {
        // fork failed
        fprintf(2, "fork error\n");
        exit(1);
    }
}
