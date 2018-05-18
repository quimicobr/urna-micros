#include<reg52.h>
#include<lcd.h>
#include<teclado.h>
#include<serial.h>
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
char hora;
char minuto;
char segundo;

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

void configura_serial();

	xdata struct {
	
	char nome[20];
	char partido[5];
	char nPartido[3];
	unsigned char nVotos;
	
	}presidente[10], governador[10], senador[10];


//Como vai funcionar: a urna tem que estar em modo standby para receber algo do teclado ou da serial.
//Se ela está em modo bloqueada, ela não pode estar disponível para receber nada do teclado.
//Se ela está em modo


//Inicialização: configurar serial, relógio, LCD e pedir os dados de todos os candidatos possíveis 

void main(){
	char c;
	
	//Configura a porta serial
	configura_serial();
	
	//Configura o relogio
	configura_relogio();
	
	//Inicializa o visor LCD
	LCD_init();

	while(1){
		c = le_teclado();
		escreve_serial("Teste ");
	}
}








