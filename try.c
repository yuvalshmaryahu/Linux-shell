#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>


void handle_SIGCHLD(int signum)
{ /*SIGCHILD handler: wait for any child without hanging (WNOHANG flag) 
    repitetively untill there are no terminated children*/
    int *status=NULL;
    while(waitpid((pid_t)(-1), status, WNOHANG)>0){}
}

int prepare(void)
{
    /////////////////////////////////////
    ///////  SIGINT handling  ///////////
    /////////////////////////////////////
    //Intstall a signal handler for SIGINT which simply ignores:
    if(signal(SIGINT, SIG_IGN)==SIG_ERR)
    {
        fprintf(stderr, "%s",strerror(errno));
        exit(1);
    }
    /////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////


    /////////////////////////////////////
    ///////  SIGCHLD handling  ///////////
    /////////////////////////////////////
    //Define a sigaction struct instance with the handle_SIGCHLD func, empty mask and some flags:
    struct sigaction siga;
    siga.sa_handler=&handle_SIGCHLD;
    sigemptyset(&(siga.sa_mask));
    siga.sa_flags = SA_NOCLDSTOP | SA_RESTART; // the SA_NOCLDSTOP flag will prevent the handler from being 
                                               // invoked for stop/continue of a child process while we need
                                               // to handle the SIGCHLD onlt when child is terminated.
                                               // the SA_RESTART flag will cause a retry of function
                                               // execution if the function was interrupted by the signal

    //Intstall a signal handler for SIGINT which simply ignores:
    if(sigaction(SIGCHLD, &siga, NULL)<0)
    {
        fprintf(stderr, "%s",strerror(errno));
        exit(1);
    }
    //////////////////////////////////////////////////////////////

    return 0;
}

int process_arglist(int count, char **arglist)
{
    void handle_child_error()
    {// general function to be called when there's an error in a child process
        fprintf(stderr,"%s", strerror(errno));
        exit(1);
    }
    int bool=1; // flag which determines if the current command was executed already
    int status=0; // keep here child status after wait

    // Handle each case separatly:
    
    //redirecting:
    if(bool && count>=2 && strcmp(arglist[count-2], "<")==0)
    {
        bool=0; //command is executed
        int child_pid=fork();
        if(child_pid<0)
        {
            fprintf(stderr, "%s",strerror(errno));
            exit(1);
        }

        if(child_pid==0)
        { //child
            // change signal handler of SIGINT to default
            if(signal(SIGINT, SIG_DFL)==SIG_ERR)
            {
                fprintf(stderr, "%s",strerror(errno));
                exit(1);
            }

            arglist[count-2]=NULL; // update cmd arglist to end before the < char
            int fd = open(arglist[count-1], O_RDONLY);
            if(fd<0 || dup2(fd, STDIN_FILENO)<0 || close(fd)<0 || execvp(arglist[0], arglist))
                handle_child_error();
        }
        else
        {
            if(waitpid(child_pid, &status, 0)==-1)
            {
                if (errno!=ECHILD && errno!=EINTR)
                {
                    fprintf(stderr, "%s",strerror(errno));
                    return 0;
                }
            }
        }

    }

    int is_pipe(int count, char **arglist)
    { // find if pipe char is part of arglist and if so return its index, else return 0
        int i;
        for(i=1; i<count; i++)
        {
            if(strcmp(arglist[i], "|")==0)
            {
                return i;
            }
        }
        return 0;
    }

    //piping
    int splitter=is_pipe(count-1, arglist);
    if (bool && splitter)
    {
        bool=0; //command is executed
        int *fd=(int *)malloc(2*sizeof(int));
        if(pipe(fd)==-1)
        {
            fprintf(stderr, "%s",strerror(errno));
            exit(1);
        }
        arglist[splitter]=NULL; // put NULL instead of the pipe char (|)
        char **p1_arglist=arglist; //arglist for first child (end just before the pipe char)
        char **p2_arglist=arglist+splitter+1; //arglist for second child (start just after the pipe char)
        int pid1=fork();
        if(pid1<0)
        {
            fprintf(stderr, "%s",strerror(errno));
            exit(1);
        }
        if(pid1==0)
        {// first child
            if(signal(SIGINT, SIG_DFL)==SIG_ERR || dup2(fd[1], STDOUT_FILENO)<0 || close(fd[0])<0 || close(fd[1])<0 || execvp(p1_arglist[0], p1_arglist))
            {
                free(fd);
                handle_child_error();
            }
        }
        else
        {// parent
            int pid2=fork();
            if(pid2<0)
            {
                fprintf(stderr, "%s",strerror(errno));
                exit(1);
            }
            if(pid2==0)
            {// second child
                if(signal(SIGINT, SIG_DFL)==SIG_ERR || dup2(fd[0], STDIN_FILENO)<0 || close(fd[0])<0 || close(fd[1])<0 || execvp(p2_arglist[0], p2_arglist))
                {
                    handle_child_error();
                    free(fd);
                }
            }
            else
            { //parent
                close(fd[0]);
                close(fd[1]);
                free(fd);
                if(waitpid(pid1, &status, 0)<0)
                {
                    if (errno!=ECHILD && errno!=EINTR)
                    {
                        fprintf(stderr, "%s",strerror(errno));
                        return 0;
                    }
                }
                if(waitpid(pid2, &status, 0)<0)
                {
                    if (errno!=ECHILD && errno!=EINTR)
                    {
                        fprintf(stderr, "%s",strerror(errno));
                        return 0;
                    }
                }
            }
        }
    }

    //background
    if(bool && strcmp(arglist[count-1],"&")==0)
    {
        bool=0; //command is executed
        int pid=fork();
        if(pid<0)
        {
            fprintf(stderr, "%s",strerror(errno));
            exit(1);
        }
        if(pid==0)
        {// child
            arglist[count-1]=NULL; //update cmd arglist to end before the & char
            if (execvp(arglist[0], arglist)<0)
                handle_child_error();
        }
    }

    //default
    if (bool)
    {
        int pid=fork();
        if(pid<0)
        {
            fprintf(stderr, "%s",strerror(errno));
            exit(1);
        }
        if(pid==0)
        {//child
            
            if(signal(SIGINT, SIG_DFL)==SIG_ERR || execvp(arglist[0], arglist)<0)
                handle_child_error();
        }
        else
        {//parent
            if (waitpid(pid, &status, 0)<0)
            {
                if (errno!=ECHILD && errno!=EINTR)
                {
                    fprintf(stderr, "%s",strerror(errno));
                    return 0;
                }
            }
        }
    }
    return 1;
}


int finalize(void)
{
    return 0;
}
