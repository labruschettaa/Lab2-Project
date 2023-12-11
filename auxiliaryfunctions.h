#include "betterfunctions.h"

void clean_my_array(char* buffer, int n);

ENTRY *crea_entry(char *s, int n);

void distruggi_entry(ENTRY *e);

char** tokenize(char* str, char* s, int* size);

void modify_my_insert_string(char* str_ninserts, int n);

void free_arrays(char** pointer_tokens, int size);

void free_array_of_arrays(char*** array, int size);