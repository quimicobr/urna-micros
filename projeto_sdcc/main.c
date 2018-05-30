#include<8052.h>

//Gestão de timers:
//TIMER0: uso geral (atraso,...)
//TIMER1: exclusivo da serial
//TIMER2: exclusivo do relógio

////////////////////////////////////////////////////////////
//Arquivos .c incluidos no main para ser compilados no SDCC:
////////////////////////////////////////////////////////////
//Primeiro: declaração de variáveis
//Após: declaração de funções

//VARIAVEIS


//lcd.c
#define RS P3_6 //sbit for sdcc
#define E  P3_7 //sbit for sdcc

//teclado.c
#define C1  P0_0 //coluna 1  //sbit for sdcc
#define C2  P0_1 //sbit for sdcc
#define C3  P0_2 //sbit for sdcc
//sbit L1 = P0^4; 	//linha 1
//sbit L2 = P0^5;
//sbit L3 = P0^6;
//sbit L4 = P0^7;

//serial.c
#define NAO 0
#define LE_N 1
#define LE_PARAMETRO 2

__idata char fifo_recepcao[30];

//buzzer.c
#define BUZZER  P3_3 //sbit for sdcc

//codigo_main.c
char estado;
#define NORMAL 0
#define BLOQUEADA 1
#define AUTONOMA 2 

//Variaveis onde estará registrada a hora
char hora = 0;
char minuto = 0;
char segundo = 0;
char OLU = 0;

// -- Flag que indica se o microcontrolador está esperando uma resposta do PC
//No  momento que o microcontrolador envia um comando que precisa de resposta
//essa flag é setada para 0. No momento que o computador responde, a flag
//é setada para 1.
char respostaPC;
#define OK 0
#define PE 1
#define PC 2 
#define PT 3
#define PS 4
#define PG 5 
#define PP 6 

//idata char pacote[30];
__xdata char pacote[40];
#define n_sen  4
#define n_gov  4
#define n_pres  8

__xdata struct {
	
	char nome[20];
	char partido[5];
	char nPartido[3];
	unsigned char nVotos;
	
	}presidente[n_pres + 2], governador[n_gov + 2], senador[n_sen + 2];


const long codigo_eleitor [] = {262659, 206603, 244444, 216628, 262645,
			208221,
			262668,
			230357,
			262672,
			143293,
			245562,
			262686,
			264768,
			161948,
			262649,
			231423,
			230340,
			195149,
			231023,
			231035,
			221689,
			220149,
			264770,
			216625,
			262670,
			180669,
			262688,
			186103,
			230230,
			219839,
			243704,
			243716,
			275628,
			262543,
			275632,
			275631,
			275629,
			221433,
			262542,
			275633,
			218776,
			216149,
			262539,
			207273,
			259797 };

char c;

char shora[3], sminuto[3];
char segundo_old = 0, minuto_old = 0, contEleitor=0, contPC=0;
char UA = 0; //urna autonoma
char estado = 0;
unsigned short cartao; //tem que ser long, mas a memoria estoura
char urna_ativa = 1;

char d[7];

			
//FUNCOES
////////////////////////////////////////////////////////////
//lcd.c
////////////////////////////////////////////////////////////

void atraso(){	
	
		TR0 = 0;
		TF0 = 0; //limpa a flag de estouro
		TMOD &= 0xF0;
		TMOD |= 0x02; //timer de 8 bits com recarga automática	
	//22,184 Mhz / 12 = 1,848 MHz (fTimer) -> Ttimer = 0,541 us
	// 40 us -> 74 ciclos - 256 - 74 ciclos = 182 -> valor a ser colocado no registrador
		TL0 = 182;
		TR0 = 1; //liga o timer
		while(TF0 == 0);
		TR0 = 0;
}
void atraso_clear(){	
	
		TR0 = 0;
		TF0 = 0; //limpa a flag de estouro
		TMOD &= 0xF0;
		TMOD |= 0x01;
	 //22,184 Mhz / 12 = 1,848 MHz (fTimer) -> Ttimer = 0,541 us
	 // 1650 us -> 3051 ciclos = 62485 (F415) -> valor a ser colocado no registrador
		TH0 = 0xF4;
		TL0 = 0x15;
		TR0 = 1; //liga o timer
		while(TF0 == 0);
		TR0 = 0;
}				

void escreve_comando(unsigned char com){
	
	RS = 0;
	P1 = com;
	E = 1;
	E = 0;
	atraso();
}


void LCD_init(){	
		
 escreve_comando(0x38);
 escreve_comando(0x38);
 escreve_comando(0x0E);
 escreve_comando(0x06);
 escreve_comando(0x01);
 atraso_clear();

}
		
void escreve_LCD(char X){
			RS = 1;
			P1 = X;

			E = 1;
			atraso();
			E = 0;
		}


void escreve_mensagem(char* mensagem){
	
	char i;
	atraso();
	for(i = 0; mensagem[i] != 0 ; i++){
		escreve_LCD(mensagem[i]);	
	}
	atraso();
	
}

void clear_lcd(){
	atraso();
	escreve_comando(0x01);
	atraso_clear();
}

////////////////////////////////////////////////////////////
//teclado.c
////////////////////////////////////////////////////////////


void atraso_1m(){
	
	TR0 = 0;
	TF0 = 0; //limpa a flag de estouro	
	TMOD &= 0xF0;
	TMOD |= 0x02;
	//22,184 Mhz / 12 = 1,848 MHz (fTimer) -> Ttimer = 0,541 us
	// 1 ms -> 1844 ciclos -> 65536 - 1844 ciclos = 63692 -> valor a ser colocado no registrador
	TH0 = 0xF8;	
	TL0 = 0xCC;
	TR0 = 1; //liga o timer
	while(TF0 == 0);

}

char debouncing(char tecla){
	
	char i = 0;
	for (i = 0; i<=8; i++){
			
		if (P0 != tecla){
			return 0;
		}
		atraso_1m();
		
	}
	return 1;

}

char varredura(){
	C1 = 0;
	C2 = 1;
	C3 = 1;
	switch(P0){
		case 0x7E:		//L1
			if(debouncing(0x7E))
				return '1';
		case 0xBE:		//L2
			if(debouncing(0xBE))
				return '4';
		case 0xDE:		//L3
			if(debouncing(0xDE))
				return '7';
		case 0xEE:		//L4
			if(debouncing(0xEE))
				return '*';
	}
	
	C1 = 1;
	C2 = 0;
	C3 = 1;
	switch(P0){
		case 0x7D:		//L1
			if(debouncing(0x7D))
				return '2';
		case 0xBD:		//L2
			if(debouncing(0xBD))
				return '5';
		case 0xDD:		//L3
			if(debouncing(0xDD))
				return '8';
		case 0xED:		//L4
			if(debouncing(0xED))
				return '0';
	}
	
	C1 = 1;
	C2 = 1;
	C3 = 0;
	switch(P0){
		case 0x7B:		//L1
			if(debouncing(0x7B))
				return '3';
		case 0xBB:		//L2
			if(debouncing(0xBB))
				return '6';
		case 0xDB:		//L3
			if(debouncing(0xDB))
				return '9';
		case 0xEB:		//L4
			if(debouncing(0xEB))
				return '#';
		}
	
	return '\0';
}


char le_teclado(){
	
	static char tecla;
	char tecla_atual;
	tecla_atual = varredura();
		
		if(tecla_atual != tecla){
			tecla = tecla_atual;
		//	escreve_LCD(tecla);
		//	sound_buzzer_teclado();
			return tecla;
		}
	return '\0';
}

////////////////////////////////////////////////////////////
//myString.c
////////////////////////////////////////////////////////////

char len_string(char* string){
	
	char i;
	for (i = 0; string[i] != '\0'; i++){}
	return i;
	
}

char maior(char arg1, char arg2){
	if (arg1 >arg2) return arg1;
	else return arg2;
}
			
char compara_string(char* string1,char* string2){
	
	//strings iguais retorna 1
	//strings diferentes retorna 0
	
	char i, out = 1;
	char lim;
	lim = maior(len_string(string1), len_string(string2));
	
	for (i = 0; i< lim; i++){
		
		if(string1[i] != string2[i]){
			return 0;
		}
	}
	return 1;
}	

void copia_string(char* alvo, char* valor){
	
	char i;
	for (i = 0; valor[i] != '\0'; i++) alvo[i] = valor[i];

}

void number_to_char(char* out, char in){
	
	if (in < 10){
		out[0] = 0;
		out[1] = in +48;
	}
	else{
		out[0] = in%10 + 48;
		out[1] = in - (in%10)*10 + 48;
	}
}

void inicializa_string(char* string, char len){
	
	char i;
	for (i = 0; i<len; i++){
		string[i] = '\0';
	}
	
}


void clear_string(char* string){
	
	char i;
	for (i = 0; string[i] != '\0'; i++){
		string[i] = '\0';
	}
	
}

////////////////////////////////////////////////////////////
//serial.c
////////////////////////////////////////////////////////////

void escreve_serial(char* mensagem){
char i;
	for(i = 0; mensagem[i] != 0 ; i++){
		SBUF = mensagem[i];
		while(!TI);
		TI=0;
	}
}


/*
	__xdata char n[2];	
	long ir =0;
	
	
	n[0] = 0;
	n[1] = 5;
	
	
	escreve_serial("MU");
	
	SBUF = 0x00;
	while(!TI);
	TI=0;
	
	SBUF = 0x05;
	while(!TI);
	TI=0;
		
	escreve_serial("teste");
	*/



void envia_boletim(){
	
	__xdata char i;
	//__xdata char votos[3];
	__xdata char ser[180];
	__xdata char len = 0;
	__xdata unsigned char pos = 0;

	
	inicializa_string(ser, 180);
	
	ser[0] = 'S';
	ser[1] = '{';
	pos += 2;
	
	for (i = 0; i<n_sen + 2; i++){		
		if(senador[i].nPartido[0] != 0){
			ser[pos] = '(';
			ser[pos + 1] = senador[i].nPartido[0];
			ser[pos + 2] = senador[i].nPartido[1];
			ser[pos + 3] = ',';
			//number_to_char(votos,senador[i].nVotos);
			ser[pos + 4] = '0';
			ser[pos + 5] = '0';
			ser[pos + 6] = ')';
			pos += 7;
		}
	}
	
	ser[pos] = '}';
	pos++;
	
	ser[pos] = 'G';
	ser[pos+1] = '{';
	pos += 2;
	
	for (i = 0; i<n_gov + 2; i++){		
		if(governador[i].nPartido[0] != 0){
			ser[pos] = '(';
			ser[pos + 1] = governador[i].nPartido[0];
			ser[pos + 2] = governador[i].nPartido[1];
			ser[pos + 3] = ',';
			//number_to_char(votos,governador[i].nVotos);
			ser[pos + 4] = '0';
			ser[pos + 5] = '0';
			ser[pos + 6] = ')';
			pos += 7;
		}
	}
	
	ser[pos] = '}';
	pos++;
	
	
	
	
	ser[pos] = 'P';
	ser[pos+1] = '{';
	pos += 2;
	
	for (i = 0; i<n_pres + 2; i++){		
		if(presidente[i].nPartido[0] != 0){
			ser[pos] = '(';
			ser[pos + 1] = presidente[i].nPartido[0];
			ser[pos + 2] = presidente[i].nPartido[1];
			ser[pos + 3] = ',';
			//number_to_char(votos,presidente[i].nVotos);
			ser[pos + 4] = '0';
			ser[pos + 5] = '0';
			ser[pos + 6] = ')';
			pos += 7;
		}
	}
	
	ser[pos] = '}';
	pos++;
	
	escreve_serial("MU");
	
	//O programa dele entede 128 como -1; so podemos enviar 127 ou 256 bytes
	
	//diz q vai enviar 256 bytes
	//SBUF = (pos>127);
	SBUF = 1;
	while(!TI);
	TI=0;
	
	//SBUF = pos-127*(pos>127);
	SBUF = 0;
	while(!TI);
	TI=0;
	
	//envia 163 bytes (pos)
	escreve_serial(ser);
	
	escreve_serial("}");
	
	//envia 256 - 163 (256-pos) bytes vazios
	
	for(i=0; i<(256-pos); i++){
		SBUF = 0;
		while(!TI);
		TI=0;
	}
		
	clear_lcd();
	escreve_LCD(pos);
	}


/*
escreve_serial("} Governador: {");
		
		if(compara_string(governador[i].nome, "")){
			
			escreve_serial("(");
			escreve_serial(governador[i].nome);
			escreve_serial(",");
			number_to_char(votos,governador[i].nVotos);
			escreve_serial(votos);
			escreve_serial(") ");
			
		}
		
		escreve_serial("} Presidente: {");
		
		if(compara_string(presidente[i].nome, "")){
			
			escreve_serial("(");
			escreve_serial(presidente[i].nome);
			escreve_serial(",");
			number_to_char(votos,presidente[i].nVotos);
			escreve_serial(votos);
			escreve_serial(") ");
			
		}
		
		//n[0] = 0;
		//n[1] = 3;
*/
	
	

void clear_FIFO(){
	char i;
	for (i = 0; fifo_recepcao[i] != '\0' && i<30; i++) fifo_recepcao[i] = '\0';
	
}

void le_parametros(){
	if(fifo_recepcao[0] == 'P'){
		switch(fifo_recepcao[1]){
			case('H'):
				hora = fifo_recepcao[2];
				minuto = fifo_recepcao[3];
				escreve_serial("MH");
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
				//escreve_serial("MH");
				return 2;
			case('U'):
				envia_boletim();
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

void trata_interrupcao_serial() __interrupt (4) {	
	static char pos = 0, l_final = 2, lendo_par = 0;
	signed char add;
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

////////////////////////////////////////////////////////////
//relogio.c
////////////////////////////////////////////////////////////

void configura_relogio(){
	T2CON = 0;
	TL2 = 0;
	TH2 = 0x4C;
	RCAP2L = 0;
	RCAP2H = 0x4C;
	TR2 = 1;
	
	EA = 1; //habilita as interrrupcoes do chip
	ET2 = 1;	//habilita interrupcao da serial
}

void atualiza_relogio() __interrupt(5) {
	
	static char count = 0;
	TF2 = 0;

	count ++;
	
	if (count >= 40){
		count = 0;
		segundo ++;
		if (segundo >= 60){
			segundo = 0;
			minuto ++;
			if (minuto >= 60){
				minuto = 0;
				if (hora >= 24){
					hora = 0;
				}
			}
		}
	}
	
	
}


////////////////////////////////////////////////////////////
//buzzer.c
////////////////////////////////////////////////////////////

void sound_buzzer_teclado(){
	
	//Nota medida: C#7 - 2217.46 Hz = 831 ciclos do timer
	//Valor do timer: FCC1
	short int i = 0;
	TR0 = 0;
	TF0 = 0; //limpa a flag de estouro
	TMOD &= 0xF0;
	TMOD |= 0x01; //timer de 16 bits 
	TH0 = 0xFC;
	TL0 = 0xC1;
	TR0 = 1; //liga o timer
	BUZZER = 0;
	for(i = 0;i < 100; i++){
		while(TF0 == 0);
		TH0 = 0xFC;
		TL0 = 0xC1;
		TF0 = 0;
		BUZZER = !BUZZER;	
	}
}
	
void sound_buzzer_voto(){
	
	//Notas: C#7 e C7 
	//C7 - 2093Hz = 881 ciclos do timer - FC8F
	//C#7	- 2217.46 Hz = 831 ciclos do timer - FCC1

	short int i;
	char n;
	TR0 = 0;
	TF0 = 0; //limpa a flag de estouro
	TMOD &= 0xF0;
	TMOD |= 0x01; //timer de 16 bits 
	TH0 = 0xFC;
	TL0 = 0xC1;
	TR0 = 1; //liga o timer
	BUZZER = 0;
	for (n = 0; n<4; n++){
		
		for (i = 0; i < 300; i++){		
			while(TF0 == 0);
			TH0 = 0xFC;
			TL0 = 0xC1;
			TF0 = 0;
			BUZZER = !BUZZER;
		}
		
		for (i = 0; i < 300; i++){		
			while(TF0 == 0);
			TH0 = 0xFC;
			TL0 = 0x8F;
			TF0 = 0;
			BUZZER = !BUZZER;
		}
	}
}

////////////////////////////////////////////////////////////
//codigo_main.c
////////////////////////////////////////////////////////////

void configura_serial();

void clear_pacote(){
	
	char i;
	for (i = 0; pacote[i] != '\0' && i<30; i++){
		pacote[i] = '\0';
	}
		
}


void varredura_candidatos(){

	char d0, d1;
	char d[3];
	char i, inicio_par;
	char nome[20];
	char partido[5];
	char pos_sen = 0, pos_gov = 0, pos_pres = 0;
	d[2] = '\0';
	
	for (d0 = 0; d0 <= 9; d0++){
		d[0] = d0 + '0';
		
		for (d1 = 0; d1 <= 9; d1++){
			
			d[1] = d1 + '0';
			
			// ---------------------------
			solicita_senador(d);
			
			while (respostaPC != OK);
			//Pega o dado do senador e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			
			for (i = 0; (pacote[i+3] != 0) && i<20; i++){
				nome[i] = pacote[i+3];
			}
			inicio_par = i + 1; 
			for (i = 0; (pacote[i+inicio_par] != 0) && i<5; i++){
				partido[i] = pacote[i+inicio_par];
			}
			
			
			if((!compara_string(nome,"Voto NULO")) && (!compara_string(nome,"Voto BRANCO"))){
				copia_string(senador[pos_sen].nome, nome);
				copia_string(senador[pos_sen].partido, partido);
				copia_string(senador[pos_sen].nPartido, d);
				pos_sen++;
			}
						
			clear_string(pacote);
			clear_string(nome);
			clear_string(partido);
			
			
			
			// ---------------------------
			solicita_governador(d);
			
			while (respostaPC != OK);
			//Pega o dado do senador e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			
			for (i = 0; (pacote[i+3] != 0) && i<20; i++){
				nome[i] = pacote[i+3];
			}
			inicio_par = i + 1; 
			for (i = 0; (pacote[i+inicio_par] != 0) && i<5; i++){
				partido[i] = pacote[i+inicio_par];
			}
			
			
			if((!compara_string(nome,"Voto NULO")) && (!compara_string(nome,"Voto BRANCO"))){
				copia_string(governador[pos_gov].nome, nome);
				copia_string(governador[pos_gov].partido, partido);
				copia_string(governador[pos_gov].nPartido, d);
				pos_gov++;
			}
						
			clear_string(pacote);
			clear_string(nome);
			clear_string(partido);
			
			// ---------------------------
			solicita_presidente(d);
			while (respostaPC != OK);
			
			//Pega o dado do presidente e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			
			for (i = 0; (pacote[i+3] != 0) && i<20; i++){
				nome[i] = pacote[i+3];
			}
			inicio_par = i + 1; 
			for (i = 0; (pacote[i+inicio_par] != 0) && i<5; i++){
				partido[i] = pacote[i+inicio_par];
			}
			
			if((!compara_string(nome,"Voto NULO")) && (!compara_string(nome,"Voto BRANCO"))){
				copia_string(presidente[pos_pres].nome, nome);
				copia_string(presidente[pos_pres].partido, partido);
				copia_string(presidente[pos_pres].nPartido, d);
				pos_pres++;
			}
						
			clear_string(pacote);
			clear_string(nome);
			clear_string(partido);
		}	
	}
}



void inicializa_structs(){
	
	char i;
	for(i = 0; i<n_sen; i++){
		
		inicializa_string(senador[i].nome, 20);
		inicializa_string(senador[i].partido, 5);
		inicializa_string(senador[i].nPartido, 3);
		senador[i].nVotos = 0;
		
	}
	
	for(i = 0; i<n_gov; i++){

		inicializa_string(governador[i].nome, 20);
		inicializa_string(governador[i].partido, 5);
		inicializa_string(governador[i].nPartido, 3);
		governador[i].nVotos = 0;
		
	}
	
	for(i = 0; i<n_pres; i++){
		
		inicializa_string(presidente[i].nome, 20);
		inicializa_string(presidente[i].partido, 5);
		inicializa_string(presidente[i].nPartido, 3);
		presidente[i].nVotos = 0;
		
	}
	
	copia_string(senador[n_sen].nome, "NULO");
	copia_string(senador[n_sen].partido, "NOP");
	copia_string(senador[n_sen].nPartido, "99");
	senador[n_sen].nVotos = 0;
	
	copia_string(governador[n_gov].nome, "NULO");
	copia_string(governador[n_gov].partido, "NOP");
	copia_string(governador[n_gov].nPartido, "99");
	governador[n_gov].nVotos = 0;
	
	copia_string(presidente[n_pres].nome, "NULO");
	copia_string(presidente[n_pres].partido, "NOP");
	copia_string(presidente[n_pres].nPartido, "99");
	presidente[n_pres].nVotos = 0;
	
	copia_string(senador[n_sen + 1].nome, "BRANCO");
	copia_string(senador[n_sen + 1].partido, "NOP");
	copia_string(senador[n_sen + 1].nPartido, "00");
	senador[n_sen + 1].nVotos = 0;
	
	copia_string(governador[n_gov + 1].nome, "BRANCO");
	copia_string(governador[n_gov + 1].partido, "NOP");
	copia_string(governador[n_gov + 1].nPartido, "00");
	governador[n_gov + 1].nVotos = 0;
	
	copia_string(presidente[n_pres + 1].nome, "BRANCO");
	copia_string(presidente[n_pres + 1].partido, "NOP");
	copia_string(presidente[n_pres + 1].nPartido, "00");
	presidente[n_pres + 1].nVotos = 0;
}


char le_numero(char n){
	static char i=0;
	char c;
	c = le_teclado();
		if(c != 0){
			if(i<n){
			escreve_LCD(c);
			d[i] = c;
			i++;
			}else{
						i = 0;
						return 1; //para 
					 }
		}
	return 0; // continua
}


void transicao_estado(char et){
	switch(et){
		case 0: clear_lcd();
						escreve_mensagem("N de cartao?");
						clear_string(d);
						estado = 0;
						break;
		
		case 1: clear_lcd();
						escreve_mensagem("N de senador?");
						clear_string(d);
						estado = 1;
						break;
		
		case 2: clear_lcd();
						escreve_mensagem("N de govern.?");
						clear_string(d);
						estado = 2;
						break;
		
		case 3: clear_lcd();
						escreve_mensagem("N de pres.?");
						clear_string(d);
						estado = 3;
						break;
						
		case 4: clear_lcd();
						escreve_mensagem("FIM!!!");
						clear_string(d);
						estado = 4;
						break;
		
		default: break;
	}
}



void main(){

	short int i;

	configura_serial();
	//configura_relogio();
	
	//Inicializa o visor LCD
	LCD_init();
	clear_lcd();
	clear_string(d);
	
	//escreve_mensagem("Inicializando...");
	inicializa_structs();
	varredura_candidatos();
	
	
	/*
	
	for(i=0; i<n_sen + 2; i++){
		if (senador[i].nPartido[0] != 0)
			solicita_senador(senador[i].nPartido);
		while(le_teclado() == 0);
	}
	
	for(i=0; i<n_gov + 2; i++){
		if (governador[i].nPartido[0] != 0)
			solicita_governador(governador[i].nPartido);
		while(le_teclado() == 0);
	}
	
	for(i=0; i<n_pres + 2; i++){
		if (presidente[i].nPartido[0] != 0)
			solicita_presidente(presidente[i].nPartido);
		while(le_teclado() == 0);
	}
	
	*/

	//clear_pacote();
	//solicita_senador("13");


	while(1){/*
	
		//envia sinal de vida para o PC todos os segundos
		if(segundo_old != segundo){
			escreve_serial("MO");
			segundo_old = segundo;
		}
		
		//bloqueia urna
		if((hora>=17 || hora<8 || OLU == 0) && urna_ativa != 0){
			clear_lcd();
			escreve_mensagem("Urna Bloqueada");
			urna_ativa = 0;
		}
		
		//libera a urna 
		if(hora<17 && hora >= 8 && OLU == 1 && urna_ativa != 1){
			urna_ativa = 1;
			transicao_estado(0);
		}
		
		if(urna_ativa){
			//leitura do cartao
			
		switch(estado){
			case 0: if(le_numero(2)){ //tem que ser 6, mas a memoria estoura				
							//cartao = (d[0]- '0')*100000 + (d[1]- '0')*10000 + (d[2]- '0')*1000 + (d[3]- '0')*100 + (d[4]- '0')*10 + (d[5]- '0')*1;  					
							cartao = (d[0]- '0')*10 + (d[1]- '0')*1;
							transicao_estado(1);
							}		
						  break;
			
			case 1: if(le_numero(2)){ 
							solicita_senador(d);
							transicao_estado(2);
							}
							break;
							
			case 2: if(le_numero(2)){ 
							solicita_governador(d);
							transicao_estado(3);
							}
							break;
							
			case 3: if(le_numero(2)){ 
							solicita_presidente(d);
							transicao_estado(4);
							}
							break;	
							
			case 4:  if(le_teclado())
								transicao_estado(0);
							 break;
			
			default: break;
								
		}
				
		}
	*/}
}

//		c = le_teclado();
//		if(c != 0){		
//			if(c == '0'){
//			shora[0] = (hora / 10) + '0';
//			shora[1] = (hora % 10) + '0';
//					
//			sminuto[0] = (minuto / 10) + '0';
//			sminuto[1] = (minuto % 10) + '0';
//					
//			clear_lcd();
//			escreve_mensagem(shora);
//			escreve_mensagem(":");
//			escreve_mensagem(sminuto);
//			
//			sound_buzzer_teclado();	
//			}
//			if(c == '*')
//					clear_lcd();
//			if(c == '#')
//					escreve_LCD(OLU + '0');



	
//		c = le_teclado();
//		if(c != 0){		
//		shora[0] = (hora / 10) + '0';
//		shora[1] = (hora % 10) + '0';
//				
//		sminuto[0] = (minuto / 10) + '0';
//		sminuto[1] = (minuto % 10) + '0';
//				
//		clear_lcd();
//		escreve_mensagem(shora);
//		escreve_mensagem(":");
//		escreve_mensagem(sminuto);
//		
//		sound_buzzer_teclado();	
//		if(c == '*')
//				clear_lcd();
//	}





		//admistração dos timeouts	
//		if(resposta_PC == OK){
//			contPC = 0;
//			UA = 0;
//		}
//		if(minuto_old != minuto){
//			contEleitor++;
//			contPC++;
//				if(contPC >=3)
//					UA = 1;
//			minuto_old = minuto;
//		}
			


	/*
	while(1){

		c = le_teclado;
		
			if(c != 0){
				
				//escreve no lcd o horario
				
				
				
				shora[1] = (hora / 10) + '0';
				shora[0] = (hora % 10) + '0';
				shora[2] = 0;
				
				sminuto[1] = (minuto / 10) + '0';
				sminuto[0] = (minuto % 10) + '0';
				sminuto[2] = 0;
				
				clear_lcd();
				escreve_mensagem(shora);
				escreve_mensagem(":");
				escreve_mensagem(sminuto);
					*/
//					escreve_LCD(c);
					
//			}
			
//		if(((hora>=17 || hora < 8) || !OLU) && estado != 0) 
//			transicao_estado(0); //bloqueia a urna

//		switch(estado){
//		
//			case 0: //urna bloqueada
//				if(OLU && hora<17 && hora>= 8)
//					transicao_estado(1);	
//				break;

//			case 1: // urna em stand-by (lê os 6 digitos do numero de cartão)				
//				
//			
//			case 2: // administrando votacao para senador (lê 2 digitos)


//			case 3: // administrando votacao para governador (lê 2 digitos)


//			case 4: // administrando votacao para presidente (lê 2 digitos)
//		
//			default:
//				break;
//		}
//	}	
//}


//void estado_0(){
//		if(OLU && hora<17 && hora>= 8)
//				transicao_estado(1);	
//}

//void estado_1(){
//		if(((hora>=17 || hora < 8) || !OLU) && estado != 0) 
//			transicao_estado(0); //bloqueia a urna
//}

//void transicao_estado(char es){
//	switch(es){
//		case 0: //urna bloqueada
//			clear_lcd();
//			escreve_mensagem("Urna Bloqueada");
//			estado = 0;
//			break;
//		case 1: //stand-by
//			clear_lcd();
//			escreve_mensagem("Digite o seu n de cartão:");
//			estado = 1;
//			break;
//	}
//}


//void transicao_estado(char et){
//	
//	switch(et){
//	
//	case 0: //urna bloqueada
//	clear_lcd();
//	escreve_mensagem("Urna Bloqueada");
//	estado = 0;
//	break;

//	case 1: //stand-by
//	clear_lcd();
//	escreve_mensagem("Digite o seu n de cartão:");
//	break;

//	case 2: //votacao senador
//	clear_lcd();
//	escreve_mensagem("Digite o seu n de senador:");
//	break;
//	
//	case 3: //votacao governador
//	clear_lcd();
//	escreve_mensagem("Digite o seu n de governador:");
//	break;

//	case 4: //votacao presidente
//	clear_lcd();
//	escreve_mensagem("Digite o seu n de presidente:");
//	break;

//	}

//estado = et;

//}




/*

	c = le_teclado();
		if(c != 0){	
		escreve_LCD(c);
		sound_buzzer_teclado();	
		if(c == '*')
				clear_lcd();


char estado, FlagUrnaAutonoma
short int cartao, ContTimeOutPc, ContTimeOutEleitor;
char cartao_str[7], i;








while(1){

	if(((hora>=17 || hora < 8) || !OLE) && estado != 0) 
		transicao_estado(0); //bloqueia a urna

	switch(estado){
	
	case 0: //urna bloqueada
		if(OLE && hora<17 && hora>= 8)
			transicao_estado(1);	
		break;

	case 1: // urna em stand-by (lê os 6 digitos do numero de cartão)
		estado_1();
				
		
	case 2: // administrando votacao para senador (lê 2 digitos)


	case 3: // administrando votacao para governador (lê 2 digitos)


	case 4: // administrando votacao para presidente (lê 2 digitos)



	}
}




char le_digito(char * d, n){
//entrada: string a retornar os digitos, numero de digitos desejados
//saida: 0, não terminou, fica no estado; 1, terminou, vai pro proximo estado; -1, cancela tudo e reinicia o estado. 
//entrada e saida: string q armazena os digitos (ponteiro)
static i = 0;
char digito;
digito = le_teclado();
	if(digito != 0){
		if(digito != '#'){
		d[i] = digito;
		i++;
			if(i == n){
			i = 0;		
			return 1; //leitura completa
			}		
		} else {
		i = 0;
		return -1; //cancela a leitura 
		}
	}
return 0; //continua a leitura
}




estado_1(){

}



	



char adquire_digito(char* string, char n_digitos){
//retorna 1 quando termina de ler os n digitos
//retorna 0 se ainda nao terminou de ler
//retorna -1 se o usuario deseja interromper a leitura
//entrada: string a retornar os digitos, numero de digitos desejados
//saida: 0, não terminou, fica no estado; 1, terminou, vai pro proximo estado; -1, cancela tudo e reinicia o estado. 
//entrada e saida: string q armazena os digitos (ponteiro)
static i = 0;
d[i] = le_teclado();
	if(d[i] != 0){
		if(digito != '#'){
			if(i <= n){ //enquanto i<n
			d[i] = digito;
			i++;
			escreve_LCD(d[i]);
			}else if(d[i] == '*'){ //espera a confirmação 
				i = 0;		
				return 1; //leitura completa
				}
			}		
		
		} else {
		i = 0;
		return -1; //cancela a leitura 
		}
	}
return 0; //continua a leitura
}


short int converte_string_int(char* string){
char i;
short int r = 0;
for(i=0; string != 0; i++)
  r = r + (string[i] - '0')*i;
return r
}



*/
