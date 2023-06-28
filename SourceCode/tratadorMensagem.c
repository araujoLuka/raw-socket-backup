#include <stdio.h>
#include "../file_transfer.h"


//=====================================================

tratador global_info;

extern mensagem men_recebida;
extern mensagem men_enviada;

FILE* arquivoAberto = NULL;
char tipoDeAcesso[3];
    
int mult = 0, totalArquivos = 0, arq = 0;
int recupera = 0;

//=====================================================


void debug(){}


// 
void trata_mensagem_recebida() {
    int initial_message;
    char nome[64];
    char char_buffer[64];

    //

    fprintf(stderr, "DEBUG: Esperando mensagem...\n");
    recebeMensagem();

    fprintf(stderr, "DEBUG: Mensagem recebida!\n");


    // analise da paridade vertical, se tiver errado continua no loop
    char paridade = geraParidade(men_recebida.dados, 
                                 obtemTamMensagem(men_recebida.tamanho_sequencia_tipo));
    if (men_recebida.paridade_vertical != paridade) {
        fprintf(stderr, "DEBUG: Mensagem recebida, mas paridade errada\n");
        enviaMensagem(0, 0, MEN_TIPO_NACK, NULL);
        return;
    }

    initial_message = obtemTipoMensagem(men_recebida.tamanho_sequencia_tipo);

    //

    switch(initial_message) {
        case (MEN_TIPO_BACKUP_1) :
            if (mult == 0) {
                printf("Tipo: Inicio de backup para 1 arquivo\n");
            }
            strcpy(tipoDeAcesso, "w");

            strcpy((char*) nome, (char*) men_recebida.dados);
            printf("Dados: Nome de arquivo recebido %s\n", nome);

            //

            arquivoAberto = fopen((char*) nome, tipoDeAcesso);
            if (arquivoAberto == NULL) {
                fprintf(stderr, "ERRO: Falha ao abrir arquivo\n");
                enviaMensagem(0, 0, MEN_TIPO_ERRO, NULL);
                return;
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
            debug();
            strcpy(tipoDeAcesso, "r");

            strcpy((char*) nome, (char*) men_recebida.dados);
            printf("Dados: Nome de arquivo para ser recuperado %s\n", nome);

            arquivoAberto = fopen((char *)men_recebida.dados, tipoDeAcesso);
            if (arquivoAberto == NULL) {
                fprintf(stderr, "ERRO: arquivo solicitado nao existe no servidor\n");
                enviaMensagem(0, 0, MEN_TIPO_ERRO, NULL);
                return;
            }

            enviaMensagem(strlen(nome), 0, MEN_TIPO_RECUPERA_NOME, (unsigned char *)nome);
        break;

        case (MEN_TIPO_RECUPERA_MULT) :
            strcpy(tipoDeAcesso, "r");

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
        break;

        //

        case (MEN_TIPO_FIM_ARQUIVO) :

            fclose(arquivoAberto);
            arquivoAberto = NULL;

            //
            
            if (mult == 0) {
                printf("Arquivo recebido: %s\n\n", nome);
            }
            else {
                printf("Arquivo %2d recebido: %s\n\n", arq, nome);
                arq++;
                if (totalArquivos - arq > 0) {
                    printf("Faltam %2d arquivos a receber\n", totalArquivos - arq);
                } else {
                    printf("Foram recebidos todos os %d arquivos multiplos\n", totalArquivos);
                    printf("Aguardando por encerramento de backup multiplo...\n\n");
                }
            }
            
            recupera = 0;
            
            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);

        break;

        case (MEN_TIPO_FIM_MULT) :
            printf("Tipo: Fim de backup multiplo\n");

            printf("Relatorio: Recebido %d arquivos de %d possiveis\n", arq, totalArquivos);

            mult = 0;
            totalArquivos = 0;
            arq = 0;

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);
        break;

        //

        case (MEN_TIPO_RECUPERA_NOME) :
            // recebe o nome de um arquivo para ser recuperado
            // responde com o arquivo
            printf("Tipo: Inicio de recuperacao para 1 arquivo\n");
            strcpy(tipoDeAcesso, "w");

            strcpy((char*) nome, (char*) men_recebida.dados);
            printf("Dados: Nome de arquivo recebido %s\n", nome);

            //

            arquivoAberto = fopen((char*) nome, tipoDeAcesso);
            if (arquivoAberto == NULL) {
                fprintf(stderr, "ERRO: Falha ao abrir arquivo\n");
                enviaMensagem(0, 0, MEN_TIPO_ERRO, NULL);
                return;
            }

            //

            enviaMensagem(0, 0, MEN_TIPO_ACK, NULL);

        break;


        case (MEN_TIPO_DADOS) :

            if (arquivoAberto == NULL) {
                fprintf(stderr, "ERRO: nenhum arquivo aberto para gravar dados\n");
                enviaMensagem(0, 0, MEN_TIPO_NACK, NULL);
                return;
            }

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

int enviarArquivo () {
    int i = 0;
    unsigned char nome[64];
    unsigned char dados[64];

    printf("Qual arquivo deseja enviar: ");
    scanf("%s", nome);

    FILE* arquivo_backup = fopen((char*)nome, "r");

    if (!arquivo_backup) {
        fprintf(stderr, "ERRO: Arquivo '%s' nao existente!\n\n", nome);
        return -1;
    }

    //

    if (!conversaPadrao(strlen((char*)nome), 0, MEN_TIPO_BACKUP_1, nome)) { 
        return -2;
    }

    //
    
    while(fgets((char*)dados, 63, arquivo_backup) != NULL) {
        if (!conversaPadrao(strlen((char*)dados), i, MEN_TIPO_DADOS, dados)) {
            return -3;
        }

        if (++i >= 63)
            i = 0;
    }

    //

    if (!conversaPadrao(0, 0, MEN_TIPO_FIM_ARQUIVO, NULL)) {
        return -4;
    }

    return 0;
}


//
void envia_proxima_mensagem() {
    int totalArquivos = 0;
    unsigned char nome[64];

    printf("Escolha o que fazer:\n");
    printf("(1) Backup 1 arquivo\n");
    printf("(2) Backup varios arquivos\n");
    printf("(3) Recupera 1 arquivo\n");
    printf("(4) Recupera varios arquivos\n");
    printf("(5) Muda Diretorio\n");
    printf("(6) Verifica arquivo\n");
    printf("(7) Encerra\n");

    //

    int input, i;
    scanf("%d", &input);

    //

    switch(input) {
        case (1) :
            enviarArquivo();
        break;

        case (2) :
            printf("Quantos arquivos deseja enviar: ");

            scanf("%d", &totalArquivos);

            //

            if (!conversaPadrao(strlen((char*)&totalArquivos), 0, MEN_TIPO_BACKUP_MULT, (unsigned char*)&totalArquivos)) {
                return;
            }

            //

            for (i = 0; i < totalArquivos; i++) {
                if (enviarArquivo() < 0) {
                    // falha ao enviar arquivo
                }
            }

            //

            if (!conversaPadrao(0, 0, MEN_TIPO_FIM_MULT, NULL)) {
                return;
            }
        break;

        case (3) :
            // obtem o nome do arquivo do usuario
            printf("Qual arquivo deseja recuperar: ");
            scanf("%s", nome);

            // pergunta se existe na outra maquina
            enviaMensagem(strlen((char *)nome), 0, MEN_TIPO_RECUPERA_1, nome);
            trata_mensagem_recebida();

            if (obtemTipoMensagem(men_recebida.tamanho_sequencia_tipo) == MEN_TIPO_ERRO) {
                fprintf(stderr, "ERRO: arquivo nao existe no servidor\n");
                return;
            }

            recupera = 1;
            while (recupera && obtemTipoMensagem(men_recebida.tamanho_sequencia_tipo) != MEN_TIPO_NACK) {
                trata_mensagem_recebida();
            }

            if (recupera) {
                fprintf(stderr, "ERRO: falha ao recuperar arquivo\n");
                recupera = 0;
                enviaMensagem(0, 0, MEN_TIPO_FIM_ARQUIVO, NULL);
                return;
            }

            strcpy((char*)nome, (char*)men_recebida.dados);
            printf("Arquivo %s restaurado com sucesso!\n", nome);

            enviaMensagem(0, 0, MEN_TIPO_FIM_ARQUIVO, NULL);
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
