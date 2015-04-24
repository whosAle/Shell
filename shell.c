#include "shell.h"

pid_t child = 0; //global child variable
int background = 0; //global background flag

int execute(char *argv[]){ //maybe not use argv?? incorporate into main??
    int fd[10][2];		//holds the file descriptors for a pipe
    int starts[10];	//keep track of positions in argv[] from which we will run execvp
    char *inputFile = NULL;
    char *outputFile = NULL;
    int count =0;		//keep track of how many commands we will be running. 
				//Count starts at 0 meaning tha there are no pipes in the line. 
				//Commands are going to be one less than the count.
    int numPipes = 0;
    starts[0] = 0;
    
   if(0 == strcmp(argv[0], "exit")){return -1;} //checks to see if the user typed exit
    
    //preprocessing step -- looping through argument vector
    //handles 6 possible scenarios
    // a < b
    // a < b | c
    // a < b > c	
    // a < b | c > d	
    // a | b > c
    // a > b
    int i = 0;
    int round = 0;		    //TESTING ONLY
    while(argv[i] != NULL){
	if(strcmp(argv[i],"<")==0){
	    inputFile = argv[i+1];   //set inputFile to the next token
	    argv[i] = NULL;         //set the "<" token to NULL to stop exec from reading further
	    i = i + 2;              //incrememnt the counter to look at the token after the input
	    //file
	    if(argv[i] == NULL){
		break;

	    }else if(strcmp(argv[i],"|")== 0){
		pipe(fd[numPipes++]);
		argv[i] = NULL;
		i++;
		starts[count++] = i;
	    }else if(strcmp(argv[i],">")==0){
		argv[i] = NULL;
		i++;
		outputFile = argv[i];

	    }
	}else if(strcmp(argv[i],">")==0){
	    argv[i] = NULL;
	    i++;
	    outputFile = argv[i];
	}else if(strcmp(argv[i],"|")==0){
	    pipe(fd[numPipes++]);
	    argv[i] = NULL;
	    i++;
	    starts[count++] = i;
	}else{
	    i++;
	}
	round++;
    }

    //loop through start array to fork and exec
    for(i = 0; i <= count; i++){
        child = fork();
	if (child == -1) {
		perror("the fork has failed");
	} else if (child > 0){ //is a parent, decide to wait if background flag is checked 
	    int status;
	    //close the descriptors you would be using if it is not stdin or stdout if != stdin_fileno / stdout
	   // if( stdin != STDIN_FILENO || stdout != STDOUT_FILENO){
		closeFD(fd, count);
	 //   }
	    if(background != 1 && i == count) { // background flag is not checked, thus; wait for it to finis
		waitpid(child, &status, 0);
	    }
	    child = 0;
	} else { 	// is the child, runs program
	    if(i == 0 && inputFile != NULL){ //checks to see if first command is a redirection for input
		int rfi = open(inputFile, O_RDONLY, 0);
		if(rfi == -1){
		    perror("file redirection failed");
		}else{
		    dup2(rfi, STDIN_FILENO);
		    close(rfi);

		}
	    }
	    if(i == count && outputFile != NULL){ //checks to see if last command is a redirection for output
		int rfo = creat(outputFile, 0644);
		if(rfo == -1){
		    perror("file redirection failed");
		}else{
		    dup2(rfo, STDOUT_FILENO);
		    close(rfo);
		}
	    }

	    if(count > 0){ //there is a pipe sequence
		//if first command
		
		//printf("i = %d\n",i);
		if(i == 0){
		    dup2(fd[i][1],STDOUT_FILENO);
		}
		//if last command
		else if(i == count){
		    dup2(fd[i-1][0],STDIN_FILENO);
		}
		//otherwise we are looking at a command in the 
		//middle of the pipe sequence
		else{
		    dup2(fd[i][1],STDOUT_FILENO);
		    dup2(fd[i-1][0],STDIN_FILENO);
		}	    
	    }	
	    closeFD(fd, count);
	    execvp(argv[starts[i]],argv+starts[i]);
	    perror("exec failed");
	    exit(1);	
	}
    }
    return 11;
}

// helper function to close all file descriptors
void closeFD(int fd[][2], int count){
    for(int i = 0; i < count; i++) {
	close(fd[i][0]);
	close(fd[i][1]);
    }

}


//checks to see if SIGINT is executed through the command line and handles accordingly
void sig_handler(int sig){ //SOMEHOW GET PID
    signal(sig, sig_handler);

    if( child > 0) {
	kill(child, SIGINT);
    }

}

//Parses a command line with whitespace into a vector or strings
char** parse(char *line){
	int i;

	/*loops through the line checking for space characters and the ampersand character.
	 * Removes additional space and ampersand*/
	for(i = strlen(line)-2; i > 0; i--){
	    if(!isspace(line[i])){
		if(line[i] == '&'){
		    background = 1;
		}else{
		    line[i+1] = '\0';
		    break;
		}
	    }
	}

        char **args = malloc(sizeof(char*)*101);
       
       	memset(args, 0, 101*sizeof(char*));    //set all bytes of args to 0/NULL
        char **buff = &line;
	
	/* for error handling
	int foundPipe = 0;
	int foundOutput = 0; */

        char *token = strsep(buff," ");
        int counter = 0;
        while(token != NULL){
	    args[counter++] = token;
	    buff = &line;
	    token = strsep(buff," ");
	   /* 
	    //error handling
	    if(strcmp(token,"|") == 0){
		foundPipe = 1;
	    }
	    if(strcmp(token,">") == 0){
		foundOutput = 1;
	    }

	    //check for illegal order of pipe or redirect commands
	    if(foundPipe == 1 && strcmp(token,"<") == 0){
		perror("Incorrect Syntax");
		exit(EXIT_FAILURE);
	    }
	    if(foundOutput == 1 && strcmp(token,"|") == 0){

		perror("Incorrect Syntax");
		exit(EXIT_FAILURE);
	    }
	    */



	}

        return args;
}

int main() {
    signal(SIGINT, sig_handler);
    while(1){
	char *buff = malloc((sizeof(char)*1024));
	
	//reap children
	pid_t oldchild;
	int status;
	while((oldchild = waitpid(-1, &status, WNOHANG)) > 0){
	    printf("Reaped child: %d\n", oldchild);
	}

	//display prompt
	printf("%s",">");

	char *line = fgets(buff,1024,stdin);

	char **tokens = parse(line);

	//need to check length of argumenrs for this it has to be == 1
	if(execute(tokens) == -1){exit(EXIT_SUCCESS);} //use whatevr line variable

/*
	//testing if tokens work
    	int counter = 0;
	while(tokens[counter] != NULL){
	    printf("%s",tokens[counter++]);
	}printf("\n");
*/	
	//freeing stuff
	free(tokens);
    }

}

