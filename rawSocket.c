#include <errno.h>
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

void connectionServer() {
    int rs = makeRawSocket("enp3s0");

    char buffer[255];
    int nbytes = 0;
    int size = 80;

    while(1) {
        printf("Waiting for message from client...\n");

        while (nbytes < 80) {
            if ((nbytes = recv(rs, buffer, 255, 0)) < 0) {
                fprintf(stderr, "Error to receive response\n");
                send(rs, "NACK", 80, 0);
                break;
            } else {
                printf("Received %s from server\n", buffer);
                send(rs, "ACK", 80, 0);
            }
        }
        nbytes = 0;

        sleep(1);
	}
    close(rs);
}

void connectionClient() {
    int rs = makeRawSocket("enp3s0");

    char buffer[255];
    int nbytes = 0;
    int size = 0;

    while(1) {
        printf("Tip a message to send: ");
        scanf("%s\n", buffer);
        
        size = strlen(buffer);
        if (size < 80)
            size = 80;

        if ((nbytes = send(rs, buffer, size, 0)) < 0) {
            fprintf(stderr, "Error to send - nbytes=%d - error=%d\n", nbytes, errno);
        } else {
            printf("Sended %d bytes with send()\n", nbytes);
        }
        nbytes = 0;

        printf("Waiting for response...\n");

        while (nbytes < 80) {
            if ((nbytes = recv(rs, buffer, 255, 0)) < 0) {
                fprintf(stderr, "Error to receive response\n");
                break;
            } else {
                printf("Received %s from server\n", buffer);
            }
        }
        nbytes = 0;

        sleep(1);
	}
    close(rs);
}

void startConnection(int client) {
    if (client)
        connectionClient();

    connectionServer();
}

int main(int argc, char *argv[]) {
    int userType;

    printf("Set your user type (1 - Client | 0 - Server): ");
    scanf("%d", &userType);

    startConnection(userType);

    return 0;
}
