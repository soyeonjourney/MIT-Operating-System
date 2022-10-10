#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXNUM 35

void sieve(int *pleft) {
    int pright[2];
    int prime, num;

    close(pleft[1]);
    if (read(pleft[0], &prime, sizeof(prime)) != sizeof(prime)) {
        fprintf(2, "read error\n");
        exit(1);
    }
    printf("prime %d\n", prime);

    pipe(pright);
    if (fork() == 0) {
        // child
        sieve(pright);
    } else {
        // parent
        close(pright[0]);
        while (read(pleft[0], &num, sizeof(num)) == sizeof(num)) {
            if (num % prime != 0) {
                if (write(pright[1], &num, sizeof(num)) != sizeof(num)) {
                    fprintf(2, "write error\n");
                    exit(1);
                }
            }
        }
        close(pright[1]);
        wait(0);
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    int pleft[2];

    pipe(pleft);
    if (fork() == 0) {
        // child
        sieve(pleft);
    } else {
        // parent
        close(pleft[0]);
        for (int i = 2; i <= MAXNUM; ++i) {
            if (write(pleft[1], &i, sizeof(i)) != sizeof(i)) {
                fprintf(2, "write error\n");
                exit(1);
            }
        }
        close(pleft[1]);
        wait(0);
    }

    exit(0);
}
