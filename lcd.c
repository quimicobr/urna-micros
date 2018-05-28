// Nomes:	Davi Nachtigall Lazzarotto   216628
//				Gustavo Fernandes (230357)			

#include<reg52.h>
#include<lcd.h>

sbit RS =P3^6;
sbit E =P3^7;

void LCD_init(){	
		
 escreve_comando(0x38);
 escreve_comando(0x38);
 escreve_comando(0x0E);
 escreve_comando(0x06);
 escreve_comando(0x01);
 atraso_clear();

}

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
		
void escreve_LCD(char X){
			RS = 1;
			P1 = X;

			E = 1;
			atraso();
			E = 0;
		}

void escreve_comando(unsigned char com){
	
	RS = 0;
	P1 = com;
	E = 1;
	E = 0;
	atraso();
}

void escreve_mensagem(char* mensagem){
	
	char i;
	for(i = 0; mensagem[i] != 0 ; i++){
		escreve_LCD(mensagem[i]);	
	}
	atraso();
	
}

void clear_lcd(){
	escreve_comando(0x01);
	atraso_clear();
}
