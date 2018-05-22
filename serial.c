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
	if(RI == 1){
		RI = 0;
		fifo_recepcao[pos] = SBUF;
		pos++;
		
		if(pos == l_final || lendo_par){
			//Aqui existem duas opções: se nao ha parametros a serem lidos, l_final continuara a mesmo
			//Se ha, l_final mudara, e dai o codigo tem que pular diretamente para ca quando ler o proximo byte
			if (!lendo_par){
				l_final += trata_dados();
				lendo_par = 1;
			}
			
			if (pos == l_final){
				trata_dados();
				clear_FIFO();
				pos = 0;
				lendo_par = 0;
			}			
		}
	}
}

//Trata dados deve ler toda a mensagem mandada pela serial
//Deve retornar o quanto mais deve ser lido nos casos em que a resposta do aplicativo
//tem parâmatros junto
char trata_dados(){
	
	//Libera urna
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'L'){
		OLU = 1; //Ordem de Liberar Urna 
		escreve_serial("ML");
		return 0;
	}
	
	//Bloqueia urna
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'B'){
		OLU = 0; //Ordem de Liberar Urna
		escreve_serial("MB");
		return 0;
	}
	
	//Atualiza o horario
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'H'){
		
		minuto = fifo_recepcao[3];
		hora = fifo_recepcao[2];
		escreve_serial("MH");
		return 2;
		
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'U'){
		//Envia boletim de urna
		return 0;
		
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'E'){
		//Confirma entrada do eleitor
		if (respostaPC == PE){
			respostaPC = OK;
		}
		return 0;
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'C'){	
		//Confirma conclusão do voto
		if (respostaPC == PC){
			respostaPC = OK;
		}
		return 0;
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'T'){
		//Confirma timeout do eleitor
		if (respostaPC == PT){
			respostaPC = OK;
		}
		return 0;
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'S' && fifo_recepcao[2] != '\0'){
		//Confirma envio do nome de senador e guarda a mensagem na fifo
		char l = fifo_recepcao[2];
		
		if (respostaPC == PS){
			copia_string(pacote, fifo_recepcao);
			respostaPC = OK;
		}
		return 1;
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'G'){
		//Confirma envio do nome de governador
		if (respostaPC == PG){
			copia_string(pacote, fifo_recepcao);
			respostaPC = OK;
		}
		return 1;
	}
	
	if (fifo_recepcao[0] == 'P' && fifo_recepcao[1] == 'P'){
		//Confirma envio do nome de presidente
		if (respostaPC == PP){
			copia_string(pacote, fifo_recepcao);
			respostaPC = OK;
		}
		return 1;
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




