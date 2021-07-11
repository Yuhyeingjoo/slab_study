#include <unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include<dirent.h>
#include<pthread.h>
void send_mes(int conn, void* , int);
struct meta_data{
	int version;
	int len;
	char* name;
	struct meta_data * next;
};
int find_version(char*); 
struct meta_data * front= 0x0;
int theNumberOf = 0;
pthread_mutex_t mutex;
struct thread_argument{
	int con;
	char *key;
};
void list_search();
void new_list(char*file_name ){
	printf("new list\n");
	struct meta_data * New;
	New = (struct meta_data*)malloc(sizeof(struct meta_data));
	New->len = strlen(file_name);
	New->name = (char*)malloc(strlen(file_name));
	strcpy(New->name,file_name);
	New->version=0;
	New->next = NULL;
	struct meta_data *cur;
//	pthread_mutex_lock(&mutex);
//	printf("new list mutex lock\n");
	cur = front;
	if(cur ==0x0){
			front = New;
			theNumberOf++;
	}
	else{
		while(1){
			if(strcmp(cur->name,New->name)==0){
				cur->version++;
				printf("cmp\n");
//				pthread_mutex_unlock(&mutex);
//				printf("new list mutex unlock\n");
				break;
			}
			else if(cur->next==NULL){
				cur->next = New;
				theNumberOf++;
//				pthread_mutex_unlock(&mutex);
//			printf("new list mutex unlock\n");
				break;
			}
			
			cur=cur->next;

		}
	}
	list_search();
	
}
void list_search(){
	struct meta_data * cur;
	cur = front;

	while(cur!=NULL){
		printf("list serach :%s\n",cur->name);
		cur= cur->next;
	}
}
void send_metadata(int conn){
	struct meta_data * cur;
	cur = front;
//	pthread_mutex_lock(&mutex);
	send_mes(conn,&theNumberOf,4);
//	printf("ib jang\n");
	while(cur!=NULL){
		printf("to %d  %s\n",conn,cur->name);
		send_mes(conn,&cur-> len,4);
		send_mes(conn,cur->name,strlen(cur->name));
		send_mes(conn, &cur->version,4);
		cur= cur->next;
	}
//	printf("out\n");
//	pthread_mutex_unlock(&mutex);
}
void send_mes(int conn, void* data, int len){
	int s;
	char* checker;
	checker =(char*) data;
	while(len>0 && (s=send(conn,checker,len,0))>0){
		checker+=s;
		len-=s;
	}
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
	int	version = find_version(file_name);
	send_mes(conn,&version,4);
	put_file(file_name,data, pwd);
	new_list(file_name);
	list_search();


}
int find_version(char* given_name){
	struct meta_data *c;
	
	c =front;
//	pthread_mutex_lock(&mutex);
//				printf("find version mutex unlock\n");
	while(c!=NULL){
		if(strcmp(c->name, given_name)==0){
//			pthread_mutex_unlock(&mutex);
//				printf("find version mutex unlock\n");
			return c->version;

		}
		c=c->next;
	}

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
	fseek(fp, 0, SEEK_END);    // 파일 포인터를 파일의 끝으로 이동시킴
        int size = ftell(fp);          // 파일 포인터의 현재 위치를 얻음
        rewind(fp);

	char * buf,* orig;
	buf = (char*)malloc(sizeof(char)*size);
	orig = buf;
	strcpy(buf,"");
	int count=0, version ;
	printf("file name : %s\n",file_name);
//	pthread_mutex_lock(&mutex);
	version = find_version(file_name);
	send_mes(conn,&version,4);
	
	count = fread(buf,sizeof(char),size,fp);
//	pthread_mutex_unlock(&mutex);
	send_mes(conn, buf,size);
	

	free(orig);
}

	

	void*
child_proc(void* received)
{
	struct	thread_argument *t_argu = (struct thread_argument*)received;
	int conn = t_argu->con;
	char* cwd;
	cwd = t_argu->key;
	char buf[1024] ;
	int len = 0 ;
	int buf_s;
	recv(conn,&buf_s,4,0);
	printf(">%d  from %d \n",buf_s, conn) ;
	//signify argument of fshare	
	if(buf_s ==1){
		send_metadata(conn);
		
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
		printf("buf 3 %d: %s\n",file_name_len,buf);
		server_put(conn, buf,cwd);
	
	}	
	
	shutdown(conn, SHUT_WR) ;
	


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
