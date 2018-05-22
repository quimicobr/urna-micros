#include<reg52.h>
#include<lcd.h>
#include<teclado.h>
#include<serial.h>
//#include<relogio.h>
#include<myString.h>


//TIMER0: uso geral (atraso,...)
//TIMER1: exclusivo da serial
//TIMER2: exclusivo do relógio



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

idata char pacote[30];

sbit BUZZER = P3^3;

void sound_buzzer(){
	
	short int counter = 0;
	TR0 = 0;
	TF0 = 0; //limpa a flag de estouro
	TMOD &= 0xF0;
	TMOD |= 0x01; //timer de 16 bits 
	//22,184 Mhz / 12 = 1,848 MHz (fTimer) -> Ttimer = 0,541 us
	// 1000 us -> 1844 ciclos - 65536 - 1844 ciclos = 63692 -> 0xF8CC - valor a ser colocado no registrador
	TH0 = 0xF8;
	TL0 = 0xCC;
	TR0 = 1; //liga o timer
	BUZZER = 0;
	while(counter < 500){
		counter ++;
		while(TF0 == 0);
		TH0 = 0xF8;
		TL0 = 0xCC;
		TF0 = 0;
		BUZZER = !BUZZER;
		counter ++;
		
	}
	
	
	
}

void clear_pacote(){
	
	char i;
	for (i = 0; pacote[i] != '\0' && i<30; i++){
		pacote[i] = '\0';
	}
		
	
}

void configura_serial();

	xdata struct {
	
	char nome[20];
	char partido[5];
	char nPartido[3];
	unsigned char nVotos;
	
	}presidente[10], governador[10], senador[10];

short code codigo_eleitor [] = {262659, 206603, 244444, 216628, 
			262645,
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
	
	
//Como vai funcionar: a urna tem que estar em modo standby para receber algo do teclado ou da serial.
//Se ela está em modo bloqueada, ela não pode estar disponível para receber nada do teclado.
//Se ela está em modo


//Inicialização: configurar serial, relógio, LCD e pedir os dados de todos os candidatos possíveis 

void varredura_candidatos(){
	
	char d0, d1;
	char d[3];
	char comprimento, i, inicio_par;
	char nome[20];
	char partido[5];
	char pos_sen = 0, pos_gov = 0, pos_pres = 0;

	for (d0 = '0'; d0 <= '1'; d0++){
		
		d[0] = d0;
		for (d1 = '0'; d1 <= '9'; d1++){
			
			d[1] = d1;
			d[2] = '\0';
			// ---------------------------
			solicita_senador(d);
			while (respostaPC != OK);
			//Pega o dado do senador e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			
			comprimento = pacote[2];
			
			for (i = 0; pacote[i+3] != ' ' ; i++){
				nome[i] = pacote[i+3];
			}
			inicio_par = i + 1; 
			for (i = 0; pacote[i+inicio_par] != '\0'; i++){
				partido[i] = pacote[i+inicio_par];
			}
			
			
			if(!compara_string(nome,"NULO")){
				copia_string(senador[pos_sen].nome, nome);
				copia_string(senador[pos_sen].partido, partido);
				copia_string(senador[pos_sen].nPartido, d);
			}
			clear_string(pacote);
			clear_string(nome);
			clear_string(partido);
			// ---------------------------
			solicita_governador(d);
			
			while (respostaPC != OK);
			//Pega o dado do senador e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			comprimento = pacote[2];
			
			for (i = 0; pacote[i+3] != ' '; i++){
				nome[i] = pacote[i+3];
			}
			inicio_par = i + 1; 
			for (i = 0; pacote[i+inicio_par] != '\0'; i++){
				partido[i] = pacote[i+inicio_par];
			}
			
			clear_pacote();
			
			if(!compara_string(nome,"NULO")){
				copia_string(governador[pos_sen].nome, nome);
				copia_string(governador[pos_sen].partido, partido);
				copia_string(governador[pos_sen].nPartido, d);
			}
			
			// ---------------------------
			solicita_presidente(d);
			
			while (respostaPC != OK);
			//Pega o dado do senador e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			comprimento = pacote[2];
			
			for (i = 0; pacote[i+3] != ' '; i++){
				nome[i] = pacote[i+3];
			}
			inicio_par = i + 1; 
			for (i = 0; pacote[i+inicio_par] != '\0'; i++){
				partido[i] = pacote[i+inicio_par];
			}
			
			clear_pacote();
			
			if(!compara_string(nome,"NULO")){
				copia_string(presidente[pos_sen].nome, nome);
				copia_string(presidente[pos_sen].partido, partido);
				copia_string(presidente[pos_sen].nPartido, d);
			}

		}	
	}
}

void main(){
		//Configura a porta serial
	configura_serial();
	
	//Configura o relogio
//	configura_relogio();
	
	//Inicializa o visor LCD
	LCD_init();

	escreve_serial("Incixializa");
	varredura_candidatos();
	
	
	while(1){

			
		
	}
		
}








