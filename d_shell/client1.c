#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include<unistd.h>
#include<signal.h>
#include<stdbool.h>
#define DEBUG 1

int recv_fixed_len(int sock_fd, int len, void* buf){
        buf = (char*)buf;
	char *tmp = buf;
        int s;
        while(len>0 && (s = recv(sock_fd,tmp,len,0))>0){
#if DEBUG==0
	printf("DEBUG in FUNCTION: %s\n",tmp);	
#endif
		tmp+=s;
                len-=s;
        }
	return s;
}
int ipv4_con(int port){
	struct sockaddr_in serv_addr; 
	int sock_fd ;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	} 

	memset(&serv_addr, '\0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(port); 
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
	return sock_fd;
}
int dim_row(char* given_cmd){
	int c =0;
	char * tmp, *slice;
	tmp = (char*)malloc(strlen(given_cmd));
	strcpy(tmp,given_cmd);
	slice = strtok(tmp," ");
	while(slice!=NULL){
		c++;
		slice = strtok(NULL," ");
	}
	return c;
}
void string_to_2dim(char** cmd, char* buffer, int row){
	char * tmp;
       tmp = strtok(buffer, " ");	
	for(int i=0; i<row; i++){
	 	
#if DEBUG==0
		printf("DEBUG[strtok] %s \n",tmp);
#endif
		cmd[i] = (char*)malloc(strlen(tmp));
		strcpy(cmd[i],tmp);
		tmp = strtok(NULL," ");
	 }

}
bool bang(char *buffer, int conn, int pid){
	if(strcmp(buffer,"bang")==0){
		printf("bang!\n");
		kill(pid,SIGINT);
		return true;
	}
	return false;
}
void close_socket(int s, int conn){
	if(s==0){
		close(conn);
		exit(1);
	}
}
int 
main(int argc, char const *argv[]) 
{
        bool end=false;	
	char ** cmd;
	int row=0, pid;
	int port = atoi(argv[1]);
	int s, len ;
	char* buffer;
	int fd[2];
	int sock_fd = ipv4_con(port);
	while(1){
		printf("start");
		s = recv_fixed_len(sock_fd,4,&len);
		close_socket(s,sock_fd);	
		buffer = (char*)malloc(len);
		recv_fixed_len(sock_fd, len, buffer);

		printf("%s",buffer);

		bang(buffer, sock_fd, pid);
		row = dim_row(buffer);
#if DEBUG==0
		printf("DEBUG: %s  row :%d\n",buffer,row);
#endif
		cmd = (char**)malloc(sizeof(char*)*row);
		 string_to_2dim(cmd,buffer,row);	
#if DEBUG==0
 		 for(int i=0; i<row; i++)
			printf("[DEBUG] ListCheck %s\n", cmd[i]);
#endif
 		pid = fork();
		if(pid==0){

			dup2(sock_fd,1);
			dup2(sock_fd,2);
			execvp(cmd[0],cmd);	
		}
	}	

		shutdown(sock_fd, SHUT_WR) ;
	printf("END!\n");		
} 
