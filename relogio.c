#include<reg52.h>
#include<relogio.h>

// O tempo de estouro do timer 2 com 16 bits é de 35.5555555 ms
// Pra chegar a 18432 ciclos, ele demora 10 ms
// Pra chegar a 46080 ciclos, ele demora 25 ms (1s/40)

//65536 - 46080 = 19456 = 0x4C00

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

void atualiza_relogio() interrupt 5 {
	
	static char count = 0;
	
	count ++;
	
	if (count >= 40){
		count = 0;
		segundos ++;
		if (segundos >= 60){
			segundos = 0;
			minutos ++;
			if (minutos >= 60){
				minutos = 0;
				if (horas >= 24){
					horas = 0;
				}
			}
		}
	}
	
	
}
