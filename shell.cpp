#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <sys/wait.h>
#include <fcntl.h>


//Comment added

char iFile[10], oFile[10]; //Files for IO redirections

using namespace std;
/*
 * Struct for storing command data
 */
typedef struct comm{

	char* com[10]; //commands
	char tCommand[40];
	int argc; //no of arguments
	bool iRed, oRed, ooRed; //Redirection flags

} cmd;

cmd* input; // list of commands



bool isBuiltin(){

	char * command = input[0].com[0];


	if( strcmp(command, "cd") == 0)
		return true;
	else if(strcmp(command, "echo") == 0)
		return true;
	else
		return false;
}


void execBuiltin(){

	char * command = input[0].com[0];

	if( strcmp(command, "cd") == 0){

		chdir(input[0].com[1]);
	}

	if( strcmp(command, "echo") == 0){

		//cout<<" Argument count: "<<input[0].argc<<endl;
		for(int i=1; i<input[0].argc; i++){

			printf("%s ",input[0].com[i]);
		}
		printf("\n");
	}

}




void piping(int argc){

	//std::cout<<"Inside piping"<<std::endl;
	int i, stat;
	pid_t pid ;
	for(i=0; i<argc-1; i++){
		//std::cout<<"Inside Loop"<<std::endl;
		int pFd[2];

		pipe(pFd);

		pid = fork();

		if( pid == 0){

			dup2( pFd[1],1);
			execvp(input[i].com[0], input[i].com);
		}

		dup2(pFd[0], 0);
		close(pFd[1]);

	}

	// do IO Redirection
	if( input[i].iRed ){

		int fd = open(iFile, O_RDONLY , 0644);
		dup2(fd,0);

	}

	if( input[i].oRed ){

		int fd = open(oFile, O_WRONLY | O_CREAT,0644 );

		dup2(fd, 1);


	}

	if( input[i].ooRed ){

		int fd = open(oFile, O_WRONLY | O_APPEND | O_CREAT, 0644 );
		dup2(fd,1);

	}

	execvp(input[i].com[0], input[i].com);


}







void parseCmd(int cmdNo){


	for(int j=0; j< cmdNo; j++){

		char *token;
		/* get the first token */
		token = strtok(input[j].tCommand, " ");
		int i=0;
		   /* walk through other tokens */
		while( token != NULL ) {


			if( strcmp(token, "<") == 0){

				input[j].iRed = true;
				token = strtok(NULL, " ");
				strcpy(iFile, token );
			}

			else if( strcmp(token, ">") == 0 ){

				input[j].oRed = true;
				token = strtok(NULL, " ");
				strcpy(oFile, token );

			}
			else if( strcmp(token, ">>") == 0 ){
				input[j].ooRed = true;
				token = strtok(NULL, " ");
				strcpy(oFile, token );

			}
			else{
				input[j].oRed = false;
				input[j].ooRed = false;
				input[j].iRed = false;

				input[j].com[i] = token;
				i++;

			}
			token = strtok(NULL, " ");
		}
		input[j].com[i] = NULL;
		input[j].argc = i;
	}


}


int parser( char* command){

	char *token;
	/* get the first token */
	token = strtok(command, "|");
	int cmdNo = 0;
	   /* walk through other tokens */
	while( token != NULL ) {

		strcpy( input[cmdNo].tCommand, token);

		token = strtok(NULL, "|");
		cmdNo++;
	}

	return cmdNo;


}


void preprocessCmd(char* command){

	char temp[100];
	strcpy(temp, command);

	int i=0;
	int j=0;
	while(temp[i] != '\0'){

		if(temp[i] == '<' || temp[i] == '>'){

			command[j++] = ' ';
			command[j++] = temp[i];
			command[j] = ' ';
		}
		else
			command[j] = temp[i];
		j++;
		i++;
	}

	command[j] = '\0';
	//std::cout<<"Command is: "<<command<<std::endl;

}


int main(){

	char  inputstr[100];
	char cwd[1024];
	int stat;
	getcwd(cwd, sizeof(cwd));
	printf("%s$",cwd);

	fgets(inputstr, 100, stdin);

	size_t ln = strlen(inputstr) - 1;
	if (inputstr[ln] == '\n')
		inputstr[ln] = '\0';

	while( strcmp(inputstr, "exit") != 0)
	{

		preprocessCmd(inputstr);


		input = new cmd[10];

		int ccount = parser(inputstr);
		parseCmd(ccount);



		if ( isBuiltin()){
			execBuiltin();

		}
		else{


			pid_t pid = fork();
			if( pid == 0)
				piping(ccount);
			else
				waitpid(pid, &stat, 0);
		}


		getcwd(cwd, sizeof(cwd));
		printf("%s$", cwd);

		fgets(inputstr, 100, stdin);
		ln = strlen(inputstr) - 1;
		if (inputstr[ln] == '\n')
			inputstr[ln] = '\0';

		delete[] input;

	}


}
