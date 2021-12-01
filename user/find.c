#include "../kernel/types.h"
#include "../kernel/fs.h"
#include "../kernel/stat.h"
#include "user.h"

/* 将路径中的文件名提取出来 */
char* fmtname(char* path)
{
    static char buf[DIRSIZ + 1];
    char* p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p) + 1);
    return buf;
}
/* 用于创建一个用下划线分割的路径字符串 */
void makePath(char* buf, char* path, char* append)
{
    // path  -->> path/filename
    strcpy(buf, path);
    strcpy(buf + strlen(buf), "/");
    strcpy(buf + strlen(buf), append);
}
/* 搜索函数 */
/* inum 是当前位置所对应的文件号，用来排除.文件 */
void find(char* location, char* filename, int loc_inum, int pre_inum)
{
    int fd;
    struct stat st;
    struct dirent de;

    if ((fd = open(location, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", location);
        exit(0);
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", location);
        exit(0);
    }

    switch (st.type) {
    case T_DEVICE:
    case T_FILE:
        if (strcmp(fmtname(location), filename) == 0) {
            strcpy(location + strlen(location), "\n");
            fprintf(1, location); // 找到了该文件
        }
        close(fd);
        break;

    case T_DIR:
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == loc_inum || de.inum == pre_inum) // 文件夹为.(inum == location.inum)或者..(1)忽略
                continue;
            if (de.inum == 0) // 一般inum为0是文件夹的末尾部分
                continue;
            char nextLoc[512] = "";
            makePath(nextLoc, location, de.name);
            find(nextLoc, filename, de.inum, loc_inum);
        }
        close(fd);
        break;
    }
}
int main(int argc, char* argv[])
{
    if (argc < 3) {
        fprintf(1, "find <root> <filename>\n");
        exit(0);
    }
    find(argv[1], argv[2], 1, 1);
    exit(0);
}
