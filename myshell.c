#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void execute(char** args);
int built_in_functions(char** args);
void sigint_handler();
void sigtstp_handler();
char* read_line();
char** tokenize_a_line(char* line);
void ampersant_service(char** args);
int did_ampersant_occured(char** args);
int get_ampersant_position(char** args);
int get_array_size(char** args);
char** line_parser(char* line);

void main_loop();

pid_t current_foreground_pid;

void execute(char** args) {
    char** left_args;
    char** right_args;
    if(built_in_functions(args) == 1) {
        return ;
    }
    printf("%d \n",did_ampersant_occured(args));
    if(did_ampersant_occured(args)) {
        printf("tak \n");
        int ampersant_position = get_ampersant_position(args);
        int array_size = get_array_size(args) - ampersant_position;
        printf("%d %d \n",ampersant_position,array_size);
        left_args = malloc((ampersant_position+1)*sizeof(char*));
        right_args = malloc((array_size-ampersant_position)*sizeof(char*));
        printf("tutaj \n");
        for(int i = 0 ; i < ampersant_position ; ++i) {
            left_args[i] = args[i];
        }
        left_args[ampersant_position] = NULL;
        
        for(int i = 1 ; i < array_size - ampersant_position ; ++i) {
            right_args[i] = args[ampersant_position+i];
        }
        right_args[array_size - ampersant_position] = NULL;
        for(int i = 0 ; left_args[i] ; ++i) {
            printf("%s ",left_args[i]);
        }
        printf("\n");
        for(int i = 0 ; right_args[i] ; ++i) {
            printf("%s ",right_args[i]);
        }
        return;
    }
    int status;
    pid_t pid = fork();
    current_foreground_pid = pid;
    if(pid == 0) {
        execvp(args[0],args);
    }
    else if (pid < 0) {
        perror("error");
    }
    waitpid(pid,&status,WUNTRACED);
}
int get_array_size(char** args) {
    int size = 0;
    while(args[size] != NULL) {
        ++size;
    }
    return size;
}
int get_ampersant_position(char** args) {
    int ampersant_position = 0;
    for(int i = 0 ; args[i] != NULL ;++i) {
        if(strcmp(args[i],"&") == 0) {
            ampersant_position = i;
            break;
        }
    }
    return ampersant_position;
}
int did_ampersant_occured(char** args) {
    for(int i = 0 ; args[i] != NULL; ++i) {
        if(strcmp(args[i],"&") == 0) {
            return 1;
        }
    }
    return 0;
}

char** line_parser(char* line) {
    /*int index_enumerator = 0;
    int space_enumerator = 0;
    while(line[index_enumerator] != '\n') {
        ++index_enumerator;
        if(line[index_enumerator] == ' ') {
            ++space_enumerator;
        }
    }
    int number_of_tokens = space_enumerator+1;*/
    //char** tokens = malloc(number_of_tokens*sizeof(char*));
    char** tokens = malloc(64*sizeof(char*));
    char* token;
    char* corector = " \t\n";
    int index = 0;
    token = strtok(line,corector);
    while(token!=NULL) {
        tokens[index] = token;
        ++index;
        token = strtok(NULL,corector);
    }
    tokens[index] = NULL;
    return tokens;
}
void ampersant_service(char** args) {
    int ampersant_position = 0;
    int if_ampersant = 0;
    for(int i = 0 ; args[i] != NULL ; ++i) {
        if(strcmp(args[i],"&") == 1) {
            if_ampersant = 1;
            ampersant_position = i;
            break;
        }
    }
    if(if_ampersant) {

    }
}

void sigtstp_handler() {
    if(kill(current_foreground_pid,SIGTSTP)<0) {
        perror("error");
    }
}

void sigint_handler() {
    if(kill(current_foreground_pid,SIGINT)<0) {
        perror("error");
    }
}
int built_in_functions(char** args) {
    if(args[0]==NULL) {
        return 1;
    }
    else if(strcmp(args[0],"exit") == 0) {
        exit(0);
    }
    else if(strcmp(args[0],"cd") == 0) {
        if(args[1] != NULL) {
            chdir(args[1]);
        }
        return 1;
    }
    return 0;
}

char* read_line(void) {
    ssize_t size = 0;// 20;
    char* line;
    printf("\033[1;32m");
        printf("Enter command >> ");
        printf("\033[0m");
    getline(&line,&size,stdin);
    return line;
}

char** tokenize_a_line(char* line) {
    /*int index_enumerator = 0;
    int space_enumerator = 0;
    int length = strlen(line);
    for(int i = 0 ; i < length ; ++i) {
        if(line[i] == ' ') {
            ++space_enumerator;
        }
    }*/
    /*while(line[index_enumerator] != '\n') {
        ++index_enumerator;
        if(line[index_enumerator] == ' ') {
            ++space_enumerator;
        }
    }*/
    //int number_of_tokens = space_enumerator + 1;
    //char** tokens = malloc((number_of_tokens)*sizeof(char*));
    char** tokens = malloc(64*sizeof(char*));
    char* token;
    char corector[] = " \t\r\n\a";
    int index_enumerator= 0;
    token = strtok(line,corector);
    while(token!= NULL) {
        tokens[index_enumerator] = strdup(token);
        ++index_enumerator;
        token = strtok(NULL,corector);
        //char * tmp = strtok(NULL,corector);
        //token = strdup(tmp);
    }
    tokens[index_enumerator] =  NULL;
    return tokens;
}

void main_loop() {
    int status = 1;
    char* line;// = malloc(10*sizeof(char));
    char** args;
    do {
        line = read_line();
        //args = tokenize_a_line(line);
        args = tokenize_a_line(line);
        //args = line_parser(line);
        execute(args);
        free(line);
        free(args);
    }while (1);
}
int main(int argc, char* argv[]) {
    //signal(SIGINT,sigint_handler);
    signal(SIGTSTP,sigtstp_handler);
    main_loop();
    return 0;
}