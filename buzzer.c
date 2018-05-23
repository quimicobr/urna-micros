#include<buzzer.h>
#include<reg52.h>

sbit BUZZER = P3^3;

void sound_buzzer_teclado(){
	
	//Nota medida: C#7 - 2217.46 Hz = 831 ciclos do timer
	//Valor do timer: FCC1
	short int counter = 0;
	TR0 = 0;
	TF0 = 0; //limpa a flag de estouro
	TMOD &= 0xF0;
	TMOD |= 0x01; //timer de 16 bits 
	TH0 = 0xFC;
	TL0 = 0xC1;
	TR0 = 1; //liga o timer
	BUZZER = 0;
	while(counter < 50){
		counter ++;
		while(TF0 == 0);
		TH0 = 0xFC;
		TL0 = 0xC1;
		TF0 = 0;
		BUZZER = !BUZZER;
		counter ++;
		
	}
	
void sound_buzzer_voto(){
	
	//Notas: C#7 e C7 
	//C7 - 2093Hz = 881 ciclos do timer - 
	//C#7	- 2217.46 Hz = 831 ciclos do timer
	//Valor do timer: FCC1
	short int counter = 0;
	TR0 = 0;
	TF0 = 0; //limpa a flag de estouro
	TMOD &= 0xF0;
	TMOD |= 0x01; //timer de 16 bits 
	TH0 = 0xFC;
	TL0 = 0xC1;
	TR0 = 1; //liga o timer
	BUZZER = 0;
	while(counter < 50){
		counter ++;
		while(TF0 == 0);
		TH0 = 0xFC;
		TL0 = 0xC1;
		TF0 = 0;
		BUZZER = !BUZZER;
		counter ++;
		
	}
	
	
	
}