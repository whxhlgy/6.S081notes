#include "../kernel/types.h"
#include "user.h"

int main(int argc, char* argv[])
{
    int p[2], n;
    char str[512];
    pipe(p);
    if (fork() == 0) {
        close(p[1]);
        while ((n = read(p[0], str, sizeof(str))) >= 0) {
            if (n == 0) {
                printf("hahahah\n");
                exit(0);
            }
            printf("%s\n", str);
        }
    } else {
        close(p[0]);
        write(p[1], "wocao", sizeof("wocao"));
        close(p[1]);
        wait(0);
        exit(0);
    }
    exit(0);
}
