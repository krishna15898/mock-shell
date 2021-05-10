#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define PATH_LEN 128
void print_history(char** inputs, int last, int hist_full);
void print_path();

char home_path[PATH_LEN];

void main()
{	
	char** inputs=calloc(5,sizeof(char*));
	int last=0;
	int hist_full = 0;
	getcwd(home_path,PATH_LEN);//save home path
	// printf("%s\n",home_path);

	while (1) {
		print_path();
		// take input command
		ssize_t size=0;
		char *input=NULL;
		getline(&input,&size,stdin);

		inputs[last]=input;//saving history
		if (last==4) hist_full=1;
		last=(last+1)%5;
		
		int input_len=strlen(input);
		input[input_len-1]='\0';//replacing '\n' by end of string
		
		// parse program and arguments
		char space[]=" ";
		strcat(input,space);
		char** words=malloc(sizeof(char*)*PATH_LEN);//pointer to array of words
		char* word=malloc(sizeof(char)*PATH_LEN);
		memset(words,0,sizeof(char*)*PATH_LEN);//setting initial to zero
		memset(word,0,sizeof(char)*PATH_LEN);
		int word_num=0;
		int word_ind=0;
		int found_quote=0;
		//iterating over all characters
		for (int i = 0; i < input_len; ++i)
		{
			if (input[i+1] && input[i]==' ' && input[i+1]==' ')//if double spaces are found
				continue;
			if (input[i]=='\"')//quotes found
				found_quote=1-found_quote;
			else if ((input[i]==' ' && !found_quote)) {//if space within quotes
				word[word_ind]='\0';
				words[word_num]=malloc(sizeof(char*)*PATH_LEN);
				memset(words[word_num],0,sizeof(char*)*PATH_LEN);
				char* empty="";
				strcpy(words[word_num],word);
				word=malloc(sizeof(char)*PATH_LEN);
				word_num++;
				word_ind=0;
			}
			else if ((input[i]=='~' && !found_quote) && (input[i+1]=='/' || input[i+1]==' ' || !input[i+1])) {
				strcpy(word,home_path);
				int home_path_len=strlen(home_path);
				word_ind=home_path_len;
			}else {
				word[word_ind]=input[i];
				word_ind++;
			}
		}
		//execute commands
		//check if history
		char* history="history";
		if (!strcmp(history,words[0])){
			print_history(inputs,last,hist_full);
		}
		else {
			//execute other commands
			pid_t pid,wpid;
			int status;
			pid=fork();//create child process to execute the command
			if (pid==0) {//child process
				char* cd="cd";
				//check if cd (builtin command) or "exec"able command
				if (!strcmp(cd,words[0]))
				{	
					if (words[2])
						perror("-bash too many arguments");
					else if (words[1]) {
						if (chdir(words[1])==-1) 
						{
							printf("%s:", words[1]);
							fflush(stdout);
							perror("");
						}
					}
					else {
						chdir(home_path);
					}
				}//if not, then exec able process
				else execvp(words[0],words);
			}
			else if (pid<0){
				perror("fork failed");
			}
			else
				do {
					wpid=waitpid(pid,&status,WUNTRACED);
				} while (!WIFEXITED(status) && !WIFSIGNALED(status));
		}
		free(word);
		free(words);
	}
}

void print_history(char** inputs, int last, int hist_full) {
	if (!hist_full)
		for (int i = 0; i < last; ++i) 
			printf("%s\n", inputs[i]);
	else
		for (int i = 0; i < 5; ++i)
			printf("%s\n", inputs[(last+i)%5]);
}

void print_path(){
	char path[PATH_LEN];//keeping track of path
	getcwd(path,PATH_LEN);
	int len1=strlen(path);
	int len2=strlen(home_path);
	if ((len1 >= len2) && (!strncmp(path,home_path,len2))) {
		printf("MTL458:~");
		for (int i = len2; i < len1; ++i)
			printf("%c", path[i]);
		printf("$");
	}
	else
		printf("MTL458:%s$", path);
}