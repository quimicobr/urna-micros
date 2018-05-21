#include<reg52.h>
#include<lcd.h>
#include<teclado.h>
#include<serial.h>
#include<relogio.h>
#include<stdio.h>
#include<string.h>


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

data char fifo_recepcao[30];

sbit BUZZER = P3^3;

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

char len_string(char* string){
	
	for (i = 0; string[i] != 0; i++)
	return i;
	
}

char maior(char arg1, char arg2){
	if (arg1 >arg2) return arg1;
	else return arg2;
}
			
char compara_string(char* string1,char* string2){
	
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
			
void varredura_candidatos(){
	
	char d0, d1;
	char d[2];
	char comprimento, i, inicio_par;
	char nome[20];
	char partido[5];

	for (d0 = '0'; d0 <= '9'; d0++){
		
		d[0] = d0;
		for (d1 = '0'; d1 <= '9'; d1++){
			
			d[1] = d1;
			solicita_senador(d);
			while (respostaPC != OK);
			//Pega o dado do senador e ve se nao é nulo
			//Se não for, armazena no vetor de struct
			comprimento = fifo_recepcao[2];
			
			for (i = 3; fifo_recepcao[i] != 0; i++){
				nome[i-3] = fifo_recepcao[i];
			}
			inicio_par = i + 1;
			for (i = inicio_par; fifo_recepcao[i] != 0; i++){
				nome[i-inicio_par] = fifo_recepcao[i];
			}
			
			if(
		
			
			solicita_governador(d);
			solicita_presidente(d);
			
			
			
		}	
	}
	
	
}
			
	


void main(){
	char c;
	char UB_escrito;
		//Configura a porta serial
	configura_serial();
	
	//Configura o relogio
	configura_relogio();
	
	//Inicializa o visor LCD
	LCD_init();

	escreve_serial("Incixializa");
	
	while(1){

	}
		
}








