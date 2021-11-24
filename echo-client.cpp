#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <thread>
#include <iostream>
using namespace std;

void usage()
{
	printf("syntax: echo-client <ip> <port>\n");
	printf("sample: echo-client 127.0.0.1 1234\n");
}

void recvThread(int sd)
{
	printf("connected!\n");
	static const int SZ = 1000;
	char buf[SZ];
	for(;;) {
		ssize_t res = recv(sd, buf, SZ - 1, 0);
		if(res == 0 || res == -1)break;
		buf[res] = 0;
		printf("Server send message : %s\n", buf);
	}
	printf("disconnected!\n");
	close(sd);
	exit(0);
}

int main(int argc, char **argv)
{
	if(argc != 3) {
		usage();
		return -1;
	}
	
	struct in_addr ip;
	
	int res = inet_pton(AF_INET, argv[1], &ip);
	if(res != 1) {
		printf("Error occured with get IP address!\n");
		return -1;
	}
	uint16_t port = stoi(argv[2]);
	
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if(sd == -1) {
		printf("Error occured with open socket!\n");
		return -1;
	}
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = ip;
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));
	
	res = connect(sd, (struct sockaddr *)&addr, sizeof(addr));
	if(res == -1) {
		printf("Error occured with socket connect!\n");
		return -1;
	}
	
	thread t(recvThread, sd);
	t.detach();
	
	for(;;) {
		string c;
		getline(cin, c);
		c += "\r\n";
		ssize_t res = send(sd, c.c_str(), c.size(), 0);
		if(res == 0 || res == -1) break;
	}
	close(sd);
	return 0;
}
