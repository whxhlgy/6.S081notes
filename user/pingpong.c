#include "user.h"
#include "kernel/types.h"
#include "kernel/stat.h"

int main(int argc, char* argv[])
{
    int p[2];
    pipe(p);
    int status;
    char buf[512];

    if (fork() == 0) {
        while (1) {
            if (read(p[0], buf, 1) > 0) {
                fprintf(1, "%d: received ping\n", getpid());
                write(p[1], buf, 1);
                exit(0);
            }
        } 
    } else {
        write(p[1], "p", 1);
        wait(&status);
        if (read(p[0], " ", 1) > 0) {
            fprintf(1, "%d: received pong\n", getpid());
            exit(0);
        } else {
            fprintf(2, "unexpect exit\n");
            exit(-1);
        }
    }
}
