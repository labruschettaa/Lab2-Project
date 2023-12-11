#include "betterfunctions.h"

void clean_my_array(char* buffer, int n){
    int i;
    for(i=n;i<2048;i++){
        //printf("Leggo: %c\n", buffer[i]);
        if(buffer[i]=='\0') break;
        buffer[i] = '\0';
    }
}

ENTRY *crea_entry(char *s, int n) {
  ENTRY *e = malloc(sizeof(ENTRY));
  if(e==NULL) fun_termina("errore malloc entry 1");
  e->key = strdup(s); // salva copia di s
  e->data = (int *) malloc(sizeof(int));
  if(e->key==NULL || e->data==NULL)
    fun_termina("errore malloc entry 2");
  *((int *)e->data) = n;
  return e;
}

void distruggi_entry(ENTRY *e)
{
  free(e->key); free(e->data); free(e);
}

char** tokenize(char* str, char* s, int* size){
    int malloc_size = 10, elements = 0;
    char** token_array = malloc(sizeof(char*)*malloc_size);
    if(token_array==NULL) fun_termina("Errore malloc in tokenize");
    char* token = strtok(str, s);
   token_array[elements] = token;
   while( token != NULL ) {
      //printf( "%d HO TOKENIZZATO: %s\n", elements, token );
      token = strtok(NULL, s);
      elements = elements + 1;
      if(elements>=malloc_size){
        malloc_size = malloc_size + 10;
        token_array = realloc(token_array,sizeof(char*)*malloc_size);
        if(token_array==NULL) fun_termina("Errore realloc in tokenize");
      }
      token_array[elements] = token;
   }
   *size = elements;
   return token_array;
}

void modify_my_insert_string(char* str_ninserts, int n){
    str_ninserts[55]= '0' + (int)((n%10000000)/1000000);
    str_ninserts[56] = '0' + (int)((n%1000000)/100000);
    str_ninserts[57] = '0' + (int)((n%100000)/10000);
    str_ninserts[58] = '0' + (int)((n%10000)/1000);
    str_ninserts[59] = '0' + (int)((n%1000)/100);
    str_ninserts[60] = '0' + (int)((n%100)/10);
    str_ninserts[61] = '0' + (n%10); 
}

void free_arrays(char** array, int size){
  int i;
  for(i=0;i<size;i++){
    free(array[i]);
  }
  free(array);
}

void free_array_of_arrays(char*** array, int size){
  int i;
  for(i=0;i<size;i++){
    free(array[i]);
  }
  free(array);
}