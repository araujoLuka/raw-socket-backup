#include "../file_transfer.h"



//=====================================================

timeout timeout_info;

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action;

// estrutura de inicializa��o do timer
struct itimerval timer;

//=====================================================




// funcao chamada pelo alarme
void funcaoTratadora(int signum) {
	fprintf(stderr, "Time Out\n");
	timeout_info.alarm_check = 1;
}



//----------------------------------------------------------------



// funcao que seta a funcaoTratadora como
// a funcao chamada pelo alarme
void preparaAlarmes() {
    action.sa_handler = funcaoTratadora;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(SIGALRM, &action, 0) < 0) {
		fprintf(stderr, "Erro: ao setar acao ativada por sinal\n");
		exit(1);
	}
}



//----------------------------------------------------------------



// inicia o temporizador do alarme
void iniciaAlarme() {
    timeout_info.alarm_check = 0;

    //

    // ajusta valores do temporizador para disparar
	// 1 milisegundo = 1000 microsegundos
	timer.it_value.tv_usec		= TIMEOUT_START_MICRO;		// primeiro disparo, em micro-segundos
	timer.it_value.tv_sec		= TIMEOUT_START_SEGUN;		// primeiro disparo, em segundos
	timer.it_interval.tv_usec	= TIMEOUT_CONTI_MICRO;		// disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec	= TIMEOUT_CONTI_SEGUN;		// disparos subsequentes, em segundos

	if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
		fprintf(stderr, "Erro: ao setar relogio\n");
		exit(1);
	}
}


// para o temporizador do alarme
void paraAlarme() {
    timer.it_value.tv_usec		= 0;		// primeiro disparo, em micro-segundos
	timer.it_value.tv_sec		= 0;		// primeiro disparo, em segundos
	timer.it_interval.tv_usec	= 0;		// disparos subsequentes, em micro-segundos
	timer.it_interval.tv_sec	= 0;		// disparos subsequentes, em segundos

	if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
		fprintf(stderr, "Erro: ao setar relogio\n");
		exit(1);
	}
}