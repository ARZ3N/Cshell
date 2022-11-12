/**
 *   =====================        Seashell CLI         =====================================================|
 * @brief:SeaShell is a custom shell program implemented using C                                            | 
 *                It supports 4 commands as of this moment                                                  |
 *                Works similar to the Linux Terminal                                                       |
 *                It uses Linux environment specific POSIX compliant libraries for system calls.            |
 * Supported Commands-                                                                                      |
 * -> cd : change directory                                                                                 |
 * -> cwd : print the Current Working Directory                                                             |
 * -> help                                                                                                  |
 * -> sayonara : exits the Cshell shell                                                                     |
 *                                                                                                          |
 * Each command is served by creating a Process using fork()                                                | 
 * After that, the Child Process runs an instance to call functions associated with the                     |
 * associated command.                                                                                      |
 *                                                                                                          |
 *       |  NOTE  |                                                                                         |
 * >> THIS PROGRAM WILL COMPILE IN A LINUX ENVIRONMENT ONLY!                                                |
 * @author: ANIRUDDHA BHATTACHARJEE (17BEE002)                                                              |
 * @date: 10-Dec-2020                                                                                       |
 *=========================================================================================================*/ 
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

#define SEASHELL_RL_BUFSIZE 1024
#define SEASHELL_TOK_BUFSIZE 64
#define SEASHELL_TOK_DELIM " \t\r\n\a"
/**
 * function Definitions
 **/ 
int seashell_execute(char **args);
char* seashell_read_line(void);
char** seashell_split_line(char *line);
int seashell_launch(char **args);
void seashell_loop(void);

/**
 * ==============================================================================================
 *               Functions declarations for Built-in seashell commands
 * ==============================================================================================
 */
int seashell_cd(char **args);
int seashell_help(char **args);
int seashell_cwd(char **args);
int seashell_sayonara(char **args);

/* Built-in commands List->
*/
char *builtin_str[] = {
    "cd",
    "help",
    "cwd",
    "sayonara"
};
int (*builtin_func[])(char **) = {
    &seashell_cd,
    &seashell_help,
    &seashell_cwd,
    &seashell_sayonara
};

int seashell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}
/**
 * Builtin function Definitions->
 */
int seashell_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, ">>>>SeaShell: expected argument to be \"cd\" \n");
    }
    else{
        if(chdir(args[1]) != 0){
            perror("SeaShell");
        }
    }
    return 1;
}
//

int seashell_help(char **args){
    int i;
    printf("\t\t\t\t-----------------------###############---------------------------- \n");
    printf("\t\t\t\t>> SeaShell <<: < HELP >");
    printf("\t\t\t\t\t These are the Built-in functions>> \n");
    for(i=0; i< seashell_num_builtins(); i++){
        printf("\t\t|: %s \n", builtin_str[i]);
    }
    return 1;
} 
//

int seashell_cwd(char **args){
    if(args[1] != NULL){
        fprintf(stderr, ">> SeaShell <<: ! Unexpected arguements following primary command >> cwd ! \n");
    }
    char *dir;
    char dirbuf[BUFSIZ];
    dir = getcwd(dirbuf, sizeof(dirbuf));
    printf("cwd @--->   %s \n",dirbuf);
    return 1;
}
int seashell_sayonara(char **args){
    //exits the main loop
    printf(" Closing SeaShell ... \n");
    return 0;
}
/**
 * ====================================================================================
 *                      End of Builtin function definitions
 * ====================================================================================
 */

int seashell_execute(char **args){
    int i;
    if(args[0] == NULL){
        //No command entered, continue the main loop
        return 1;
    }
    for(i=0; i < seashell_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return seashell_launch(args);
}
//
/**
 * ======================================================================================
 *                      INPUT PROCESSING FUNCTIONS (PARSING)
 *  seashell_read_line: reads input and converts it into C-string
 *  seashell_split_line: splits the C-string inout from cshell_read_line into individual string
 */ 
char* seashell_read_line(void){
    int position = 0;
    int bufsize = CSHELL_RL_BUFSIZE;
    char* buffer = malloc(sizeof(char)* bufsize);
    int c;
    if(!buffer){
        fprintf(stderr, ">> c-shell <<: ! malloc Error inside cshell_read_line(void) __ !\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        //read one character at a time
        c = getchar();
        //If EOF or '\n' encountered, replace it with '\0'
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            int i;
            printf("Command: \t");
            for(i=0;buffer[i] != '\0'; ++i){
                printf("%c", buffer[i]);
            }
            printf("\n");
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;
        //if the set buffer size of 1024 is breached, reallocate memory
        if(position >= bufsize){
            bufsize += SEASHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, ">> SeaShell <<: ! realloc Error inside __ seashell_read_line(void) __!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

//
char** seashell_split_line(char *line){
    int bufsize = SEASHELL_TOK_BUFSIZE;
    int position=0;
    char** tokens = malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens){
        fprintf(stderr, ">> SeaShell <<: ! Malloc Error inside __ seashell_split_line(char *) __!\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, SEASHELL_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += SEASHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, ">> SeaShell <<: ! realloc Error inside __ seashell_split_line(char*) __!\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SEASHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * ======================================================================================
 *          PROCESS CREATION FUNCTION- Cshell_launch()
 */ 
int seashell_launch(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();   //Create a process duplicate
    if(pid == 0){
        if(execvp(args[0], args) == -1){
            perror("SeaShell");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0){
        perror("SeaShell");
    }
    else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
 * ======= CONDITIONAL INFINITE LOOP FOR Cshell   ================
 */ 
void seashell_loop(void){
    char *line;
    char **args;
    int status;
    do{
        printf("> ");
        line = seashell_read_line();
        args = seashell_split_line(line);
        status = seashell_execute(args);

        free(line);
        free(args);

    }while(status);
}
//

int main(int argc, char** argv){
    printf("\t\t\t\t <<-- This is SeaShell -->> \n");
    /**
     * The duration of the Shell is the time this program spends inside 
     * the function- cshell_loop(), below.
     */ 
    seashell_loop();
    return EXIT_SUCCESS;
}
