/**
 *   =====================        Cshell CLI         =====================================================|
 * @brief:CSHELL is a custom shell program implemented using C                                            | 
 *                It supports 4 commands as of this moment                                                 |
 *                Works similar to the Linux Terminal                                                      |
 *                It uses Linux environment specific POSIX compliant libraries for system calls.           |
 * Supported Commands-                                                                                     |
 * -> cd : change directory                                                                                |
 * -> cwd : print the Current Working Directory                                                            |
 * -> help                                                                                                 |
 * -> sayonara : exits the Sheldon shell                                                                   |
 *                                                                                                         |
 * Each command is served by creating a Process using fork()                                               | 
 * After that, the Child Process runs an instance to call functions associated with the                    |
 * associated command.                                                                                     |
 *                                                                                                         |
 *       |  NOTE  |                                                                                        |
 * >> THIS PROGRAM WILL COMPILE IN A LINUX ENVIRONMENT ONLY!                                               |
 * @author: ANIRUDDHA BHATTACHARJEE (17BEE002)                                                             |
 * @date: 10-Dec-2020                                                                                      |
 *=========================================================================================================*/ 
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

#define CSHELL_RL_BUFSIZE 1024
#define CSHELL_TOK_BUFSIZE 64
#define CSHELL_TOK_DELIM " \t\r\n\a"
/**
 * function Definitions
 **/ 
int cshell_execute(char **args);
char* cshell_read_line(void);
char** cshell_split_line(char *line);
int cshell_launch(char **args);
void cshell_loop(void);

/**
 * ==============================================================================================
 *               Functions declarations for Built-in Cshell commands
 * ==============================================================================================
 */
int cshell_cd(char **args);
int cshell_help(char **args);
int cshell_cwd(char **args);
int cshell_sayonara(char **args);

/* Built-in commands List->
*/
char *builtin_str[] = {
    "cd",
    "help",
    "cwd",
    "sayonara"
};
int (*builtin_func[])(char **) = {
    &cshell_cd,
    &cshell_help,
    &cshell_cwd,
    &cshell_sayonara
};

int cshell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}
/**
 * Builtin function Definitions->
 */
int cshell_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, ">>>>c-shell: expected argument to be \"cd\" \n");
    }
    else{
        if(chdir(args[1]) != 0){
            perror("c-shell");
        }
    }
    return 1;
}
//

int cshell_help(char **args){
    int i;
    printf("\t\t\t\t--------------------------------------------------------------------- \n");
    printf("\t\t\t\t>>>>c-shell: < HELP >");
    printf("\t\t\t\t\t These are the Built-in functions>> \n");
    for(i=0; i< cshell_num_builtins(); i++){
        printf("\t\t|: %s \n", builtin_str[i]);
    }
    return 1;
} 
//

int cshell_cwd(char **args){
    if(args[1] != NULL){
        fprintf(stderr, ">>>>c-shell: Unexpected arguements following primary command >> cwd ! \n");
    }
    char *dir;
    char dirbuf[BUFSIZ];
    dir = getcwd(dirbuf, sizeof(dirbuf));
    printf("cwd @--->   %s \n",dirbuf);
    return 1;
}
int sheldon_sayonara(char **args){
    //exits the main loop
    printf(" Closing C-SHELL ... \n");
    return 0;
}
/**
 * ====================================================================================
 *                      End of Builtin function definitions
 * ====================================================================================
 */

int cshell_execute(char **args){
    int i;
    if(args[0] == NULL){
        //No command entered, continue the main loop
        return 1;
    }
    for(i=0; i < cshell_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return sheldon_launch(args);
}
//
/**
 * ======================================================================================
 *                      INPUT PROCESSING FUNCTIONS (PARSING)
 *  cshell_read_line: reads input and converts it into C-string
 *  cshell_split_line: splits the C-string inout from sheldon_read_line into individual string
 */ 
char* cshell_read_line(void){
    int position = 0;
    int bufsize = CSHELL_RL_BUFSIZE;
    char* buffer = malloc(sizeof(char)* bufsize);
    int c;
    if(!buffer){
        fprintf(stderr, ">>>>{c-shell}: ! ALLOCATION ERROR !\n");
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
            bufsize += CSHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, ">>>>c-shell: ! Failed to allocate additional memory!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

//
char** cshell_split_line(char *line){
    int bufsize = CSHELL_TOK_BUFSIZE;
    int position=0;
    char** tokens = malloc(sizeof(char*) * bufsize);
    char *token;
    if(!tokens){
        fprintf(stderr, ">>>>SHELDON: pal, we have an ALLOCATION ERROR here!\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, CSHELL_TOK_DELIM);
    while(token != NULL){
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += CSHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, ">>>>SHELDON: boy, we COULD NOT ALLOCATE MORE MEMORY!\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, CSHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
 * ======================================================================================
 *          PROCESS CREATION FUNCTION- Cshell_launch()
 */ 
int cshell_launch(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();   //Create a process duplicate
    if(pid == 0){
        if(execvp(args[0], args) == -1){
            perror("cshell");
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0){
        perror("cshell");
    }
    else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
 * ======= CONDITIONAL INFINITE LOOP FOR SHELDON   ================
 */ 
void cshell_loop(void){
    char *line;
    char **args;
    int status;
    do{
        printf("> ");
        line = cshell_read_line();
        args = cshell_split_line(line);
        status = cshell_execute(args);

        free(line);
        free(args);

    }while(status);
}
//

int main(int argc, char** argv){
    printf("\t\t\t\t <<-- This is C-SHELL -->> \n");
    /**
     * The duration of the Shell is the time this program spends inside 
     * the function- sheldon_loop(), below.
     */ 
    cshell_loop();
    return EXIT_SUCCESS;
}