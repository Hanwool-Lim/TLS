#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>

// openssl
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

//homedir for key and cert file
#define HOME "./"

//key and cert file
#define CERTF HOME "server.crt"
#define KEYF HOME "server.key"

//소켓을 생성하는 함수
int OpenListener(int port){
	int sd; //listen sd
	struct sockaddr_in addr; //server address 
	//sockaddr_in : IPv4에서 소켓구조를 표현하는 구조체

	sd = socket(PF_INET, SOCK_STREAM, 0);
	//socket(int domain, int type, int protocol)
	//소켓을 생성하는 함수
		//domain : PF_INET
		//type : SOCK_STREAM = TCP
		//Protocol : type에서 미리 지정한 경우 0을 입력
	
	bzero(&addr, sizeof(addr));
	//bzero(void *ptr, size_t size)
	//메모리 공간을 size만큼 0으로 채우는 함수(ptr주소값부터 size바이트 만큼 0으로 채움)
	
	addr.sin_family = AF_INET;
	//sin_family : 주소체계(AF_INET)
	addr.sin_port = htons(port); 
	//sin_port : 포트번호
	addr.sin_addr.s_addr = INADDR_ANY;
	//sin_addr.sddr : IP주소, INADDR_ANY를 사용하면 IP주소를 자동으로 찾아서 대입

	if(bind(sd, (struct sockaddr *) &addr, sizeof(addr)) != 0){
		perror("can't bind port");
		abort(); //terminate program
	}
	//bind(int sockfd, struct sockaddr *addr, socklen_t addlen)
	//클라이언트에서 접속할수 있는 서버의 주소를 설정하는 함수(소켓에 정보를 저장)
		//소켓 : 프로세스가 네트워크를 통해 데이터를 주고받기위한 창구
			//소켓 : IP주소, 포트번호, 프로토콜이 모여져서 만들어지는 것
		//포트 : 네트워크를 통해 데이터를 주고받는 프로세스를 식별하기위한 고유한 값

	if(listen(sd, 10)!=0){
		perror("Can't configure listening port");
		abort();
	}
	//listen : 지정한 소켓 디스크립터로 클라이언트의 접속요청을 기다리는 함수(보통 bind()함수와 같이 사용)
	return sd;
}

int isRoot(){
	if(getuid()!=0)
		return 0;

	else
		return 1;
}

SSL_CTX* InitServerCTX(void){
	SSL_METHOD *method;
	SSL_CTX *ctx;

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	method = TLSv1_2_server_method();
	ctx = SSL_CTX_new(method);

	if(ctx == NULL){
		ERR_print_errors_fp(stderr);
		abort();
	}
	return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile){
	if(
