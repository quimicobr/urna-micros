#include<serial.h>
#include<lcd.h>
#include<reg52.h>
#include<myString.h>

//0 	EXTERNAL INT 0 	0003h
//1 	TIMER/COUNTER 0 	000Bh
//2 	EXTERNAL INT 1 	0013h
//3 	TIMER/COUNTER 1 	001Bh
//4 	SERIAL PORT 	0023h
//5 	TIMER/COUNTER 2 (8052) 	002Bh

#define NORMAL 0
#define BLOQUEADA 1
#define AUTONOMA 2 

#define OK 0
#define PE 1
#define PC 2 
#define PT 3
#define PS 4
#define PG 5 
#define PP 6

#define NAO 0
#define LE_N 1
#define LE_PARAMETRO 2

char fifo_recepcao[30];

void configura_serial(){
	//configura o timer 1
	TMOD &= 0x0F;
	TMOD |= 0x20; //timer de 8 bits com recarga automática
	TL1 = 0xFD; //19200 bps
	TH1 = 0xFD; //recarga
	TR1 = 1; //liga o timer
	//configura a serial
	SCON = 0x50; //serial com freq variável + habilita recepcao
	EA = 1; //habilita as interrrupcoes do chip
	ES = 1;	//habilita interrupcao da serial
}

void escreve_serial(char* mensagem){
char i;
	for(i = 0; mensagem[i] != 0 ; i++){
		SBUF = mensagem[i];
		while(!TI);
		TI=0;
	}
}

void clear_FIFO(){
	char i;
	for (i = 0; fifo_recepcao[i] != '\0' && i<30; i++) fifo_recepcao[i] = '\0';
	
}



void trata_interrupcao_serial() interrupt 4 {	
	static char pos = 0, l_final = 2, lendo_par = 0;
	char add;
	if(RI == 1){
		RI = 0;
		fifo_recepcao[pos] = SBUF;
		pos++;
		
		if(pos == l_final){
			//Aqui existem tres opções: se nao ha parametros a serem lidos, l_final continuara a mesmo
			//
			if (lendo_par == NAO){
				add = trata_dados();
				
				//Se trata_dados() mostra que não há mais bytes a serem lidos, limpa a FIFO e volta para 
				//a posição inicial
				if (add == 0){
					clear_FIFO();
					pos = 0;
					l_final = 2;
				}
				
				//Se trata_dados() envia o valor -1, isso quer dizer que a serial está esperando o valor 
				//n que indica o número de bytes a serem lidos
				else if(add == -1) {
					lendo_par = LE_N;
					l_final++;
				}
				
				//Se trata_dados() mostra que há mais bytes a serem lidos, adiciona o número de bytes 
				else{
					l_final += add;
					lendo_par = LE_PARAMETRO;
				}	
			}
			else if (lendo_par == LE_N){
				add = fifo_recepcao[2];
				l_final += add;
				lendo_par = LE_PARAMETRO;
			}
			else if (lendo_par == LE_PARAMETRO){
				le_parametros();
				clear_FIFO();
				pos = 0;
				lendo_par = NAO;
				l_final = 2;
			}			
		}
	}
}

void le_parametros(){
	if(fifo_recepcao[0] == 'P'){
		switch(fifo_recepcao[1]){
			case('H'):
				hora = fifo_recepcao[2];
				minuto = fifo_recepcao[3];
				return;
				
			case('S'):
				if (respostaPC == PS){
					copia_string(pacote, fifo_recepcao);
					respostaPC = OK;
				}
				return;
			case('G'):
				if (respostaPC == PG){
					copia_string(pacote, fifo_recepcao);
					respostaPC = OK;
				}
				return;
				
			case('P'):
				if (respostaPC == PP){
					copia_string(pacote, fifo_recepcao);
					respostaPC = OK;
				}
				return;
		}
}


//Trata dados deve ler toda a mensagem mandada pela serial
//Deve retornar o quanto mais deve ser lido nos casos em que a resposta do aplicativo
//tem parâmatros junto
char trata_dados(){
	
	if(fifo_recepcao[0] == 'P'){
		
		switch(fifo_recepcao[1]){
			case('L'):
				//Libera urna
				OLU = 1; //Ordem de Liberar Urna 
				escreve_serial("ML");
				return 0;
			case('B'):
				//Bloqueia urna
				OLU = 0; //Ordem de Liberar Urna
				escreve_serial("MB");
				return 0;
			case('H'):
				//Atualiza o horario
				escreve_serial("MH");
				return 2;
			case('U'):
				//Envia boletim de urna
				return 0;
			case('E'):
				//Confirma entrada do eleitor
				if (respostaPC == PE){
					respostaPC = OK;
				}
				return 0;
			case('C'):
				//Confirma conclusão do voto
				if (respostaPC == PC){
					respostaPC = OK;
				}
				return 0;
			case('T'):
				//Confirma timeout do eleitor
				if (respostaPC == PT){
					respostaPC = OK;
				}
				return 0;
			case('S'):
				//Confirma envio do nome de senador
				return -1;
			case('G'):
				//Confirma envio do nome de governador
				return -1;
			case('P'):
				//Confirma envio do nome de presidente
				return -1;
			default:
				return 0;
				
		}
	}
	return 0;
}

void solicita_senador(char* codigo){
	
	char mensagem[6];
	mensagem[0] = 'M';
	mensagem[1] = 'S';
	mensagem[2] = 2;
	mensagem[3] = codigo[0];
	mensagem[4] = codigo[1];
	mensagem[5] = '\0';
	respostaPC = PS;
	escreve_serial(mensagem);
	
}

void solicita_governador(char* codigo){
	
	char mensagem[6];
	mensagem[0] = 'M';
	mensagem[1] = 'G';
	mensagem[2] = 2;
	mensagem[3] = codigo[0];
	mensagem[4] = codigo[1];
	mensagem[5] = '\0';
	respostaPC = PG;
	escreve_serial(mensagem);
}

void solicita_presidente(char* codigo){
	
	char mensagem[6];
	mensagem[0] = 'M';
	mensagem[1] = 'P';
	mensagem[2] = 2;
	mensagem[3] = codigo[0];
	mensagem[4] = codigo[1];
	mensagem[5] = '\0';
	respostaPC = PP;
	escreve_serial(mensagem);
}




