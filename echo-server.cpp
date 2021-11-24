#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <thread>
#include <set>
using namespace std;

set<int> client_list;

void usage()
{
	printf("syntax: echo-server <port> [-e[-b]]\n");
	printf("sample: echo-server 1234 -e -b\n");
}

void recvThread(int sd)
{
	printf("Client %d connected!\n", sd);
	static const int SZ = 1000;
	char buf[SZ];
	for(;;) {
		ssize_t res = recv(sd, buf, SZ - 1, 0);
		if(res == 0 || res == -1)break;
		buf[res] = 0;
		printf("Client %d send message : %s", sd, buf);
	}
	client_list.erase(sd);
	printf("Client %d disconnected!\n", sd);
	close(sd);
	exit(0);
}

int main(int argc, char **argv)
{
	if(argc != 4) {
		usage();
		return -1;
	}
	
	if(strncmp(argv[2], "-e", 2) != 0 || strncmp(argv[3], "-b", 2) != 0) {
		usage();
		return -1;
	}
	
	uint16_t port = stoi(argv[1]);
	
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1) {
		printf("Error occured with open socket!\n");
		return -1;
	}
	
	int res, optval = 1;
	res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(res == -1) {
		printf("Error occured with setsockopt!\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if(res2 == -1) {
		printf("Error occured with bind!\n");
		return -1;
	}
	
	res = listen(sd, 5);
	if(res == -1) {
		printf("Error occured with listen!\n");
		return -1;
	}
	
	for(;;) {
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(client_addr);
		int client_sd = accept(sd, (struct sockaddr *)&client_addr, &len);
		if(client_sd == -1) {
			printf("Error occured with accept!\n");
			return -1;
		}
		
		client_list.insert(client_sd);
		
		thread *t = new thread(recvThread, client_sd);
		t->detach();
	}
	close(sd);
	return 0;
}
