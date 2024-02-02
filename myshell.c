#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

// arglist - a list of char* arguments (words) provided by the user
// it contains count+1 items, where the last item (arglist[count]) and *only* the last is NULL
// RETURNS - 1 if should continue, 0 otherwise

// prepare and finalize calls for initialization and destruction of anything required

void sigint_handler(int signo);
int prepare(void);
int finalize(void);
int process_arglist(int count, char** arglist);
int input_redirection(int index,char** arglist);
int output_redirection(int index,char** arglist);
int normal_running(char** arglist);
int background_running(int count , char** arglist);
int custom_pipe(int index,char** arglist);

void sigint_handler(int signo) {
    // Do nothing
}

int prepare(void){
    struct sigaction newAction = {.sa_handler = sigint_handler};
    if (sigaction(SIGINT, &newAction, NULL) == -1) {
        perror("Prepare not good");
        return 1;
    }
    return 0;

}

int finalize(void){
    return 0;
}

int process_arglist(int count, char** arglist){
    int i;
    int index = 0;
    for (i=0 ; i<count ; i++){
        if (arglist[i] == "|"){
            index = i;
            return custom_pipe(index,arglist);
        }
        else if (arglist[i] == "&"){
            return background_running(count,arglist);
        }
        else if (arglist[i] == "<"){
            index = i;
            return input_redirection(index,arglist);
        }
        else if (arglist[i] == ">"){
            index = i;
            return output_redirection(index,arglist);
        }
    }
    return normal_running(arglist);
}

int input_redirection(int index,char** arglist){
    arglist[index] = NULL;
    int fd = open(arglist[index+1], O_RDONLY);
    if (fd == -1){
        perror("file");
        return 0;
    }
    pid_t pid = fork();
    if (pid == -1){
        perror(fork);
        return 0;
    }
    else if (pid == 0){
        struct sigaction defaultAction = {
                .sa_handler = SIG_DFL, // Set the default signal handler
                .sa_flags = 0 // No special flags
        };
        if (0 != sigaction(SIGINT, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        if (0 != sigaction(SIGCHLD, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        if (dup2(fd,0) == -1){
            perror("dup2");
            exit(1);
        }
        close(fd);
        execvp(arglist[0],arglist);
        perror("execvp");
        exit(1);
    }
    else {
        if (waitpid(pid,NULL,0) == -1){
            perror("wait");
            return 0;
        }
        else if (errno != ECHILD && errno != EINTR ){
            perror("SIGNAL");
            return 0;
        }
    }
    return 1;
}

int output_redirection(int index,char** arglist){
    pid_t pid ;
    arglist[index] = NULL;
    int fd = openopen(arglist[index+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("file");
        return 0;
    }
    pid = fork();
    if (pid == -1){
        perror("fork");
        return 0;
    }
    else if (pid == 0){
        struct sigaction defaultAction = {
                .sa_handler = SIG_DFL, // Set the default signal handler
                .sa_flags = 0 // No special flags
        };
        if (0 != sigaction(SIGINT, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        if (0 != sigaction(SIGCHLD, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }

        if (dup2(fd,1) == -1){
            perror("dup2");
            exit(1);
        }
        close(fd);
        execvp(arglist[0],arglist);
        perror("execvp");
        exit(1);
    }
    else {
        if (waitpid(pid,NULL,0) == -1){
            perror("wait");
            return 0;
        }
        else if (errno != ECHILD && errno != EINTR ){
            perror("SIGNAL");
            return 0;
        }
    }
    return 1;
}

int normal_running(char** arglist){
    pid_t pid = fork();
    if (pid == -1){
        perror("fork");
        return 0;
    }
    if (pid == 0){
        struct sigaction defaultAction = {
                .sa_handler = SIG_DFL, // Set the default signal handler
                .sa_flags = 0 // No special flags
        };
        if (0 != sigaction(SIGINT, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        if (0 != sigaction(SIGCHLD, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        execvp(arglist[0],arglist);
        perror("execvp");
        exit(1);
    }
    else{
        if (waitpid(pid,NULL,0) == -1){
            perror("wait");
            return 0;
        }
        else if (errno != ECHILD && errno != EINTR ){
            perror("SIGNAL");
            return 0;
        }
    }
    return 1;
}

int background_running(int count , char** arglist){
    pid_t pid = fork();
    if (pid == -1) {
        perror(fork);
        return 0;
    }
    else if (pid == 0){
        struct sigaction defaultAction = {
                .sa_handler = SIG_DFL, // Set the default signal handler
                .sa_flags = 0 // No special flags
        };
        if (0 != sigaction(SIGINT, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        arglist[count-1] = NULL;
        execvp(arglist[0],arglist);
        perror("execvp");
        exit(1);
    }
    return 1;
}


int custom_pipe(int index,char** arglist){
    arglist[index] = NULL;
    int pipe_fd[2];

    if (-1 == pipe(pipefd)) {
        perror("pipe");
        return 0;
    }

    pid_t first_pid = fork();

    if (first_pid == -1){
        perror("fork");
        return 0;
    }

    if (first_pid == 0){
        //writer(parent) process
        struct sigaction defaultAction = {
                .sa_handler = SIG_DFL, // Set the default signal handler
                .sa_flags = 0 // No special flags
        };
        if (0 != sigaction(SIGINT, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        if (0 != sigaction(SIGCHLD, &defaultAction, NULL)) {
            perror("SIGNAL");
            exit(1);
        }
        close(pipe_fd[0]);
        if (dup2(pipe_fd[1],1) == -1){
            perror("dup2");
            exit(1);
        }
        close(pipe_fd[1]);
        execvp(arglist[0],arglist);
        perror("execvp");
        exit(1);
    }
    else {
        pid_t second_pid = fork();

        if (second_pid == -1){
            perror("fork");
            exit(1);
        }
        //reader(child) process
        if (second_pid == 0){
            struct sigaction defaultAction = {
                    .sa_handler = SIG_DFL, // Set the default signal handler
                    .sa_flags = 0 // No special flags
            };
            if (0 != sigaction(SIGINT, &defaultAction, NULL)) {
                perror("SIGNAL");
                exit(1);
            }
            if (0 != sigaction(SIGCHLD, &defaultAction, NULL)) {
                perror("SIGNAL");
                exit(1);
            }
            close(pipe_fd[1]);
            if (dup2(pipe_fd[0], 0) == -1){
                perror("dup2");
                exit(1);
            }
            close(pipe_fd[0]);
            execvp(arglist[index + 1], &arglist[index + 1]);
            perror("execvp");
            exit(1);

        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }
    if (wait() == -1){
        perror("wait");
        return 0;
    }
    else if (errno != ECHILD && errno != EINTR ){
        perror("SIGNAL");
        return 0;
    }
    return 1;

}

int main(void)
{
    if (prepare() != 0)
        exit(1);

    while (1)
    {
        char** arglist = NULL;
        char* line = NULL;
        size_t size;
        int count = 0;
        if (getline(&line, &size, stdin) == -1) {
            free(line);
            break;
        }

        arglist = (char**) malloc(sizeof(char*));
        if (arglist == NULL) {
            printf("malloc failed: %s\n", strerror(errno));
            exit(1);
        }
        arglist[0] = strtok(line, " \t\n");

        while (arglist[count] != NULL) {
            ++count;
            arglist = (char**) realloc(arglist, sizeof(char*) * (count + 1));
            if (arglist == NULL) {
                printf("realloc failed: %s\n", strerror(errno));
                exit(1);
            }

            arglist[count] = strtok(NULL, " \t\n");
        }

        if (count != 0) {
            if (!process_arglist(count, arglist)) {
                free(line);
                free(arglist);
                break;
            }
        }

        free(line);
        free(arglist);
    }

    if (finalize() != 0)
        exit(1);

    return 0;
}
