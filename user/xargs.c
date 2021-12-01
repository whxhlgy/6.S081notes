#include "../kernel/param.h"
#include "../kernel/types.h"
#include "user.h"

void argv_add(char* buf, char* argv_a[])
{
    if (strlen(buf) < 1)
        return;

    int i = 0;
    while (strcmp(argv_a[i], 0) != 0) {
        i++;
    }
    if (i < MAXARG) {
        argv_a[i] = buf;
    }
}
void parse_line(char* line, char* argv_n[])
{
    if (strlen(line) == 0)
        return;
    char* p = line;
    while (1) {
        while (*p != 0 && *p != ' ') {
            p++;
        }
        if (*p == ' ') {
            *p = 0;
            argv_add(line, argv_n);
            p++;
            line = p;
        } else {
            argv_add(line, argv_n);
            break;
        }
    }
}
int main(int argc, char* argv[])
{
    char* my_argv[MAXARG];
    char line[512] = "";
    char* p = line;
    int i, status, n;

    /* read lines */
    while (1) {
        n = read(0, p, 1);

        if (*p == '\n' || n == 0) {
            /* 保证字符串的完整 */
            if (n == 0) {
                *(p + 1) = 0;
            } else {
                *p = 0;
            }
            /* 初始化将传递给子进程的参数列表 */
            for (i = 0; i < MAXARG; i++) {
                if (i < argc - 1) {
                    my_argv[i] = argv[i + 1];
                } else {
                    my_argv[i] = 0;
                }
            }
            parse_line(line, my_argv);
            /* 开始执行命令 */
            if (fork() != 0) {
                wait(&status);
                p = line; // 重置p的位置
            } else {
                exec(my_argv[0], my_argv);
                fprintf(2, "exec error\n");
                exit(-1); // 执行出错，退出
            }

            /* 如果已经到文件末尾，结束 */
            if (n == 0) {
                exit(0);
            }
        } else {
            p++;
        }
    }
    exit(0);
}
