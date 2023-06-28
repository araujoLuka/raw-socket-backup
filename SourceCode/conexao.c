#include "../file_transfer.h"




//=====================================================

extern tratador global_info;

//=====================================================




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



void startConnection() {
    global_info.socket = makeRawSocket("enp3s0");
}


//------------------------------------------------------------------


void encerraConexao() {
    global_info.funcionando = 0;

    //

    montaMensagem(0, 0, MEM_TIPO_ENCERRADO, NULL);
    enviaMensagem();

    //

    close(global_info.socket);
}