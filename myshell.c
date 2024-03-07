#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

void execute(char** args) {
    pid_t pid = fork();
    if(pid == 0) {
        execvp(args[0],args);
    }
}

char* read_line(void) {
    ssize_t size = 0;
    char* line;
    printf("\033[1;32m");
        printf("Enter command >> ");
        printf("\033[0m");
    getline(&line,&size,stdin);
    return line;
}

char** tokenize_a_line(char* line) {
    int index_enumerator = 0;
    int space_enumerator = 0;
    while(line[index_enumerator] != '\n') {
        ++index_enumerator;
        if(line[index_enumerator] == ' ') {
            ++space_enumerator;
        }
    }
    int number_of_tokens = space_enumerator+1;
    char** tokens = malloc(number_of_tokens*sizeof(char*));
    char* token;
    char* corector = " \t\n";
    index_enumerator= 0;
    token = strtok(line,corector);
    while(token!= NULL) {
        tokens[index_enumerator] = token;
        ++index_enumerator;
        token = strtok(NULL,corector);
    }
    tokens[index_enumerator] =  NULL;
    return tokens;
}

void main_loop() {
    int status = 1;
    char* line;
    char** args;
    do {
        line = read_line();
        args = tokenize_a_line(line);
        execute(args);
        free(line);
        free(args);
    }while (status);
    printf("hello \n");
}
int main(int argc, char* argv[]) {
    main_loop();
    return 0;
}