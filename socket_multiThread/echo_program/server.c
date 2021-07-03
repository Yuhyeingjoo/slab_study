#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>

void 
file_read_print(FILE* fp, int conn){
	char buffer[17];
	char * buf;
	buf  = (char*)malloc(sizeof(char)*1024);
	strcpy(buf,"");
	int count=0, total = 0, s= 0;
	while(feof(fp)==0){
		count = fread(buffer,sizeof(char), 16,fp);
		strcat(buf,buffer);
		total+=count;	
		//memset(buffer,0,16);
	}
	printf("%s",buf);
	while(total>0&& (s = send(conn, buf, total, 0))>0){
		buf+=s;
		total-=s;
	}
}
void
*child_proc(void* soc)
{
	int conn = *(int*)soc;
	char buf[1024] ;
	char * data = 0x0, * orig = 0x0 ;
	int len = 0 ;
	int s ;
	char buffer[1024];
	while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
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
	printf(">%s\n", data) ;
	FILE* fp = fopen(data,"r");
	file_read_print(fp,conn);
	orig = data ;
	shutdown(conn, SHUT_WR) ;
	if (orig != 0x0) 
		free(orig) ;
	fclose(fp);
}

int 
main(int argc, char const *argv[]) 
{ 
	int listen_fd, new_socket ; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 

	char buffer[1024] = {0}; 

	listen_fd = socket(AF_INET /*IPv4*/, SOCK_STREAM /*TCP*/, 0 /*IP*/) ;
	if (listen_fd == 0)  { 
		perror("socket failed : "); 
		exit(EXIT_FAILURE); 
	}
	
	memset(&address, '0', sizeof(address)); 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY /* the localhost*/ ; 
	address.sin_port = htons(8093); 
	if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed : "); 
		exit(EXIT_FAILURE); 
	} 

	while (1) {
		if (listen(listen_fd, 16 /* the size of waiting queue*/) < 0) { 
			perror("listen failed : "); 
			exit(EXIT_FAILURE); 
		} 

		new_socket = accept(listen_fd, (struct sockaddr *) &address, (socklen_t*)&addrlen) ;
		if (new_socket < 0) {
			perror("accept"); 
			exit(EXIT_FAILURE); 
		} 
		
		pthread_t threadd;
		if(pthread_create(&threadd,0x0, child_proc, (void*)&new_socket)<0){
			printf("error\n");
			exit(1);
		}
	}
} 

