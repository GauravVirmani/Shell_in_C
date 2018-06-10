/*
Guarav Virmani - 2015A3PS0175P
Tushar Tripathi - 2015A3PS0235P
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#define STYLE_BOLD         "\033[1m"
#define STYLE_NO_BOLD 	"\033[22m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

char inputBuffer[1000];
char *command[1000];
int alternate;
char *args[100];
char *iredirf;
char *oredirf;

void cdCommand()
{
	struct passwd *pw = getpwuid(getuid());
	const char *h1 = pw->pw_dir;
	char h[1000];
	strcpy(h, h1);
	if(args[1]==NULL)
	        chdir(h);
	if(args[1][strlen(args[1]-1)]=='/')
		args[1][strlen(args[1]-1)] = '\0';
	if (args[1][0]== '~') {
		args[1][0]='/';
	    chdir(h);
	}
	else {
		chdir(args[1]);
	}
}
void display() {
	char cwd[1000];
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	printf(STYLE_BOLD ANSI_COLOR_GREEN "%s@customShell" ANSI_COLOR_RESET STYLE_NO_BOLD, pw->pw_name);
	printf(STYLE_BOLD ANSI_COLOR_BLUE ":%s" ANSI_COLOR_RESET STYLE_NO_BOLD, getcwd(cwd, sizeof(cwd)));
	printf(STYLE_BOLD "$ "STYLE_NO_BOLD);
}

void splitWith(char *com_exec, char *pat) {
	char *com_exec1;
	com_exec1 = strdup(com_exec);
	int m = 1;
	args[0] = strtok(com_exec1, pat);
	while((args[m]=strtok(NULL, pat))!=NULL) ++m;
	args[m] = NULL;
	// return m;
}

void findFiles(char *command, int i, int o)
{
  char *tkn[100];
  char *tmp;
  tmp=strdup(command);
  int m=1;
  if(i!=0)
	  tkn[0]=strtok(tmp,"<");
  else
	  tkn[0]=strtok(tmp,">");
  if(o!=0)
	  while((tkn[m]=strtok(NULL,">"))!=NULL)
	        m++;
  else
  	while((tkn[m]=strtok(NULL,"<"))!=NULL)
	        m++;
	if(o==1 && i==1) {
		while (*(tkn[2])==' ') ++(tkn[2]);
  		oredirf=strdup(tkn[2]);
	  	while (*(tkn[1])==' ') ++(tkn[1]);
		iredirf=strdup(tkn[1]);
  	}
	else if(i==1) {
		while (*(tkn[1])==' ') ++(tkn[1]);
		iredirf=strdup(tkn[1]);
	}
	else {
		while (*(tkn[1])==' ') ++(tkn[1]);
		oredirf=strdup(tkn[1]);
	}
	splitWith(tkn[0], " ");
}

int executeSingle(int alternate, int first, int last, char *command) {
	int fd[2];
	pipe(fd);
	int append = 0;
	int pid = fork();

	if(pid==0) {
        if (first==1 && last==0 && alternate==0)
        {
        	dup2( fd[1], 1 );
        }
        else if (first==0 && last==0 && alternate!=0)
        {
         	dup2(fd[1], 1);
        	dup2(alternate, 0);
        }
        else
        {
        	dup2(alternate, 0);
        }
		int iFd, oFd;
        if(strchr(command, '<') && strchr(command, '>')) {
        	if (strstr(command, ">>")!=NULL)
        	{
        		append = 1;
        	}
        	findFiles(command, 1, 1);

            if(append==1) {
            	oFd= open(oredirf, O_WRONLY | O_APPEND | O_CREAT, 0644);
            }
            else
            	oFd= open(oredirf, O_WRONLY | O_CREAT, 0644);

            dup2(oFd, 1);
            close(oFd);

        	iFd=open(iredirf,O_RDONLY, 0);

			dup2(iFd, 0);
			close(iFd);
        }
        else if(strchr(command, '<')) {

        	findFiles(command, 1, 0);

        	iFd=open(iredirf,O_RDONLY, 0);
			dup2(iFd, 0);
			close(iFd);
        }
        else if(strchr(command, '>')) {
        	if (strstr(command, ">>")!=NULL)
        	{
        		append = 1;
        	}
        	findFiles(command, 0, 1);

			if(append==1) {
            	oFd= open(oredirf, O_WRONLY | O_APPEND | O_CREAT, 0644);
            }
            else
            	oFd= open(oredirf, O_WRONLY | O_CREAT, 0644);
            dup2(oFd, 1);
            close(oFd);
        }
        execvp(args[0], args);
        exit(0);
    }
    else
    {
	    int status;
	    int childPid = waitpid(pid, &status, 0);
	    // printf("child with pid = %d exited with status = %d\n", childPid, status);
	    close(fd[1]);
	    if (alternate != 0)
	        close(alternate);
	    if (last == 1)
	        close(fd[0]);
	    return fd[0];
    }
}

void seperate() {
	if(strlen(inputBuffer)>3&&strchr(inputBuffer, '|')==0 && strchr(inputBuffer, '>')==0 && strchr(inputBuffer, '<')==0 && inputBuffer[0]=='c' && inputBuffer[1]=='d' && inputBuffer[2]==' ') {
    	 	splitWith(inputBuffer, " ");
		    cdCommand();
	}
    else {
    	int alternate = 0;

		int n = 1;
		command[0] = strtok(inputBuffer, "|");
		while((command[n]=strtok(NULL, "|"))!=NULL)
			++n;
	    command[n]=NULL;

		for(int i=0; i<n; ++i) {
				splitWith(command[i], " ");

				if(args[0]!=NULL) {
			        if (strcmp(args[0], "exit") == 0)
			            exit(0);
				}
				alternate = executeSingle(alternate, (i==0), (i==n-1), command[i]);
		}
		alternate = 0;
	}
}

int main(int argc, char const *argv[])
{
	while(1) {
        display();
        fgets(inputBuffer, sizeof(inputBuffer), stdin);
        if(strcmp(inputBuffer, "\n")==0||strcmp(inputBuffer, "\n")==0||strcmp(inputBuffer, "\r\n")==0)
            continue;
        int len = strlen(inputBuffer);
        inputBuffer[len-1] = '\0';
        if(strcmp(inputBuffer, "exit")==0) {
            exit(0);
        }
		if(strstr(inputBuffer, "|||") != NULL) {
	    	int n = 1;
			command[0] = strtok(inputBuffer, "|");
			while((command[n]=strtok(NULL, "|"))!=NULL)
				++n;
			char *token[1000];
		    char *tmp;
			tmp = strdup(command[1]);
			int m=1;
			token[0] = strtok(tmp, ",");
			while((token[m]=strtok(NULL, ","))!=NULL) ++m;
    		strcpy(inputBuffer, command[0]);
			strcat(inputBuffer, " | ");
			strcat(inputBuffer, token[0]);
			seperate();
    		strcpy(inputBuffer, command[0]);
			strcat(inputBuffer, " | ");
			strcat(inputBuffer, token[1]);
			seperate();
			strcpy(inputBuffer, command[0]);
			strcat(inputBuffer, " | ");
			strcat(inputBuffer, token[2]);
			seperate();
	    }
    	else if(strstr(inputBuffer, "||") != NULL) {
	    	int n = 1;
			command[0] = strtok(inputBuffer, "|");
			while((command[n]=strtok(NULL, "|"))!=NULL)
				++n;
			char *token[1000];
		    char *tmp;
			tmp = strdup(command[1]);
			int m=1;
			token[0] = strtok(tmp, ",");
			while((token[m]=strtok(NULL, ","))!=NULL) ++m;
    		strcpy(inputBuffer, command[0]);
			strcat(inputBuffer, " | ");
			strcat(inputBuffer, token[0]);
			seperate();
    		strcpy(inputBuffer, command[0]);
			strcat(inputBuffer, " | ");
			strcat(inputBuffer, token[1]);
			seperate();
    	}
		else {
			seperate();
    	}
	}
	return 0;
}
