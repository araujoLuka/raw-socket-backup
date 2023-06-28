#include "../file_transfer.h"


//=====================================================

tratador global_info;

extern mensagem men_recebida;
extern mensagem men_enviada;

FILE* arquivoAberto;
char tipoDeAcesso[3];
    
int mult = 0, totalArquivos = 0, arq = 0;

//=====================================================




// 
void trata_mensagem_recebida() {
    int initial_message;
    unsigned char char_buffer[64];

    //

    fprintf(stderr, "DEBUG: Esperando mensagem...\n");
    recebeMensagem();

    fprintf(stderr, "DEBUG: Mensagem recebida!\n");


    // analise da paridade vertical, se tiver errado continua no loop
    char paridade = geraParidade(men_recebida.dados, 
                                 obtemTamMensagem(men_recebida.tamanho_sequencia_tipo));
    if (men_recebida.paridade_vertical != paridade) {
        fprintf(stderr, "DEBUGG Mensagem recebida, mas paridade errada\n");
        enviaMensagem(0, 0, MEN_TIPO_NACK, NULL);
        return;
    }

    initial_message = obtemTipoMensagem(men_recebida.tamanho_sequencia_tipo);

    //

    switch(initial_message) {
        case (MEN_TIPO_BACKUP_1) :
            if (mult == 0) {
                printf("Tipo: Inicio de backup para 1 arquivo\n");
                strcpy(tipoDeAcesso, "w");
            }

            printf("Dados: Nome de arquivo recebido %s\n", men_recebida.dados);
            strcpy((char*) char_buffer, (char*) men_recebida.dados);

            //

            arquivoAberto = fopen((char*) char_buffer, tipoDeAcesso);
            if (arquivoAberto == NULL) {
                enviaMensagem(0, 0, MEN_TIPO_ERRO, NULL);
                fprintf(stderr, "ERRO ao abrir arquivo");
                return;
            }

            //
            
            if (mult == 0) {
                printf("Arquivo recebido: %s\n\n", char_buffer);
            }
            else {
                printf("Arquivo %2d recebido: %s\n\n", arq, char_buffer);
                arq++;
                if (totalArquivos - arq > 0) {
                    printf("Faltam %2d arquivos a receber\n", totalArquivos - arq);
                } else {
                    printf("Foram recebidos todos os %d arquivos multiplos\n", totalArquivos);
                    printf("Aguardando por encerramento de backup multiplo...\n\n");
                }
            }

            //

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
        break;

        case (MEN_TIPO_BACKUP_MULT) :
            printf("Tipo: Inicio de backup para multiplos arquivos\n");
            mult = 1;
            totalArquivos = (int)*men_recebida.dados;
            arq = 1;

            if (totalArquivos < 0) {
                fprintf(stderr, "ERRO: total de arquivos a realizar backup precisa ser maior que zero\n");
                enviaMensagem(0, 0, MEN_TIPO_ERRO, NULL);
                return;
            }

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
        break;

        case (MEN_TIPO_RECUPERA_1) :
            strcpy(tipoDeAcesso, "r");

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
        break;

        case (MEN_TIPO_RECUPERA_MULT) :
            strcpy(tipoDeAcesso, "r");

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
        break;

        //

        case (MEN_TIPO_FIM_ARQUIVO) :

            fclose(arquivoAberto);
            
            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);

        break;

        case (MEN_TIPO_FIM_MULT) :
            printf("Tipo: Fim de backup multiplo\n");

            if (totalArquivos - arq > 0) {
                fprintf(stderr, "ERRO: Houve inconsistencia nos arquivos recebidos e alguns nao foram processados\n");
                enviaMensagem(0, 0, MEN_TIPO_ERRO, NULL);
            } else {
                enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
            }

            mult = 0;
            totalArquivos = 0;
            arq = 0;
        break;

        //

        case (MEN_TIPO_RECUPERA_ARQUIVO) :
            // recebe o nome de um arquivo para ser recuperado
            // responde com o arquivo

        break;


        case (MEN_TIPO_DADOS) :

            strcpy((char*) char_buffer, (char*) men_recebida.dados);

            printf("dados: %s\n\n", char_buffer);
            fprintf(arquivoAberto, "%s", (char*)char_buffer);
            
            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);

        break;

        case (MEN_TIPO_ENCERRADO) :
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

    int input, i;
    scanf("%d", &input);

    //

    switch(input) {
        case (1) :
            printf("qual arquivo:\n");

            unsigned char nome[64];
            unsigned char dados[64];
            scanf("%s", nome);

            FILE* arquivo_backup = fopen((char*)nome, "r");

            if (!arquivo_backup) {
                fprintf(stderr, "ERRO: Arquivo '%s' nao existente!\n\n", nome);
                return;
            }

            //

            if (!conversaPadrao(strlen((char*)nome), 0, MEN_TIPO_BACKUP_1, nome)) {
                return;
            }

            //

            i = 0;

            while(!feof(arquivo_backup)) {

                fgets((char*)dados, 63, arquivo_backup);

                //

                if (!conversaPadrao(strlen((char*)dados), i, MEN_TIPO_DADOS, dados)) {
                    return;
                }

                if (++i >= 63)
                    i = 0;
            }

            //

            if (!conversaPadrao(0, 0, MEN_TIPO_FIM_ARQUIVO, NULL)) {
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
