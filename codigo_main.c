#include<reg52.h>
#include<lcd.h>
#include<teclado.h>
#include<serial.h>
#include<relogio.h>
#include<myString.h>
#include<buzzer.h>


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

short code codigo_eleitor [] = {262659, 206603, 244444, 216628, 262645,
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
/*
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

*/

/*

void envia_boletim(){
	
	char i;
	char votos[3];
	
	for (i = 0; i<10; i++){
		
		
		escreve_serial("Senador: {");
		
		if(compara_string(senador[i].nome, "")){
			
			escreve_serial("(");
			escreve_serial(senador[i].nome);
			escreve_serial(",");
			number_to_char(votos,senador[i].nVotos);
			escreve_serial(votos);
			escreve_serial(") ");
			
		}
		
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
		
		escreve_serial("}");
	}
}

*/

/*

void inicializa_structs(){
	
	char i;
	for(i = 0; i<10; i++){
		
		inicializa_string(senador[i].nome, 20);
		inicializa_string(senador[i].partido, 5);
		inicializa_string(senador[i].nPartido, 3);
		senador[i].nVotos = 0;
		
		inicializa_string(governador[i].nome, 20);
		inicializa_string(governador[i].partido, 5);
		inicializa_string(governador[i].nPartido, 3);
		governador[i].nVotos = 0;
		
		inicializa_string(presidente[i].nome, 20);
		inicializa_string(presidente[i].partido, 5);
		inicializa_string(presidente[i].nPartido, 3);
		presidente[i].nVotos = 0;
		
	}
	
}

*/

char d[7];

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




char shora[3], sminuto[3];
//char segundo_old = 0, minuto_old = 0, contEleitor=0, contPC=0;
//char UA = 0; //urna autonoma
char estado = 0;
unsigned short cartao; //tem que ser long, mas a memoria estoura
char urna_ativa = 1;


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



char c;
void main(){

	configura_serial();
//	configura_relogio();
	
	//Inicializa o visor LCD
	LCD_init();
	clear_lcd();
	clear_string(d);
	//escreve_mensagem("Inicializando...");
	
	//inicializa_structs();
	//clear_pacote();
	//solicita_senador("13");
	


	while(1){
		
		//envia sinal de vida para o PC todos os segundos
//		if(segundo_old != segundo){
//			escreve_serial("MO");
//			segundo_old = segundo;
//		}
		
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
							
			case 4:  if(le_teclado)
								transicao_estado(0);
							 break;
			
			default: break;
								
		}
				
		}
	}
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
				
				/*
				
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
