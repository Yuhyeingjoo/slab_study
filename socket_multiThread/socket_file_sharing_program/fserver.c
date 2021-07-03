#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include<dirent.h>
#include<pthread.h>
struct thread_argument{
	int con;
	char *key;
};
void send_mes(int conn, void* data, int len){
	int s;
	while(len>0 && (s=send(conn,data,len,0))>0){
		data+=s;
		len-=s;
	}
		}
void list(int conn, char* cwd){
	int len,s;
	char * ls, *orig;
	ls = (char*)malloc(sizeof(char)*256);
	orig = ls;
	strcpy(ls,"");
	DIR * dir = NULL;
        struct dirent * entry = NULL;
	 if( (dir = opendir(cwd)) == NULL)
        {
                printf("current directory error\n");
                exit(1);
        }
	 while( (entry = readdir(dir)) != NULL)

        {
		char name[16];
		strcpy(name,entry->d_name);
		if(name[0]!='.'){
			strcat(ls,name);
			strcat(ls,"\n");
		}

        }
	printf("%s",ls);
	len = strlen(ls); s= 0;
	while (len > 0 && (s = send(conn, ls, len, 0)) > 0) {
		ls += s ;
		len -= s ;
	}
	printf("%s\n",ls);
        closedir(dir);
	free(orig);
}

void put_file(char * file_name, char* data, char* pwd){
	char* path;
	path = (char*)malloc(sizeof(char)* 48);
	strcpy(path,"");
	strcat(path,"./");
	strcat(path,pwd);
	strcat(path,"/");
	strcat(path,file_name);

	FILE*fp = fopen(path,"w");

	printf("file name :%s\n", file_name);
	if(fp==NULL){
		printf("No Such File\n");
		exit(1);
	}
	fwrite(data,strlen(data),1,fp);
	fclose(fp);
}

void server_put(int conn, char * file_name, char* pwd){
	char* data=0x0;
	char buf[1024];
	int len=0, s=0;
	
	while ( (s = recv(conn, buf, 1023, 0)) > 0 ) {
		buf[s] = 0x0 ;
		printf("%d\n",s);
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
	
	printf("%s\n",data);
	put_file(file_name,data, pwd);
}
void get(char * file_name, int conn, char * pwd){
	char* path;
	path = (char*)malloc(sizeof(char)* 48);
	strcpy(path,"");
	strcat(path,"./");
	strcat(path,pwd);
	strcat(path,"/");
	strcat(path,file_name);

	FILE * fp = fopen(path,"r");
	if(fp==NULL){
		printf("NULL\n");
		exit(1);
	}
	char buffer[8];
	char * buf,* orig;
	buf = (char*)malloc(sizeof(char)*2048);
	orig = buf;
	strcpy(buf,"");
	int count=0, total = 0, s=0;
	printf("file name : %s\n",file_name);
	
	while(feof(fp)==0){
		count = fread(buffer,sizeof(char),4,fp);
		send_mes(conn, buffer,4);
		strcat(buf,buffer);
		total+=count;
		}

//	send(conn,buf,total,0);
//	free(orig);
}

	
void error_respond( int conn){
	char* message = "error\n";
	int len,s;
	len = sizeof(message)+1;
	while (len > 0 && (s = send(conn, message, len, 0)) > 0) {
		message += s ;
		len -= s ;
	}
	
//	free(message);
}
	void*
child_proc(void* received)
{
	struct	thread_argument *t_argu = (struct thread_argument*)received;
	int conn = t_argu->con;
	char* cwd;
	cwd = t_argu->key;

	char buf[1024] ;
	char *data = 0x0,  * orig = 0x0 ;
	int len = 0 ;
	int s, sig=1;
	int buf_s;
	recv(conn,&buf_s,4,0);

	printf(">%d \n",buf_s) ;

	


	//signify argument of fshare	
	
	if(buf_s ==1){
		list(conn,cwd);
	}
	else if(buf_s ==2){
		int k;
		recv(conn,&k,4,0);
		recv(conn,buf,k,0);	
		if(strstr(buf,"/")!=NULL){
			exit(1);
		}
		printf("%d: %s\n",k,buf);
		get(buf,conn, cwd);
	}
	else if(buf_s ==3){
		int file_name_len;
		recv(conn,&file_name_len,4,0);
		recv(conn,buf,file_name_len,0);
		printf("%d: %s\n",file_name_len,buf);
		server_put(conn, buf,cwd);
	
	}	
		

	
	shutdown(conn, SHUT_WR) ;
	orig = data ;
	if (orig != 0x0) 
		free(orig) ;


}



int main(int argc, char * argv[]){
	// command line arguments 
	//
	int  p_index=0, d_index=0,port_num=0;
	for(int i=1; i<argc; i++){
		if(strcmp("-p",argv[i])==0){
			p_index = i+1;
		}
		if(strcmp("-d",argv[i])==0)
			d_index = i+1;
	}

	printf("%s %s \n",argv[p_index],argv[d_index]);
	port_num = atoi(argv[p_index]);
	

	// connect socket

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
	address.sin_port = htons(port_num); 
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
		struct thread_argument * t_argu;
		t_argu = (struct thread_argument*)malloc(sizeof(struct thread_argument));
		t_argu->key = (char*)malloc(sizeof(char)*32);
		strcpy(t_argu->key,argv[d_index]);


		t_argu->con = new_socket;	
		if(pthread_create(&threadd, 0x0,child_proc,(void*)t_argu)<0){
			printf("thread error\n");
			exit(1);
		}
	}
	

}
