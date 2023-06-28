typedef struct {
    int alarm_check;
} timeout;



//--------------------



#define TIMEOUT_START_MICRO 0
#define TIMEOUT_START_SEGUN 5
#define TIMEOUT_CONTI_MICRO 0
#define TIMEOUT_CONTI_SEGUN 5


//--------------------


// funcao que seta a funcaoTratadora como
// a funcao chamada pelo alarme
void preparaAlarmes();

// inicia o temporizador do alarme
void iniciaAlarme();

// para o temporizador do alarme
void paraAlarme();