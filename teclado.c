#include<reg52.h>
#include<teclado.h>


sbit C1 = P0^0;	//coluna 1
sbit C2 = P0^1;
sbit C3 = P0^2;
sbit L1 = P0^4; 	//linha 1
sbit L2 = P0^5;
sbit L3 = P0^6;
sbit L4 = P0^7;


char le_teclado(){
static char tecla;
char tecla_atual;
	tecla_atual = varredura();
	if(tecla_atual!= tecla){
	tecla = tecla_atual;
	return tecla;
	}
return '\0';
}

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
