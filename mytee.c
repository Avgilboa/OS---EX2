// ./mytee file1.txt file2.txt
//  gcc mytee.c -o mytee
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(int argc , char* argv[]){
    if(argc < 2){
        perror("Usage <mytee> <file.txt> ...");
        exit(1);
    }
    FILE* f1;
    f1 = fopen(argv[1] , "w+");
    if(!f1){
            perror("can't open file");
            exit(1);
        }
    char ch;
    while((ch = getc(stdin)) != EOF){
        fputc(ch , f1);
    }
    fclose(f1);
    f1 = fopen(argv[1], "r");
    for(int i=2; i<argc;i++){
        fseek(f1,0,0);
        FILE* tmpFile;
        tmpFile = fopen(argv[i], "w");
        if(!tmpFile){
            perror("can't open file");
            exit(1);
        }
        while(fread(&ch,1,1,f1) ==1 ){
            fwrite(&ch, 1,1,tmpFile);
        }
        fclose(tmpFile);
    }
    fclose(f1);
    return 0;
}