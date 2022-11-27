// gcc myShell.c -o shell
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>

int main() {
    DIR * dir;
    if( (dir =opendir(".")) == NULL){
        perror("cannot open");
        exit(1);
        
    }
    struct dirent * dp;
    while(( dp = readdir(dir))!= NULL){
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        printf("%s \n",dp->d_name);
    }
    closedir(dir);
    return 0;
}