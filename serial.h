extern char estado;
extern char hora;
extern char minuto;
extern char respostaPC;
extern char OLU;
extern idata char pacote[30];

void configura_serial();
void escreve_serial(char* mensagem);
void trata_interrupcao_serial();
void le_parametros();
char trata_dados();
void solicita_senador(char* codigo);
void solicita_governador(char* codigo);
void solicita_presidente(char* codigo);

