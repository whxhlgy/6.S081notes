#include "user.h"

int main(int argc, char* argv[])
{
    int p[2];
    pipe(p);
    int left, right, status, n;
    //    int eof = -1;
    if (fork() != 0) {
        close(p[0]);
        right = p[1];
        for (int i = 2; i <= 35; i++) {
            write(p[1], &i, sizeof(i));
        }
        close(right); // 关闭输入，将使子进程得到EOF
        wait(&status);
    } else {
        //循环
        while (1) {
            int prime, num;
            left = p[0]; // 这是左边pipe的读端口
            close(p[1]); // 关闭左边pipe的写端口
            pipe(p);     // 创建一个右边的pipe
            /* 读取第一个数 */
            if ((n = read(left, &prime, sizeof(prime))) < 0) {
                fprintf(2, "read error\n");
                exit(-1);
            }
            if (n == 0) {
                exit(0); // 第一个数就没读到，说明是最右边的进程，直接退出
            }
            printf("prime %d\n", prime);

            /* 已经读到了第一个数，再向右分支 */
            if (fork() != 0) {
                close(p[0]); // 关闭右边pipe的读端口
                right = p[1];
                // 读取第二个及以后的数，并发给right
                while (1) {
                    if ((n = read(left, &num, sizeof(prime))) < 0) {
                        fprintf(2, "read error\n");
                        exit(-1);
                    }
                    if (n == 0) {
                        close(right);  // 读到了EOF，关闭输入，向右进程传递EOF
                        wait(&status); // 读取完毕，等待右边进程结束
                        exit(0);
                    }
                    if (num % prime != 0) { // 可能的素数
                        write(right, &num, sizeof(num));
                    }
                }
            } else
                continue;
        }
    }
    //循环结束

    exit(0);
}
