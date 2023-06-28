#include "../file_transfer.h"


//=====================================================

tratador global_info;

extern mensagem men_recebida;
extern mensagem men_enviada;

FILE* arquivoAberto;

//=====================================================




// 
void trata_mensagem_recebida() {
    int initial_message;

    //

    fprintf(stderr, "DEBUG: Esperando mensagem...\n");
    recebeMensagem();

    fprintf(stderr, "DEBUG: Mensagem recebida!\n");
    initial_message = obtemTipoMensagem(men_recebida.tamanho_sequencia_tipo);

    //

    switch(initial_message) {
        case (MEM_TIPO_BACKUP_1) :
            montaMensagem(0, 0, MEM_TIPO_ACK, NULL);
            enviaMensagem();
        break;


        case (MEM_TIPO_NOME_ARQUIVO) :
            unsigned char nome[64];
            strcpy((char*) nome, (char*) men_recebida.dados);
            printf("arquivo: %s\n\n", nome);

            //

            arquivoAberto = fopen((char*) nome, "w");
            if (arquivoAberto == NULL) {
                montaMensagem(0, 0, MEM_TIPO_ERRO, NULL);
                enviaMensagem();
                fprintf(stderr, "ERRO ao abrir arquivo");
                return;
            }

            //

            montaMensagem(0, 0, MEM_TIPO_ACK, NULL);
            enviaMensagem();
        break;


        case (MEM_TIPO_DADOS) :

            strcpy((char*) nome, (char*) men_recebida.dados);

            printf("dados: %s\n\n", nome);
            fprintf(arquivoAberto, "%s", (char*)nome);
            
            montaMensagem(0, 0, MEM_TIPO_ACK, NULL);

            enviaMensagem();
            recebeMensagem();

        break;


        case (MEM_TIPO_ENCERRADO) :
            // mensagem que deve ser encerrado o programa, apenas sai
            // sem enviar nada nem processar nada

            global_info.funcionando = 0;
        break;
    }
}



//----------------------------------------------------------------



//
void envia_proxima_mensagem() {
    printf("Escolha oque fazer:\n");
    printf("(1) Backup 1 arquivo\n(2) Backup varios arquivo\n(3) Recupera 1 arquivo\n(4) Recupera varios arquivo\n(5) Muda Dir\n(6) Verifica arquivo\n(7) Encerra\n");

    //

    int input;
    scanf("%d", &input);

    //

    switch(input) {
        case (1) :
            printf("qual arquivo:\n");

            unsigned char nome[64];
            scanf("%s", nome);

            //

            montaMensagem(0, 0, MEM_TIPO_BACKUP_1, NULL);
            if (!conversaPadrao()) {
                return;
            }

            //

            montaMensagem(strlen((char*)nome), 0, MEM_TIPO_NOME_ARQUIVO, nome);
            if (!conversaPadrao()) {
                return;
            }

            //

            unsigned char dados[64];
            FILE* arquivo_backup = fopen((char*)nome, "r");

            while(feof(arquivo_backup)) {

                fgets((char*)dados, 63, arquivo_backup);

                //

                montaMensagem(strlen((char*)dados), 0, MEM_TIPO_DADOS, dados);
                if (!conversaPadrao()) {
                    return;
                }
            }

            //

            montaMensagem(0, 0, MEM_TIPO_FIM_ARQUIVO, NULL);
            if (!conversaPadrao()) {
                return;
            }
        break;

        case (2) :
            // manda multiplos arquivos
        break;

        case (3) :
            // obtem o nome do arquivo do usuario
            // pergunta se existe na outra maquina
            // se existe, começa a receber
            // recebe o titulo salvando em uma variavel
            // vai aos poucos preenchendo o arquivo
            // ate receber um fim de arquivo
        break;

        case (4) :
            // recupera multiplos arquivos
        break;

        case (5) :
            // manda uma mensagem que vamos mudar o diretorio
            // a mensagem contem o nome do novo diretorio nos dados 63 letras
            // espera uma mensagem de retorno caso deu certo ou tem erro
            // erros: diretorio nao existe, nao eh diretorio e arquivo
        break;

        case (6) :
            // envia que vamos verificar um arquivo
            // manda o titulo
            // se existe pede o MD5 dele
            // ao receber compara com o nosso e informa o usuario se ta certo
        break;

        case (7) :
            encerraConexao();
            // nao esperamos para receber a resposta apenas saimos
        break;
    }
}



//----------------------------------------------------------------



void inicia_transferencia() {
    printf("\nesse eh o client? (1 = sim , 0 = nao)\n");
    scanf("%d", &global_info.client);
    if (global_info.client > 1 || global_info.client < 0) {
        fprintf(stderr, "ERRO, valor invalido");
        return;
    }

    //

    startConnection();

    //

    preparaAlarmes();

    //

    global_info.funcionando = 1;

    while(global_info.funcionando) {
        
        if (global_info.client) {
            envia_proxima_mensagem();
        } else {
            trata_mensagem_recebida();
        }

    }
}