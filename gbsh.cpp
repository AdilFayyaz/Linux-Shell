#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <bits/stdc++.h>  
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define WHITE "\x1b[0m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define clearscr() printf("\e[1;1H\e[2J")
using namespace std;
void Output_Redirection(char * );
void Input_Redirection(char * );
int stdout_val,stdin_val;
int background=0;
/*
	Signal Function to handle Cntl + C  
*/
void SIGINTHANDLER(int signalvalue){
	if(fork()){
		wait(NULL);
	}
	else{
	signal(SIGINT, SIGINTHANDLER);
		exit(0);	
	}
}
/*
	Tokenization Function on the delimiters passed into function
*/
int tokens(string user_query, char ** query, char * delimiters,int counter){
	char * token;
	char c_user_query[1024];
	strcpy(c_user_query,user_query.c_str());
	token=strtok(c_user_query,delimiters);
	int token_length=strlen(token);
	
	while(token!=NULL){
		query[counter]=new char[token_length];
		strcpy(query[counter],token);
		int diff_out,diff_in;
		diff_out=dup(1); diff_in= dup(0);
		dup2(stdout_val,1);
		dup2(stdin_val,0);
		printf("%s\n",query[counter]);
		dup2(diff_out,1);
		dup2(diff_in,0);
		token=strtok(NULL,delimiters);
		++counter;
	}
	query[counter]=NULL;
	
	return counter;
}
/*
	Forking + Exec function
*/
void call_via_exec(char ** query, char * working_directory){
	int s1=1; int xx;
	pid_t pid=fork();
	if(pid>0){
		if(background==-1){
		//	cout<<"bg was -1\n";
			while((xx=wait(&s1))>0);
		}
		char parent_path[1024]="";
		char gbsh[8]="/gbsh";
		strcat(parent_path,working_directory);
		strcat(parent_path,gbsh);
		int status=setenv("parent",parent_path,0);
		if(!status){
			const char* s = getenv("parent");
		} 
	}
	else{
		execvp(query[0],query);
		printf("Error in Exec\n" );
		exit(s1);
	}
	dup2(stdout_val,1);
	dup2(stdin_val,0);
}
/*
	Function used to insert spaces in the users search query
*/
char * Add_Space_Delimiter(string user_query){
	char * c_user_query = new char [user_query.length()*2];
	int x=0;
	for(int i=0;i<user_query.length();++i){
		if(user_query[i]=='|' || user_query[i] == '>' || user_query[i] == '<'){
			if(i!=0 && user_query[i-1]!=' ')
			{	c_user_query[x]=' '; ++x;	}
			c_user_query[x]=user_query[i];++x;
			if(i!=user_query.length()-1 && user_query[i+1]!=' ')
			{	c_user_query[x]=' '; ++x;   }
		}
		else{
			c_user_query[x]=user_query[i]; ++x;
		}
	}
	c_user_query[x]='\0';
	return c_user_query;
}
/*
	Redirect the Output using DUP2
*/
void Output_Redirection(char * out){
	int output= open(out,O_WRONLY | O_TRUNC | O_CREAT,0664);
	dup2(output,1); close(output);
}
/*
	Redirect the Input using DUP2
*/
void Input_Redirection(char * in){
	int input=open(in,O_RDONLY,0666);
	dup2(input,0); close(input);
}
/*
	Function to allow piping of instructions in the user query
*/
void Piping(char ** query,char * wd){
	int file_desc[2]; pipe(file_desc);
	pid_t pid=fork();
	if(pid==0){
		close(file_desc[0]);
		dup2(file_desc[1],1);
		call_via_exec(query,wd);
		exit(0);
	}
	else{
		wait(NULL);
		close(file_desc[1]);
		dup2(file_desc[0],0);
	}	
	return;
}
/*
	Function to find redirections in the input query and call the above functions respectively
*/
void check_redirections(char * c_user_query, bool flag, char * working_directory,char ** query, string user_query ,int wait,int can_pipe){
	int par_count;
	if((strstr(c_user_query,"<") || strstr(c_user_query,">") || strstr(c_user_query,"|"))&& flag){
		char * app_query = Add_Space_Delimiter(user_query);
		char *new_user_query[1024];
		char less[]="<"; char greater[]=">"; char pipee[]="|";
		int x=0;
		char * tok=strtok(app_query," ");
		while(tok != NULL){
			if(strcmp(tok,greater)==0){
				char * temp_out=strtok(NULL," ");
				Output_Redirection(temp_out);
			}
			else if(strcmp(tok,less)==0){
				char * temp_in=strtok(NULL," ");
				Input_Redirection(temp_in);	
			}
			else if(strcmp(tok,pipee)==0){
				new_user_query[x]=NULL;
				Piping(new_user_query,working_directory);x=0;
			}
			else{
				new_user_query[x]=tok; ++x;
			}
			tok=strtok(NULL," ");
		}
		new_user_query[x]=NULL;
		if(!wait and !can_pipe)
			call_via_exec(new_user_query,working_directory);

	}
	else if(flag){
		char delim[]=" ";
		par_count=tokens(user_query,&*query,delim,0);
		call_via_exec(query,working_directory);
	}
}

int main(int argc, char *argv[]) {
	stdout_val=dup(1); stdin_val=dup(0);
	char ** query;
	query= new char *[1024];
	while(true){
		string user_query;
		char host_name[1024];
		gethostname(host_name,sizeof(host_name));
		int status=1; int enter=0;
		int nq=0; int bg;
		char user[1024];
		getlogin_r(user,sizeof(user));
		char working_directory[1024];
		getcwd(working_directory,sizeof(working_directory));
		fprintf(stdout, GREEN "<%s>@<%s>",user,host_name);
		fprintf(stdout, BLUE "<%s> > " MAGENTA,working_directory);
		signal(SIGINT,SIGINTHANDLER);
		getline(cin,user_query);
		char flag=1;
		if(user_query==""){enter=1;}
		/*
			Determining whether the given query should run as a background process or not
		*/
		background=user_query.find("&"); 
		if(background!=-1){
			for(bg=0;bg<user_query.length();++bg){}
			user_query[bg-1]='\0';
		}
		char shell_path[1024]="SHELL=";
		char gbsh[8]="/gbsh";
		strcat(shell_path,working_directory);
		putenv(strcat(shell_path,gbsh));
		char c_user_query[1024];
		strcpy(c_user_query,user_query.c_str());
		int par_count; char delimiters[4]=" |,";
		if(!enter){
			//The Exit Command
			if(strstr(c_user_query,"exit")){
				flag=0;
				return EXIT_SUCCESS;
			}
			/*
				Implementing the pwd command
			*/
			else if(strstr(c_user_query,"pwd")){
				pid_t pid=fork();
				if(pid==0){
					if(strstr(c_user_query,"|")){
						check_redirections(c_user_query,flag,working_directory,query,user_query,0,0);
						exit(0);
					}
					if(strstr(c_user_query,">")){
						int found_ls=user_query.find("pwd") +3;
						int uq_length=user_query.length();
						user_query=user_query.substr(found_ls,uq_length);
						strcpy(c_user_query,user_query.c_str());
						check_redirections(c_user_query,flag,working_directory,query,user_query,1,1);
						flag=0;
					}
					char * app_query = Add_Space_Delimiter(user_query);
					char * tok=strtok(app_query," ");
					char greater[]=">"; flag=0;
					int x=0;
					while(tok != NULL){
						if(strcmp(tok,greater)==0){
							char * temp_out=strtok(NULL," ");
							Output_Redirection(temp_out);
						}
						tok=strtok(NULL," ");
					}

					printf("%s\n",working_directory);
					exit(0);					
				}
				else{
					wait(NULL); flag=0;
				}
			}
			/*
				Implementing the clear command
			*/
			else if(strstr(c_user_query,"clear")){
				flag=0;
				clearscr();
			}
			/*
				Implementing the ls command
			*/
			else if(strstr(c_user_query,"ls")){
				//Output redirection for ls command
				pid_t pid=fork();
				if(pid==0){
					par_count=tokens(user_query,&*query,delimiters,0);
					struct dirent *dirStr;
		  			DIR *directory;
		  			if(par_count==1){
						directory=opendir(working_directory);
		  			}
		  			else{
		  				int file_status=access(query[1],F_OK);
		  				if(file_status){
		  					directory=opendir(working_directory);
		  				}
		  				else{
		  					directory=opendir(query[1]);
		  				}
		  			}
					if(!directory){
				   		printf("Directory didn't open");
				   	}
					if(strstr(c_user_query,"|")){
						check_redirections(c_user_query,flag,working_directory,query,user_query,0,0);
						exit(0);
					}
					if(strstr(c_user_query,">")){
						int found_ls=user_query.find("ls") +2;
						
						int uq_length=user_query.length();
						user_query=user_query.substr(found_ls,uq_length);
					
						strcpy(c_user_query,user_query.c_str());
						check_redirections(c_user_query,flag,working_directory,query,user_query,1,1);
						
						flag=0;
					}

				   	int ls_count=0;
				   	while(dirStr=readdir(directory)){
				   		string temp_file_name=dirStr->d_name;
				   		if(temp_file_name != ".." && temp_file_name!="."){
				   			if(ls_count==0){
				   				if(strstr(c_user_query,">")){
				   					printf("%s  ",dirStr->d_name);	
				   				}
				   				else
				   					printf(RED "%s  " WHITE,dirStr->d_name);
				   			}
				   			else{
				   				printf("%s  ",dirStr->d_name);
				   			}
					   			 ++ls_count;
				   		}
				   	}
				   	printf("\n"); 
				   	exit(0);
				}
				else{
					wait(NULL); flag=0;
				}
			} 
			/*
				Implementing the cd command
			*/
			else if(strstr(c_user_query,"cd")){
				flag=0;
				par_count=tokens(user_query,&*query,delimiters,0);
				int file_status=access(query[1],F_OK);
				//File Exists if Status returned is = 0
				if(!file_status){
					chdir(query[1]);
					char new_wd[1024];
			    	getcwd(new_wd,sizeof(new_wd));
				}
			}
			/*
				Implementing the environ command
			*/
			else if(strstr(c_user_query,"environ")){
				pid_t pid=fork();
				if(pid==0){
					if(strstr(c_user_query,"|")){
						check_redirections(c_user_query,flag,working_directory,query,user_query,0,0);
						exit(0);
					}
					if(strstr(c_user_query,">")){
						int found_ls=user_query.find("environ") +7;
						int uq_length=user_query.length();
						user_query=user_query.substr(found_ls,uq_length);
						strcpy(c_user_query,user_query.c_str());
						check_redirections(c_user_query,flag,working_directory,query,user_query,1,1);
						flag=0;
					}
					par_count=tokens(user_query,&*query,delimiters,0);
					extern char ** environ;
					int x=0;
					while(environ[x]){
						printf("%s\n",environ[x]);++x;	
					}
					exit(0);
				}
				else{
					wait(NULL); flag=0;
				}
			}
			/*
				Implementing the unsetenv command
			*/
			else if(strstr(c_user_query,"unsetenv") && c_user_query != "setenv"){
				flag=0;
				par_count=tokens(user_query,&*query,delimiters,0);
				const char * genv=query[1];
				char * already_exists = getenv(genv);
				if(already_exists){
					int unset_value=unsetenv(query[1]);
				}
				else{
					printf("The environment variable does not exist\n");
				}
			}
			/*
				Implementing the setenv command
			*/
			else if(strstr(c_user_query,"setenv")){
				flag=0;
				par_count=tokens(user_query,&*query,delimiters,0);
				const char * genv=query[1];
				char * already_exists = getenv(genv);
				if(!already_exists){
					if(par_count==3){
						int status=setenv(query[1],query[2],0);
					}
					else if(par_count==2){
						int stauts=setenv(query[1],"",0);
					}
				}
				else{
					printf("The environment variable already exists\n");
				}
			}
			/*
				Implementing the getenv command
			*/
			else if(strstr(c_user_query,"getenv")){
				flag=0;
				par_count=tokens(user_query,&*query,delimiters,0);
				const char* get_env=query[1];
				char * ret_path;
				ret_path=getenv(get_env);
				if(ret_path){
					printf("%s\n",ret_path); 
				}
			}
			/*
				Otherwise, call the redirections function which will cater for other commands as well...
			*/
			check_redirections(c_user_query,flag,working_directory,query,user_query,0,0);
			
		}
	}
}

