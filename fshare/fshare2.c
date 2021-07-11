#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include "sys/types.h"

#include "regex.h"
void send_mes(int sock_fd,void* header, int len ){
	int  s=0;
	
	while (len > 0 && (s = send(sock_fd,header, len, 0)) > 0) {
		header += s ;
		len -= s ;
	}
}
int main(int argc, char* argv[]){
	//command line argumenti
	
	//regex
	regex_t state;
	const char* ip4_pattern = "^[0-9]+.[0-9]+.[0-9]+.[0-9]+:[0-9]+$";
	regcomp(&state,ip4_pattern,REG_EXTENDED);
//	printf("%s\n",argv[3]);	
	int status = regexec(&state,argv[1],0,NULL,0);

	if(status ==0){
	}
	else{
		printf("Ip or Port error\n");
		exit(1);
	}

	char * ip = strtok(argv[1],":");
	char * port =strtok(NULL,":");
	char* file_name;
	int port_num=0;
	printf("ip:%s  port: %s\n", ip, port);
	
	
	port_num = atoi(port);

	// connect socket
	
	struct sockaddr_in serv_addr;
	int sock_fd ;
	int s, len ;
	char buffer[1024] = {0};
	char * data ;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0) ;
	if (sock_fd <= 0) {
		perror("socket failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	memset(&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_num);
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ;
		exit(EXIT_FAILURE) ;
	}

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
	
	if(strcmp("list",argv[2])==0){
		int list_type =1;
		send_mes(sock_fd,&list_type,4);
	}
	if(strcmp("get",argv[2])==0){
		int list_type = 2;
		int file_name_len = strlen(argv[3])+1;
		send_mes(sock_fd, &list_type,4);
		send_mes(sock_fd, &file_name_len,4);
		
		send_mes(sock_fd, argv[3],file_name_len);
	}
	if(strcmp("put",argv[2])==0){
	}
	
	

	shutdown(sock_fd, SHUT_WR) ;

	char buf[1024] ;
	data = 0x0 ;
	len = 0 ;
	while ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
		if (data == 0x0) {
			data = strdup(buf) ;
			len = s ;
		}
		else {
			data = realloc(data, len + s + 1) ;
			strncpy(data + len, buf, s) ;
			data[len + s] = 0x0 ;
			len += s ;
		}

	}
	printf("%s",data);

}
