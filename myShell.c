// gcc myShell.c -o shell
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include<limits.h>

void init_shell();
int Dir();
int Copy();


int main(int argc , char* argv[]){
    init_shell();
    char* command = malloc(256);
    while(scanf("%s",command) != EOF){
        if(strncmp(command,"exit",4) ==0) break;
        else if(strcmp(command,"DIR") ==0) Dir();
        else if(strncmp(command,"COPY",4) ==0) Copy(command);

    }


    free(command);
    printf("goodbye\n");
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
int Copy(char* str)
{
    char* src_dest = malloc(sizeof(str));
    strcpy(src_dest,str);
    scanf("%s",src_dest);
    char src[128], dest[128];
    if (src_dest[4]!='<')
    {
        perror("Usage: COPY<src><dest>");
        exit(1);
    }
    int i =5;
    int index = 0;
    
    while (1)
    {
        src[index++] = src_dest[i++];
        if (src_dest[i] == '>')
        {
            src[index] = '\0';
            break;
        } 
    }
    if (src_dest[++i]!='<')
    {
        perror("Usage: COPY<src><dest>");
        exit(1);
    }
    i++;
    index=0;
    while (1)
    {
        dest[index++] = src_dest[i++];
        if (src_dest[i] == '>')
        {
            dest[index] = '\0';
            break;
        } 
    }
    
    int fdsrc;
    int fddst;
    if ((fdsrc = open(src ,O_RDONLY)) < 0) //open file 1 for read
    {
        printf("%s\n",src);
        perror(" cant open sorce file");
        exit(1);
    }
    if ((fddst = open(dest ,O_RDONLY | O_WRONLY | O_TRUNC | O_WRONLY|O_CREAT , 0666)) < 0) //open file 2 to write
    {
        perror(" cant open dest file");
        exit(1);
    }
    ssize_t readBytes, wroteBytes;
    char *buf[10];
    lseek(fddst,0,SEEK_SET); //start the writing from the begining of the file
    do
    {
        readBytes = read(fdsrc, buf, 10); //read every time 10 byte from file 1
        if (readBytes < 0)
        {
            printf("I/O errors\n");
            exit(-10);
        }

        wroteBytes = write(fddst, buf, readBytes); // write the bytes we read to file 2
        if (wroteBytes < 0)
        {
            printf("this is the error\n");
            if (errno == EDQUOT)
            {
                printf("ERROR: out of quota.\n");
            }
            else if (errno == ENOSPC)
            {
                printf("ERROR: not enough disk space.\n");
            }
        }
    } while (readBytes > 0);
    free(src_dest);
    close(fdsrc);
    close(fddst);
    return 1;
}