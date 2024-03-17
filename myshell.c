#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

void execute(char** args);
int built_in_functions(char** args);
void sigint_handler();
void sigtstp_handler();
void child_handler();
char* read_line();
char** tokenize_a_line(char* line);
void ampersant_service(char** args);
int did_ampersant_occured(char** args);
int get_ampersant_position(char** args);
int get_array_size(char** args);
void execute_with_ampersant(char** args);
int look_for_redirection(char** args, int if_wait);
void main_loop();

pid_t current_foreground_pid;

void execute(char** args) {
    if(built_in_functions(args) == 1) {
        return ;
    }
    if(did_ampersant_occured(args)) {
        execute_with_ampersant(args);
        return;
    }
    if(look_for_redirection(args,1)) {
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

void execute_with_ampersant(char** args) {
    char** left_args;
    char** right_args;
    int ampersant_position = get_ampersant_position(args);
    int array_size = get_array_size(args) - ampersant_position;
    left_args = malloc((ampersant_position+1)*sizeof(char*));
    right_args = malloc((array_size-ampersant_position+2)*sizeof(char*));
    for(int i = 0 ; i < ampersant_position ; ++i) {
        left_args[i] = args[i];
    }
    left_args[ampersant_position] = NULL;
    for(int i = 1 ; i < array_size - ampersant_position + 2 ; ++i) {
        right_args[i-1] = args[ampersant_position+i];
    }
    right_args[array_size - ampersant_position+1] = NULL;
    int status;
    if(look_for_redirection(left_args,0) == 0) {
        pid_t pid = fork();
        if(pid == 0) {
            execvp(left_args[0],left_args);
        }
        else if (pid < 0) {
            perror("error");
        }
    }
    if(right_args[0]!=NULL) {
        execute(right_args);
    }
    return;
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

void child_handler() {
    int status;
    pid_t pid;
    while(pid = waitpid(0, &status, WNOHANG | WUNTRACED)>0) {
        if(WIFEXITED(status)) {
            //printf("die out. \n");
        }
        else if(WIFSIGNALED(status)) {
            //printf("killed. \n");
        }
        else if(WIFSTOPPED(status)) {
            //printf("stopped. \n");
        }
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
    int index_enumerator = 0;
    int space_enumerator = 0;
    while(line[index_enumerator] != '\n') {
        ++index_enumerator;
        if(line[index_enumerator] == ' ') {
            ++space_enumerator;
        }
    }
    int number_of_tokens = space_enumerator + 1;
    char** tokens = malloc((number_of_tokens)*sizeof(char*));
    char* token;
    char corector[] = " \t\r\n\a";
    index_enumerator= 0;
    token = strtok(line,corector);
    while(token!= NULL) {
        tokens[index_enumerator] = strdup(token);
        ++index_enumerator;
        token = strtok(NULL,corector);
    }
    tokens[index_enumerator] =  NULL;
    return tokens;
}

int look_for_redirection(char** args,int if_wait) {
    int result = 0 ;
    int fd;
    int status;
    char* file;
    pid_t pid;
    int array_size = get_array_size(args);
    if(array_size < 3) {
        return result;
    }
    if(strcmp(args[array_size - 2],">") == 0) {
        file = args[array_size-1];
        args[array_size-1] = NULL;
        args[array_size-2] = NULL;
        if(file!=NULL) {
            pid = fork();
            if (pid == 0) {
                fd = open(file,O_RDWR | O_CREAT,0777);
                dup2(fd,1);
                execvp(args[0],args);
                close(fd);
            }
            result = 1;
        }

    }
    else if(strcmp(args[array_size-2],"<") == 0) {
        file = args[array_size-1];
        args[array_size-1] = NULL;
        args[array_size-2] = NULL;
        if(file!=NULL) {
            pid = fork();
            if(pid == 0) {
                fd = open(file,O_RDWR | O_CREAT,0777);
                dup2(fd,0);
                execvp(args[0],args);
                close(fd);
            }
            result = 1;
        }

    }
    else if(strcmp(args[array_size - 2],"2>") == 0) {
        file = args[array_size-1];
        args[array_size-1] = NULL;
        args[array_size-2] = NULL;
        if(file!=NULL) {
            pid = fork();
            if(pid == 0) {
                fd = open(file,O_RDWR | O_CREAT,0777);
                dup2(fd,2);
                execvp(args[0],args);
                close(fd);
            }
            result = 1;
        }
    }
    if(result == 1 && if_wait == 1) {
        waitpid(pid,&status,WUNTRACED);
    }
    return result;
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
    }while (1);
}
int main(int argc, char* argv[]) {
    signal(SIGINT,sigint_handler);
    signal(SIGCHLD,child_handler);
    signal(SIGTSTP,sigtstp_handler);
    main_loop();
    return 0;
}