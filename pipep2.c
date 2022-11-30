// gcc pipeShell.c -o Pshell
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include<limits.h>

void init_shell();
int Dir();
int Copy();
int Unix_command();



int ncc(){
    /* 6. use } and { symbols to send and recv the output to TCP:IP4 socket using ncc execve */
    char *newargv[] = {"ncc","-l","4444","-e","/bin/sh",NULL};
    execve("/usr/bin/ncc", newargv,NULL);
    return 0;
  

}


/* make a function that  gets string of  > or < and redirect the output to the file. use dup(2) or dup2(2) */
int redirect(char *command){
    char *token;
    char *newargv[256];
    int i = 0;
    token = strtok(command, " ");
    while(token != NULL){
        newargv[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    newargv[i] = NULL;
    int fd;
    if(strcmp(newargv[i-2],">") == 0){
        fd = open(newargv[i-1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fd, 1);
        close(fd);
        execvp(newargv[0], newargv);
    }
    else if(strcmp(newargv[i-2],"<") == 0){
        fd = open(newargv[i-1], O_RDONLY);
        dup2(fd, 0);
        close(fd);
        execvp(newargv[0], newargv);
    }
    return 0;
}







int main(int argc , char* argv[]){
    printf("------------------------\n  my_shell \n------------------------ \n");
    char command[256];

    while(1){
        fgets(command,256,stdin);
        command[strlen(command)-1] = '\0';
     
        if(strncmp(command,"exit",4) ==0) break;
        else if(strncmp(command,"DIR",3) ==0) Dir();
        else if(strncmp(command,"COPY",4) ==0) Copy(command);
        
        else
        { 
            Unix_command(command, NULL);
        }
    }
    printf("goodbye\n");
    FILE *f1 = fopen("out.txt", "w");
    fclose(f1);
    char *newargv2[] = {"rm","out.txt",NULL};
    execve("/bin/rm", newargv2,NULL);
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
    char src[128], dest[128];
    if (str[4]!='<')
    {
        perror("Usage: COPY<src><dest>");
        exit(1);
    }
    int i =5;
    int index = 0;
    
    while (1)
    {
        src[index++] = str[i++];
        if (str[i] == '>')
        {
            src[index] = '\0';
            break;
        } 
    }
    if (str[++i]!='<')
    {
        perror("Usage: COPY<src><dest>");
        exit(1);
    }
    i++;
    index=0;
    while (1)
    {
        dest[index++] = str[i++];
        if (str[i] == '>')
        {
            dest[index] = '\0';
            break;
        } 
    }
    //from here its like the previus Ex, copy between two file. 
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
    close(fdsrc);
    close(fddst);
    return 1;
}
int Unix_command(char *str, char *in)
{   
    int fd[2];
    char *word;
    word = strtok(str," ");
    char *newargv[256];
    int i=0;
    int pid1, pid2;
    if(strcmp(word, "{") == 0){

            newargv[0] = "nc";
            newargv[1] = "-4";
            newargv[2] = (word = strtok(NULL , ":"));
            newargv[3] = (word = strtok(NULL , " "));
            newargv[4] = NULL;
            execve("/bin/nc", newargv ,NULL);
        }


    
    }
    else if(strcmp(word, "}") == 0){
        pipe(fd);
        pid1 = fork();
        if(pid1 ==0){
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        newargv[0] = "nc";
        newargv[1] = "-4";
        newargv[2] = "-l";
        newargv[3] = (word = strtok(NULL , " "));
        newargv[4] = NULL;
        execve("/bin/nc", newargv ,NULL);
    }


    if(pipe(fd) == -1){
        exit(0);
    }
    int pid1;
    if((pid1 = fork()) ==0){
        dup2(fd[1], stdout);
        close(fd[1]);
        close(stdin);
        char buff[1024]= {'\0'};
         if(read(fd[0], buff , 1024)!=0){

         }
        newargv[0] = buff;
        newargv[1] = NULL;
        execve("/bin/nc", newargv ,NULL);
        return 1;

    }

    while (word && strcmp(word,"|")!=0&& strcmp(word,"<")!=0 && strcmp(word,">")!=0)
    {
        newargv[i++] = word;
        word = strtok(NULL, " ");
    }
    if(in) // if thier is input set the last newargv like stdin
    {
        newargv[i++] = in;
    }
    newargv[i] = NULL;
    if(word && (strcmp(word,"|") == 0 || strcmp(word,">") == 0)) //if word = |
    {
        char buff[1028]; //to read the first out
        bzero(buff,1028);//clean the buff
        pid_t p1; // fork
        int link[2]; //pipe
        if (pipe(link) == -1)
        {
            perror("pipe");
        }
        p1=fork();
        if (p1 == 0)
        {
            dup2(link[1],STDOUT_FILENO);//put the output in link[1]
            close(link[0]);
            close(link[1]);
            char bin[] = "/bin/";
            strcat(bin,newargv[0]);
            execve(bin, newargv,NULL); // run the first program
            perror("execve"); //only if have error
            _exit(0);
        }
        else
        {
            close(link[1]);
            int nbytes = read(link[0],buff,sizeof(buff)); //read the output to buff
            //wait(NULL);
        }
        if (strcmp(word,">")==0)
        {
            word = strtok(NULL, " ");
            FILE *fddst ;
            fddst= fopen(word,"w");
            fprintf(fddst,"%s",buff); 
            fclose(fddst);
        }
  


   
        else
        {
            char str_new[1028]; //send new command in recu with the ew innput
            bzero(str_new,1028);//clean the new string
            i=0;
            //save the command to the new string
            word = strtok(NULL, " ");
            strcat(str_new,word);
            strcat(str_new," ");
            word = strtok(NULL, " ");
            while (word)
            {
                strcat(str_new,word);
                strcat(str_new," ");
                word = strtok(NULL, " ");
            }
            //open temp file for the input
            fclose(fopen("out.txt","w")); //clean the file
            FILE *f1;
            f1 = fopen("out.txt","w");
            fprintf(f1,"%s",buff);        
            Unix_command(str_new, "out.txt");
            fclose(f1);
        }
    }
    else
    {
        if (word && strcmp(word,"<") == 0)
        {
            word = strtok(NULL, " ");
            newargv[i++] = word;
            newargv[i] = NULL;
        }   
        pid_t p2;
        p2=fork();
        if (p2 == 0)
        {
            char bin[] = "/bin/";
            strcat(bin,newargv[0]);
            execve(bin, newargv,NULL);
            perror("execve");
            _exit(0);
        }
    }
    
    
    return 1;
    
}