#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>

// Allocate memory for an array of unsigned chars.
uint8_t *allocate_ustrmem(int len) {
	void *tmp;

	if (len <= 0) {
		fprintf(stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
		exit(EXIT_FAILURE);
	}

	tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
	if (tmp == NULL) {
		fprintf(stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
		exit(EXIT_FAILURE);
	}

	memset(tmp, 0, len * sizeof (uint8_t));
	return(tmp);
}

// Pega o mac da maquina atual
int getMac(int *rs, uint8_t **src_mac) {
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));
	if (ioctl(*rs, SIOCGIFHWADDR, &ifr) < 0) {
		perror("ioctl() failed to get source MAC address ");
		return 0;
	}

	// Copy source MAC address.
	memcpy (*src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));

	return 1;
}
 
// Gerador de socket raw (Professor Todt)
int makeRawSocket(char *interface) {
    // Cria arquivo para o socket sem qualquer protocolo
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }
 
    int ifindex = if_nametoindex(interface);
 
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;
    // Inicializa socket
    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }
 
    return soquete;
}

int main(int argc, char *argv[]) {
    int rs;
	char *interface = "eth0";
	uint8_t *src_mac;

	src_mac = allocate_ustrmem(6);

	rs = makeRawSocket(interface);

	if (!getMac(&rs, &src_mac))
		exit(EXIT_FAILURE);

	char buffer[255];
	char *hello = "Hello Raw";
	int nbytes = 0;

	while(1) {
		if (send(rs, hello, sizeof(&hello), 0) < 0) {
			fprintf(stderr, "Error to send\n");
		}

		if ((nbytes = recv(rs, buffer, 255, 0)) < 0)
		{
			fprintf(stderr, "Error to receive\n");
		} else {
			printf("Received %s\n", buffer);
		}
	}






    close(rs);

    return 0;
}
