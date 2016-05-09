#include <utils.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BOUNDARY_PREFIX "--boundaryTPSO1IPC"

static bool seeded = false;

static int rand_max_digits = -1;

int getrnd(int min, int max) {
	if (!seeded) {
		srand(time(NULL));
		seeded = true;
	}
	return rand() % (max-min) + min;
}

char * gen_boundary() {

	char *boundary;
	size_t length;

	if (rand_max_digits == -1) {
		rand_max_digits = ceilf(log10(RAND_MAX));
	}

	length = strlen(BOUNDARY_PREFIX)+rand_max_digits+2;
	boundary = malloc(length+1);

	length = sprintf(boundary, "%s%d__", BOUNDARY_PREFIX, getrnd((int)pow(10,rand_max_digits-1), RAND_MAX ) );
	boundary[length] = ZERO;

	return boundary;

}

char** split_arguments(char * sentence){

	int index=0, last_begin=0, count=0, last_was_char=no, quotation_open=no;
	char ** result;
	

	while(sentence[index] != '\0') {
		if (sentence[index] == ' '){
    		if (last_was_char==yes && !quotation_open){
        		count++;
	
        		char * aux_str = malloc(sizeof(char)*(index-last_begin+1));

				memset(aux_str, '\0', (index-last_begin+1));
        		strncpy(aux_str, sentence+last_begin, index-last_begin);
        		aux_str[index-last_begin] = '\0';
        		
        		result = add(aux_str, result, count);
    		}
    		last_was_char=no;
		}
		else{
				if(!quotation_open && last_was_char==no){ last_begin = index;}
		    	last_was_char=yes;
		    	if (sentence[index] == '\"') 
				quotation_open = !quotation_open;
		    }

    	index++;
	}

	if(last_was_char==yes){
		if(quotation_open)
			return NULL;
		else{
			count++;
			char * aux_str = malloc(sizeof(char)*(index-last_begin+1));
        	strncpy(aux_str, sentence + last_begin, index-last_begin);
        	aux_str[index-last_begin] = '\0';
        		
        		
        	result = add(aux_str, result, count);
    		
		}
	}
	result= add("\0", result,count+1);
	return result;
}

char ** add(char * str, char ** str_vector, int cant){
	
	char ** aux = malloc(sizeof(char*)*cant);
	int i;
	for(i = 0; i < cant-1 ; i++){
    	aux[i] = str_vector[i];
	}
	aux[cant-1] = str;
	return aux;
}

int count_elements(char ** vector){

	int count = 0;
	while(strcmp(vector[count],"\0"))
		count++;
	return count;
}
