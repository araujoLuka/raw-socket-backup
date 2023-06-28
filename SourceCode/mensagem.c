#include "../file_transfer.h"



//=====================================================

mensagem men_recebida;
mensagem men_enviada;

extern timeout timeout_info;

extern tratador global_info;

//=====================================================




// retorna o tipo da mensagem em int
int obtemTipoMensagem(unsigned int tamanho_sequencia_tipo) {
    // pega para returning val, apenas os ultimos 4 bits
    int returning_val = tamanho_sequencia_tipo & 15;

    //

    return returning_val;
}


// retorna o tipo da mensagem em int
int obtemSequenciaMensagem(unsigned int tamanho_sequencia_tipo) {
    // pega para returning val, apenas os 6 bits do meio
    int returning_val = (tamanho_sequencia_tipo >> 4) & 63;

    //

    return returning_val;
}


// retorna o tipo da mensagem em int
int obtemTamMensagem(unsigned int tamanho_sequencia_tipo) {
    // pega para returning val, apenas os primeiros 6 bits
    int returning_val = (tamanho_sequencia_tipo >> 10);

    //

    return returning_val;
}



//----------------------------------------------------------------



// com os parametros recebidos,
// monta a mensagem e salva na variavel global.
void montaMensagem(int tam, int sequencia, int tipo, unsigned char* dados) {
    men_enviada.marcador_ini = MARCADOR_INIT;

    //

    men_enviada.paridade_vertical = geraParidade(dados, tam);

    //

    men_enviada.tamanho_sequencia_tipo = 0;
    men_enviada.tamanho_sequencia_tipo = men_enviada.tamanho_sequencia_tipo | ((tam & 63) << 10) | ((sequencia & 63) << 4) | (tipo & 15);

    //

    for (int i = 0; i < tam; i ++) {
        men_enviada.dados[i] = dados[i];
    }
}



//----------------------------------------------------------------



// envia a mensagem salva na variavel global para a outra maquina.
void enviaMensagem(int tam, int sequencia, int tipo, unsigned char* dados) {
    montaMensagem(tam, sequencia, tipo, dados);

    //

    int nbytes = 0;

    nbytes = send(global_info.socket, (char*) &men_enviada, sizeof(mensagem), 0);

    if (nbytes < 0) {
        fprintf(stderr, "ERRO: ao enviar a mensagem\n");
    }

}



// inicia o timeout e espera uma mensagem,
// caso receba termina o timeout e retorna 1
// caso de timeout retorna 0
int recebeMensagem() {
	int nbytes = 0;
	char buffer[sizeof(mensagem)], paridade;
	mensagem men;

    //

    int mensagem_recebida = 0;


    while (!mensagem_recebida && !timeout_info.alarm_check) {
		nbytes = recv(global_info.socket, &buffer, sizeof(mensagem), 0);

        // caso receba:
		if (nbytes > 0)
		{
			// copia o buffer para a struct mensagem temporaria
			strcpy((char*) &men, buffer); // AI LUCAO, acho q eh meior copiar direto pra men_recebida, n precisa se preocupar em perder a original em caso de erro

			// analise marcador de init, se nao tiver eh lixo
            fprintf(stderr, "%d\n", men.marcador_ini);
			if (men.marcador_ini != MARCADOR_INIT) {
                fprintf(stderr, "DEBUGG Mensage recebida, mas nao possui marcador de inicio\n");
				continue;
            }

			// analise da paridade vertical, se tiver errado continua no loop
			paridade = geraParidade(men.dados, obtemTamMensagem(men.tamanho_sequencia_tipo));
			if (men.paridade_vertical != paridade) {
                fprintf(stderr, "DEBUGG Mensagem recebida, mas paridade errada\n");
				continue;
            }

			// copia dados para men_recebida e sai do loop
			men_recebida = men; // AI LUCAO, acho q eh meior copiar direto pra men_recebida, n precisa se preocupar em perder a original em caso de erro
			mensagem_recebida = 1;
		}
    }

    //

    // caso de timeout
	if (timeout_info.alarm_check)
		return 0;

    //
    
    fprintf(stderr, "DEBUGG Mensagem recebida com sucesso\n");
    return 1;

}


//----------------------------------------------------------------

// procedimento para conversa entre cliente e servidor
// controla o limite de timeouts
int conversaPadrao(int tam, int sequencia, int tipo, unsigned char* dados) {
    iniciaAlarme();

    //

    for (int timeouts = 0; timeouts < 2; timeouts++) {
        timeout_info.alarm_check = 0;

        printf("DEBUG: Enviando mensagem para servidor...\n");
        enviaMensagem(tam, sequencia, tipo, dados);

        if (recebeMensagem()) {
            printf("DEBUG: Recebido resposta do servidor\n");
            paraAlarme();
            return 1;
        }

        printf("DEBUG: Falha ao receber resposta do servidor\n");
    }

    //

    paraAlarme();
    printf("DEBUG: Atingido limite de timeout\n");
    printf("DEBUG: Encerrando conexao!\n");
    encerraConexao();
    return 0;
}
