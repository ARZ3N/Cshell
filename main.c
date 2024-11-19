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
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

/* MACROS Definitions */
#define SHELL_RL_BUFSIZE 1000
#define SHELL_TOK_BUFSIZE 100
#define SHELL_TOK_DELIM " \t\a"

/* Global Variable Declarations */
pid_t pid, wpid;
int status;
int hard_error_flag = 0;
int eof_terminate = 0;

/* Function Definitions */ 
int seashell_execute(char **args);
char* seashell_read_line(void);
char** seashell_split_line(char *line);
int quote_split_line(const char *line, char single_quotes[][SHELL_RL_BUFSIZE], char double_quotes[][SHELL_RL_BUFSIZE], int *single_count, int *double_count);
int seashell_launch(char **args);
void seashell_loop(void);

/* Functions declarations for Built-in/implemented commands */
int seashell_cmd_cd(char **args);
int seashell_cmd_help(char **args);
int seashell_cmd_pwd(char **args);
int seashell_cmd_exit(char **args);

/* Built-in commands List */
char *builtin_str[] = {
    "cd",
    "help",
    "pwd",
    "exit"
};
//

int (*builtin_func[])(char **) = {
    &seashell_cmd_cd,
    &seashell_cmd_help,
    &seashell_cmd_pwd,
    &seashell_cmd_exit
};
//

int seashell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}
//

/* Builtin function Definitions */
int seashell_cmd_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, "error: expected argument to be \"cd\" \n");
    }
    else{
        if(chdir(args[1]) != 0){
            perror("cd");
        }
    }
    return 1;
}
//

int seashell_cmd_help(char **args){
    int i;
    printf("\t ==== SHELL < help > ====\n");
    printf("\t ----------------------\n");
    printf("\tBuilt-in functions with custom implementation >> \n");
    for(i=0; i< seashell_num_builtins(); i++){
        printf("\t|: %s \n", builtin_str[i]);
    }
    return 1;
} 
//

int seashell_cmd_pwd(char **args){
    if(args[1] != NULL){
        fprintf(stderr, "error: Unexpected arguements following primary command >> pwd \n");
    }
    char *dir;
    char dirbuf[BUFSIZ];
    dir = getcwd(dirbuf, sizeof(dirbuf));
    printf("%s \n",dirbuf);
    return 1;
}
//

int seashell_cmd_exit(char **args){
    //exits the main loop
    //printf(" Shell Closed \n");
    return 0;
}
//

/* SHELL function to initiate command executions */
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

/** ================================================================
 *          SHELL INPUT PARSING FUNCTIONS
 * =================================================================
 */

/*Function to read raw input*/
char* seashell_read_line(void){
    int position = 0;
    int bufsize = SHELL_RL_BUFSIZE;
    char* buffer = malloc(sizeof(char)* bufsize);
    int c;
    if(!buffer){
        fprintf(stderr, "error: ! malloc Error inside seashell_read_line(void) [Unable to allocate buffer memory] !\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        
        c = getchar();  // read a character from input
        //If EOF or '\n' encountered, replace it with '\0'
        if(c == EOF || c == '\n'){
            if(c == EOF){
                //set shell to exit if EOF encountered
                eof_terminate = 1;
            }
            eof_terminate = 0;
            buffer[position] = '\0';
            int i;
            /*
            printf("Command: \t");
            for(i=0;buffer[i] != '\0'; ++i){
                printf("%c", buffer[i]);
            }
            printf("\n");
            */
            int cmd_start = 0, cmd_end = 0;
            int it = 0;
            while(buffer[it] == ' '){
                it = it + 1;
            }
            cmd_start = it;
            
            it = position-1;
            while(buffer[it] == ' '){
                it = it - 1;
            }
            cmd_end = it;
            //printf("start: %d, end: %d \n", cmd_start, cmd_end);
            char *clean_buffer = malloc(sizeof(char) * bufsize);
            int jt = 0;
            for(it = cmd_start; it<= cmd_end; ++it){
                clean_buffer[jt] = buffer[it];
                jt = jt + 1;
            }
            //printf("cl_buf: %s \n", clean_buffer);
            clean_buffer[it+1] = '\0';
            return clean_buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;
        //if the set buffer size of 1004 is breached, reallocate memory
        if(position >= bufsize){
            bufsize += SHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "error: realloc Error inside seashell_read_line(void) [Unable to alloc more memory for buffer]!\n");
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "warning: User Input character length is more than 1000 characters[!] \n");
        }
    }
}
//

/* Function to split line input */
char** seashell_split_line(char *line){
    int bufsize = SHELL_TOK_BUFSIZE;
    int position=0;
    char** words = malloc(sizeof(char*) * bufsize);
    char *token;
    char *temp_line = malloc(sizeof(char) * SHELL_RL_BUFSIZE);
    int it, word_count = 0;

    for(it = 0; line[it] != '\0'; ++it){
        temp_line[it] = line[it];
    }
    temp_line[it] = '\0';

    // TO SEPARATE LINE WITH SPACE DELIMITER
    if(!words){
        fprintf(stderr, "error: Malloc Error inside shell_split_line(char *) [Unable to alloc memory for words]\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(temp_line, SHELL_TOK_DELIM);
    while(token != NULL){
        words[position] = token;
        position++;
        if(position >= bufsize){
            bufsize += SHELL_TOK_BUFSIZE;
            words = realloc(words, bufsize * sizeof(char*));
            if(!words){
                fprintf(stderr, "error: realloc Error inside shell_split_line(char*) [Unable to alloc more memory for words]\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    words[position] = NULL;
    
    //printf("All separated words: \n");
    for(it = 0; words[it] != NULL; ++it){
        //printf("words[%d]:%s. \n", it, words[it]);
    }
    word_count = it;
    //printf("word count: %d \n", word_count);

    // TO SEPARATE LINE WITH QUOTE DELIMITERS ========
    
    int in_sig_quotes = 0, in_dbl_quotes = 0;
    char *start = NULL;
    int single_count = 0, double_count = 0;
    char single_quotes[SHELL_TOK_BUFSIZE / 2][SHELL_RL_BUFSIZE];
    char double_quotes[SHELL_TOK_BUFSIZE / 2][SHELL_RL_BUFSIZE];

    if(double_quotes == NULL || single_quotes == NULL){
        fprintf(stderr, "error: Unable to allocate memory for quote strings \n");
        return NULL;
    }

    if(quote_split_line(line, single_quotes, double_quotes, &single_count, &double_count) == 0){
        //printf("single count: %d, double_count: %d \n", single_count, double_count);
        char **qwords = malloc(sizeof(char*) * bufsize);
        int qpos = 0;
        qwords[0] = words[0];
        qpos++;
        for(it=0; it<double_count; ++it){
            qwords[qpos] = double_quotes[it];
            //printf("qwords[%d]: %s\n", qpos, qwords[qpos]);
            qpos++;
        }
        int sig_pos = qpos;
        for(it=0; it<single_count; ++it){
            qwords[qpos] = single_quotes[it];
            //printf("qwords[%d]: %s\n", qpos, qwords[qpos]);
            qpos++;
        }
        // check different possiblities for quotes and space separations
        if(word_count-1 == (single_count + double_count)){
            //printf("entered mode 1 \n");
            return qwords;
        }
        else if(single_count > 0 && double_count > 0){
            //printf("Entered mode 1 \n");
            return qwords;
        }
        else if((single_count > 0 && double_count == 0) && word_count - 1 >= single_count){
            //printf("Entered mode 2 \n");
            for(it=0; line[it] != '\0'; ++it){
                if(line[it] == '\''){
                    break;
                }
            }
            int first_quote = it;
            int words_fs = 0, words_sd = 0;
            char **temp = malloc(sizeof(char*) * bufsize);
            for(it=0; words[it] != NULL; ++it){
                int jt;
                char *ptr = words[it];
                for(jt = 0; ptr[jt] != '\0'; ++jt){
                    if(ptr[jt] == '\''){
                        if(words_fs == 0){
                            words_fs = it;
                        }
                        else{
                            words_sd = it;
                        }
                    }
                    if(words_sd != 0){
                        break;
                    }
                }
                if(words_sd != 0){
                    break;
                }
            }
            if(words_fs == words_sd){
                return words;
            }
            else{
                for(it=0; it<words_fs; ++it){
                    temp[it] = words[it];
                }
                int temp_pos = words_fs;
                for(it=sig_pos; it<=single_count; ++it){
                    char *cat_str = malloc(sizeof(char) * (3+strlen(qwords[it])));
                    cat_str[0] = '\0';
                    
                    //strcat(cat_str, "\'\0");
                    
                    strcat(cat_str, qwords[it]);
                    
                    //strcat(cat_str, "\'\0");
                    strcat(cat_str, "\0");
                    temp[temp_pos] = cat_str;
                    temp_pos++;
                }
                // print temp
                /*
                for(it=0; temp[it] != NULL; ++it){
                    printf("temp[%d]: %s\n", it, temp[it]);
                }
                */
                return temp;
            }
        }
        else if((double_count > 0 && single_count == 0) && word_count-1 >= double_count){
            //printf("entered mode 3 \n");
            for(it=0; line[it] != '\0'; ++it){
                if(line[it] == '\"'){
                    break;
                }
            }
            int first_quote = it;
            int words_fs = 0, words_sd = 0;
            char **temp = malloc(sizeof(char*) * bufsize);
            for(it=0; words[it] != NULL; ++it){
                int jt;
                char *ptr = words[it];
                for(jt = 0; ptr[jt] != '\0'; ++jt){
                    if(ptr[jt] == '\"'){
                        if(words_fs == 0){
                            words_fs = it;
                        }
                        else{
                            words_sd = it;
                        }
                    }
                    if(words_sd != 0){
                        break;
                    }
                }
                if(words_sd != 0){
                    break;
                }
            }
            if(words_fs == words_sd){
                return words;
            }
            else{
                for(it=0; it<words_fs; ++it){
                    temp[it] = words[it];
                }
                int temp_pos = words_fs;
                for(it=1; it<=double_count; ++it){
                    char *cat_str = malloc(sizeof(char) * (3+strlen(qwords[it])));
                    cat_str[0] = '\0';
                    
                    //strcat(cat_str, "\'\0");
                    
                    strcat(cat_str, qwords[it]);
                    
                    //strcat(cat_str, "\'\0");
                    strcat(cat_str, "\0");
                    temp[temp_pos] = cat_str;
                    temp_pos++;
                }
                // print temp
                /*
                for(it=0; temp[it] != NULL; ++it){
                    printf("temp[%d]: %s\n", it, temp[it]);
                }
                */
                return temp;
            }
        }
        
        return words;
    }
    else{
        return words;
    }
}
//

/* to split line with quotes as delimiters */
int quote_split_line(const char *line, char single_quotes[SHELL_TOK_BUFSIZE/2][SHELL_RL_BUFSIZE], char double_quotes[SHELL_TOK_BUFSIZE/2][SHELL_RL_BUFSIZE], int *single_count, int *double_count) {
    int in_single_quotes = 0;
    int in_double_quotes = 0;
    const char *start = NULL;

    *single_count = 0;
    *double_count = 0;

    for (const char *ptr = line; *ptr != '\0'; ++ptr) {
        if (*ptr == '\'' && !in_double_quotes) {
            if (in_single_quotes) {
                int len = ptr - start;
                if (*single_count <= (SHELL_TOK_BUFSIZE / 2) && len < SHELL_RL_BUFSIZE) {
                    strncpy(single_quotes[*single_count], start, len);
                    single_quotes[*single_count][len] = '\0';
                    (*single_count)++;
                }
                in_single_quotes = 0;
            } else {
                in_single_quotes = 1;
                start = ptr + 1; // Start pos
            }
        } else if (*ptr == '"' && !in_single_quotes) {
            if (in_double_quotes) {
                int len = ptr - start;
                if (*double_count < (SHELL_TOK_BUFSIZE / 2) && len < SHELL_RL_BUFSIZE) {
                    strncpy(double_quotes[*double_count], start, len);
                    double_quotes[*double_count][len] = '\0';
                    (*double_count)++;
                }
                in_double_quotes = 0;
            } else {
                in_double_quotes = 1;
                start = ptr + 1; // start pos
            }
        }
    }

    if (in_single_quotes || in_double_quotes) {
        fprintf(stderr, "error: Mismatched quotes \n");
        hard_error_flag = 1;
        return -1;
    }
    else{
        hard_error_flag = 0;
        return 0;
    }
}
//

/** ===========================================================
 *   SHELL PROCESS CREATION AND EXECUTION HANDLING FUNCTIONS
 *  ===========================================================
 */

int seashell_launch(char **args){
    
    pid = fork();
    if(pid == 0){
        // child process
        if(execvp(args[0], args) == -1){
            perror("execvp");
        }
        if(eof_terminate == 1){
            seashell_cmd_exit(args);
        }
        exit(EXIT_FAILURE);
    }
    else if(pid < 0){
        perror("fork");
    }
    else{
        //parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if(exit_code != 0){
                    fprintf(stderr, "error: command exited with code %d\n", exit_code);
                }
            }
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));

        //printf("[*] wpid status: %d\n", status);
    }
    return 1;
}
//

/* infinite loop for shell */ 
void seashell_loop(void){
    printf("\t\t <<-- SHELL Start -->> \n");
    char *line;
    char **args;
    int status;
    char caller_exit;
    do{
        //fprintf(stderr, "> ");
        fprintf(stderr, "$ ");
        line = seashell_read_line();
        args = seashell_split_line(line);
        if(hard_error_flag){
            continue;
        }
        else{
            status = seashell_execute(args);
        } 
        
        free(line);
        free(args);
        fflush(stdin);

    }while(status);
}
//

/** ===================================================================
 *            MAIN FUNCTION, ENTRY AND EXIT POINTS
 * ====================================================================
 */
int main(int argc, char** argv){
    char *open_choice = NULL;
    ssize_t len;
    int input_stream_len;
    int flag = 1;
 
    Enter_Shell_Loop:
        seashell_loop();
    
    printf("------------< SEASHELL Closed (exit) >--------------\n");
    printf("==== Press 'S' to OPEN SeaShell again ....\n");
    printf("==== Press 'E' to EXIT PROGRAM ....\n");
    do{
        
        input_stream_len = getline(&open_choice, &len, stdin);
        
        if((open_choice[0] == 'S' || open_choice[0] == 's') && input_stream_len == 2){
            goto Enter_Shell_Loop;
        }
        else if((open_choice[0] == 'E' || open_choice[0] == 'e') && input_stream_len == 2){
            flag = 0;
        }
        else{
            printf("error: Invalid Input, 'S': OPEN, 'E': EXIT \n");
        }
    }while(flag);
    return EXIT_SUCCESS;
}
//
