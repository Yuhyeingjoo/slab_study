#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h> 
#include<pthread.h>
#include <netinet/tcp.h>
#include<fcntl.h>
void *receive_print(void* fd){
	char buf[1024];
	char * data;
	int len, s;
	int *sock_fd_p = (int*)fd;
	int sock_fd = *sock_fd_p;
	while(1){
		data = 0x0 ;
		len = 0 ;
		if ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
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
		else{
			//printf("watong\n");
		}

		if(data!=0x0)		
		printf(">%s\n", data);
	}
}
void *input_send(void* fd){

	int* sock_fd_p = (int*)fd;
	int sock_fd =*sock_fd_p;
	char buffer [1024]	= {0};
	char *data;
	while(1){
		int s, len;
		scanf("%s", buffer) ;
	
		data = buffer ;
		len = strlen(buffer) ;
		s = 0 ;
		if (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
			data += s ;
			len -= s ;
		}

	}
}
	int 
main(int argc, char const *argv[]) 
{ 
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
	serv_addr.sin_port = htons(8095); 
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("inet_pton failed : ") ; 
		exit(EXIT_FAILURE) ;
	} 

	if (connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect failed : ") ;
		exit(EXIT_FAILURE) ;
	}
//	
	int opt_val = 1;

	setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, sizeof(opt_val));
//	fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL) | O_NONBLOCK);
//






//	setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, sizeof(opt_val));
	/*
	while(1){

		scanf("%s", buffer) ;
	
		data = buffer ;
		len = strlen(buffer) ;
		s = 0 ;
		if (len > 0 && (s = send(sock_fd, data, len, 0)) > 0) {
			data += s ;
			len -= s ;
		}



		char buf[1024] ;
		data = 0x0 ;
		len = 0 ;
		if ( (s = recv(sock_fd, buf, 1023, 0)) > 0 ) {
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
		else{
			printf("watong\n");
		}

		
	printf(">%s\n", data); 
	}
*/

	pthread_t Input_Send;
	if(pthread_create(&Input_Send, 0x0,input_send, (void*)&sock_fd)<0){
		printf("in send thread error\n");
		exit(1);
	}

	pthread_t Receive_Print;
	if(pthread_create(&Receive_Print, 0x0,receive_print, (void*)&sock_fd)<0){
		printf("receve print thread error\n");
		exit(1);
	}

	pthread_join(Input_Send, NULL);
	pthread_join(Receive_Print, NULL);

} 

