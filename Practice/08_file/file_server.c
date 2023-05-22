#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sd, clnt_sd;
	FILE * fp;
	char buf[BUF_SIZE];
	int read_cnt;
	
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t clnt_adr_sz;
	
	if(argc!=2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	}
	
	fp=fopen("test.dat", "rb"); 
	serv_sd=socket(PF_INET, SOCK_STREAM, 0);   
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
	listen(serv_sd, 5);
	
	clnt_adr_sz=sizeof(clnt_adr);    
	clnt_sd=accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	
	while(1)  // 파일 하나를 정해진 크기에 만큼 나눠 하나씩 전송 (버퍼에 쌓는다.)
	{
		read_cnt=fread((void*)buf, 1, BUF_SIZE, fp);	// 버퍼 사이즈 만큼 fp를 읽어서 버퍼에 저장, 1은 바이트 단위	
		if(read_cnt<BUF_SIZE)	// 지정된 크기보다 작은 크기를 읽는다는 것은 30바이트 보다 적은 크기의 데이터가 남았다는 뜻으로 마지막 부분을 의미
		{
			write(clnt_sd, buf, read_cnt);	// 마지막 부분의 데이터를 전송하고	
			break;  // 루프해제
		}
		write(clnt_sd, buf, BUF_SIZE);  // 통신용 소켓에다가 저장된 만큼의 데이터를 전송
		// 현재 코드에선 30바이트씩 client에게 계속 전송
	}
	
	shutdown(clnt_sd, SHUT_WR);	 // SHUT_WR은 출력 스트림만 닫음
	// close와 같은 역할 : EOF를 client로 전송함
	// close와 다른 기능 : half close, 소캣을 닫지만 받을 것이 있을 때 close대신 사용
	/*
		소캣은 입력 스트림과 출력 스트림을 만듦
	   [서버]       [클라]   <--- shutdown(clnt_sd, SHUT_WR);
		출력 x ----- 입력 x
		 |           |
		입력 o ----- 출력 o
	*/
	// 위 코드에 적용해본다면 서버의 출력 스트림을 닫고(SHUT_WR) 입력 스트림은 살아있고 클라이언트의 입력은 닫히고 출력은 살아있게 됨
	read(clnt_sd, buf, BUF_SIZE);
	printf("Message from client: %s \n", buf);
	
	fclose(fp);
	close(clnt_sd); // shutdown(clnt_sd, SHUT_RD);
	close(serv_sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}