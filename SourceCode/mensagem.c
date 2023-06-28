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
	mensagem men;

    //

    int mensagem_recebida = 0;


    while (!mensagem_recebida && !timeout_info.alarm_check) {
		nbytes = recv(global_info.socket, (char*) &men, sizeof(mensagem), 0);

        // caso receba:
		if (nbytes > 0)
		{
			// analise marcador de init, se nao tiver eh lixo
			if (men.marcador_ini != MARCADOR_INIT) {
				continue;
            }

			// copia dados para men_recebida e sai do loop
			men_recebida = men; 
			mensagem_recebida = 1;
		}
    }

    //

    // caso de timeout
	if (timeout_info.alarm_check)
		return 0;

    //
    
    fprintf(stderr, "DEBUG: Mensagem recebida com sucesso\n");
    return 1;

}


//----------------------------------------------------------------

// procedimento para conversa entre cliente e servidor
// controla o limite de timeouts
int conversaPadrao(int tam, int sequencia, int tipo, unsigned char* dados) {
    int tipoResposta;

    iniciaAlarme();

    //

    for (int timeouts = 0; timeouts < 2; timeouts++) {
        timeout_info.alarm_check = 0;

        printf("DEBUG: Enviando mensagem para servidor...\n");
        enviaMensagem(tam, sequencia, tipo, dados);

        if (recebeMensagem()) {
            printf("DEBUG: Recebido resposta do servidor\n");
            paraAlarme();

            tipoResposta = obtemTipoMensagem(men_recebida.tamanho_sequencia_tipo);
            if (tipoResposta == MEN_TIPO_NACK) {
                printf("ERRO: Recebido NACK\n");
                return 0;
            } else if (tipoResposta == MEN_TIPO_ERRO) {
                printf("ERRO: Recebido codigo de erro\n");
                return 0;
            }
            return 1;
        }

        printf("DEBUG: Falha ao receber resposta do servidor\n");
        printf("DEBUG: Numero de timeouts = %d\n", timeouts+1);
    }

    //

    paraAlarme();
    printf("DEBUG: Atingido limite de timeout\n");
    printf("DEBUG: Encerrando conexao!\n");
    encerraConexao();
    return 0;
}
