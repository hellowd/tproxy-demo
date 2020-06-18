#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main() {
	struct sockaddr_in addr;
	char buf[256];
	int lis = -1;
	int downstream = -1;
	int upstream = -1;
	socklen_t len = sizeof(addr);
	int ret = 1;
	int value = 1;
	ssize_t rx, tx;

	lis = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(50080);

	setsockopt(lis, SOL_IP, IP_TRANSPARENT, &value, sizeof(value));

	if(bind(lis, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind:");
		goto cleanup;
	}
	listen(lis, 5);

	if((downstream = accept(lis, (struct sockaddr*)&addr, &len)) == -1) {
		perror("accept:");
		goto cleanup;
	}
	printf("Incomming connection\nfrom:%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	upstream = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(upstream, SOL_IP, IP_TRANSPARENT, &value, sizeof(value));
	if(bind(upstream, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind upstream:");
		goto cleanup;
	}

	getsockname(downstream, (struct sockaddr*)&addr, &len);
	printf(" to:%s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	if(connect(upstream, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("connect:");
		goto cleanup;
	}
	
	while(1) {
		rx = recv(downstream, buf, sizeof(buf), 0);
		if(rx <= 0) break;
		tx = send(upstream, buf, rx, 0);
		if(tx <= 0) break;
	}

	ret = 0;	
cleanup:
	if(lis != -1) close(lis);
	if(downstream != -1) close(downstream);
	if(upstream!= -1) close(upstream);
	return ret;
}
