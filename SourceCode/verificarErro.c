#include "../file_transfer.h"




//=====================================================


extern mensagem men_recebida;
extern mensagem men_enviada;


//=====================================================




// verifica o marcado de inicio da mensagem
int verificaInicio(unsigned char marcador) {
    if (marcador == MARCADOR_INIT) { 
        return 1;
    }

    //

    return 0;
}



//----------------------------------------------------------------



// gera paridade de uma lista de char
// retorna um char como valor
char geraParidade(unsigned char* valores, int tam) {
    char returning_val = 0;

    //

    for (int byte = 0; byte < tam; byte ++) {
        returning_val = returning_val ^ valores[byte];
    }

    //

    return returning_val;
}


// verifica a paridade da mensagem recebida
int verificaParidade() {
    // gera a paridade obtida
    char teste = geraParidade(men_recebida.dados, obtemTamMensagem(men_recebida.tamanho_sequencia_tipo));

    //

    // se as 2 baterem
    if (teste == men_recebida.paridade_vertical) {
        return 1;
    }

    //

    return 0;
}