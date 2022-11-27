// gcc myShell.c -o shell
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>

void init_shell();



int main(int argc , char* argv[]){
    init_shell();
    char* command = malloc(256);
    while(scanf("%s",command) != EOF){
        if(strncmp(command,"exit",4) ==0) break;
        
    }


    free(command);
    printf("goodbye");
    return 0;

}
void init_shell(){
    printf("------------------------\n  my_shell \n------------------------ \n");
}
int Dir(){
    DIR * dir;
    if( (dir =opendir(".")) == NULL){
        perror("cannot open");
        exit(0);
        
    }
    struct dirent * dp;
    while(( dp = readdir(dir))!= NULL){
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        printf("%s \n",dp->d_name);
    }
    closedir(dir);
    return 1;
}