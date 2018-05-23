#include <myString.h>

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

void clear_string(char* string){
	
	char i;
	for (i = 0; string[i] != '\0'; i++){
		string[i] = '\0';
	}
	
}

void inicializa_string(char* string, char len){
	
	char i;
	for (i = 0; i<len; i++){
		string[i] = '\0';
	}
	
}