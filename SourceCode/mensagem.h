#ifndef MENSG_H
#define MENSG_H

#include <stdint.h>

typedef struct {
    // deve ser igual a 0111 1110
    unsigned char marcador_ini;

    uint16_t tamanho_sequencia_tipo;    // apenas usado para salvar os valores
    // primeiros 6 bits = tamanho
    // proximos 6 bits = sequencia
    // ultimos 4 bits = tipo

    unsigned char dados[64];

    unsigned char paridade_vertical;

} mensagem;



#define MARCADOR_INIT 126



// retorna o tipo da mensagem em int
int obtemTipoMensagem(unsigned int tamanho_sequencia_tipo);

// retorna o tipo da mensagem em int
int obtemSequenciaMensagem(unsigned int tamanho_sequencia_tipo);

// retorna o tipo da mensagem em int
int obtemTamMensagem(unsigned int tamanho_sequencia_tipo);

//--------------------------------------------------------------------

// da mensagem recebida, desmembra ela e salva os valores
// na variavel global
void desmembraMensagem();


// com os parametros recebidos,
// monta a mensagem e salva na variavel global.
void montaMensagem(int tam, int sequencia, int tipo, unsigned char* dados);


// envia a mensagem salva na variavel global para a outra maquina.
void enviaMensagem();


// inicia o timeout e espera uma mensagem,
// caso receba termina o timeout e retorna 1
// caso de timeout retorna 0
int recebeMensagem();


// procedimento para conversa entre cliente e servidor
// controla o limite de timeouts
int conversaPadrao();

// funcao do servidor de enviar e receber mensagens
void envia_recebe();


#endif