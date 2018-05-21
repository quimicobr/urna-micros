extern char estado;
extern char hora;
extern char minuto;
extern char respostaPC;
extern char OLU;
extern char fifo_recepcao[30];

void configura_serial();
void escreve_serial(char* mensagem);
void trata_interrupcao_serial();
void trata_dados();
