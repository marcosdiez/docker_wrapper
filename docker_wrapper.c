/* by Marcos Diez <marcos AT unitron DOT com DOT br>  on 2016-02-19 */

char *help = "usage:   docker_wrapper [-v] DOCKER_EXECUTABLE [OPTIONS] run [arg...] --name CONTAINER_NAME [arg...]\n\n"
             "example: docker_wrapper -v docker run --rm -t -i --name CONTAINER_NAME ubuntu /bin/bash\n\n"

             "This program launches a docker container (or actually any other command) and waits while it is executed.\n"
             "If it receives SIGTERM, it will run [DOCKER_EXECUTABLE stop CONTAINER_NAME].\n"
             "If it receives further SIGTERMs, it will run [DOCKER_EXECUTABLE kill CONTAINER_NAME].\n\n"
             "It's purpose is to be a wrapper for docker so it works as expected on supervisorctl, upstart, systemd and similar tools.\n\n"
             "It is compiled statically so you can copy it's executable and it should work anywhere on the same plataform."
             ;

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <errno.h>

#define NAME_PARAMETER "--name"

#define EXIT_ERROR_NOT_ENOUGH_PARAMETERS  1
#define EXIT_ERROR_MISSING_CONTAINER_NAME 2
#define EXIT_ERROR_CANT_CATCH_SIGNAL      3
#define EXIT_ERROR_RUNNING_FORK           4
#define EXIT_ERROR_EXEC                   5


int verbose = 0;
int initial_argv = 1;

char *container_name = NULL;
char *docker_executable = NULL;

int printf_verbose(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int return_value = 0;
    if(verbose){
        return_value = vprintf(format, args);
    }
    va_end(args);
    return return_value;
}


char *get_container_name(int argc, char** argv){
    for(int i = initial_argv; i < argc -1; i++){
        if(!strcmp(NAME_PARAMETER, argv[i])){ // no point in using strncmp here
            return argv[i+1];
        }
    }
    printf("Error: missing parameters [%s CONTAINER NAME]\n", NAME_PARAMETER);
    exit(EXIT_ERROR_MISSING_CONTAINER_NAME);
    return NULL;
}

int first_time = 1;

void sig_handler(int signo)
{
  if (signo == SIGTERM){
    char *docker_kill_command = first_time ? "stop" : "kill";
    first_time = 0;
    char *parameters[] = { docker_executable, docker_kill_command, container_name, (char *) NULL};
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("Error running fork\n");
        exit(EXIT_ERROR_RUNNING_FORK);
    }
    else if (pid > 0)
    {
        printf_verbose("Received SIGTERM. Running [%s %s %s]\n", parameters[0], parameters[1], parameters[2]);
    }
    else
    {
        // we are the child
        execvp(docker_executable, parameters);
        exit(EXIT_ERROR_EXEC);   // exec never returns
    }
  }
}

void parse_parameters(int argc, char **argv){
    if(argc < 4){
        puts(help);
        exit(EXIT_ERROR_NOT_ENOUGH_PARAMETERS);
    }
    if(!strcmp("-v", argv[1])){
        verbose = 1;
        initial_argv++;
    }
    docker_executable = argv[initial_argv];
    container_name = get_container_name(argc, argv);
}

void run_command(int argc, char **argv){
    printf_verbose("Launching container with name [%s]. Type [kill %d] to [docker stop] the container. Type again to [docker kill] it.\n", container_name, getpid());
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("Error running fork\n");
        exit(EXIT_ERROR_RUNNING_FORK);
    }
    else if (pid > 0)
    {
        int status;

        waitpid(pid, &status, 0);

        int exit_status = WEXITSTATUS(status);
        printf_verbose("Docker returned with exit status: [%d]\n", exit_status);
        exit(exit_status);
    }
    else
    {
        // we are the child
        execvp(argv[initial_argv], argv + initial_argv );
        // if we are here exec failed and returned -1
        int error = errno; // errno is volatile
        printf("Error [%d] executing [%s]: %s\n", error, argv[initial_argv], strerror(errno));
        exit(error);   // exec never returns
    }
}

int main(int argc, char **argv){
    parse_parameters(argc, argv);
    if (signal(SIGTERM, sig_handler) == SIG_ERR){
        printf("\nerror: can't catch SIGTERM\n");
        return EXIT_ERROR_CANT_CATCH_SIGNAL;
    }
    run_command(argc, argv);
    return 0;
}
