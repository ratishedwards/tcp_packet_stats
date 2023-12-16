#include<stdio.h>		// printf()
#include<string.h>		// memset()
#include<sys/socket.h>		// socket()
#include<netinet/in.h>		// IPPROTO_RAW
#include<unistd.h>		// close()
#include<stdlib.h>		// EXIT_FAILURE
#include<signal.h>
#include<netinet/ip.h>		// ip header
#include<linux/if_ether.h>	// ETH_P_IP

#define MAX_BUFF 65536

// Only intrested in counts stats for protocols upto 20, index 0 for all packets
int stats[21][2];

void update_stats(const char *buffer);
void dump_stats();

int main(int argc, char *argv[]){

	int sock_fd, bytes_read, saddr_size;
	int opt = 1;
	struct sockaddr saddr;
	char buffer[MAX_BUFF] = {0};

	if ((sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0){
		perror("socket()");
		exit(EXIT_FAILURE);}

	signal(SIGINT, dump_stats);

	printf("Counting packets\n");
	while(1){
		saddr_size = sizeof(saddr);
		if ((bytes_read = recvfrom(sock_fd, buffer, MAX_BUFF, 0, &saddr , &saddr_size)) < 0){
			perror("socket()");
			close(sock_fd);
			exit(EXIT_FAILURE);}

		update_stats(buffer);
		memset(buffer, 0, MAX_BUFF);
	}

	close(sock_fd);
	return 0;
}

void update_stats(const char *buffer){
	struct iphdr *ip_header = (struct iphdr *)(buffer + 14);
	// packets with ip protocol greater than 20 are not counted
	if (ip_header->protocol <= 20){
		stats[ip_header->protocol][1]++;
		stats[0][1]++;
		printf(".");
	}
	return;
}

void dump_stats(){
	printf("\n");
	for (int i = 1; i <= 20; i++){
		printf("Protocol[ %2d ] ---------------> packets received [ %4d]\n", i, stats[i][1]);
	}
	printf("Total Packets[ %4d ]\n", stats[0][1]);
	exit(EXIT_SUCCESS);
}
