#include "betterfunctions.h"
#include "auxiliaryfunctions.h"
#define HERE __LINE__,__FILE__
#define PC_buffer_len 10
#define Num_elem 1000000
#define Max_sequence_length 2048

// --------- STRUCT THREAD --------- 
typedef struct {
  int val;  
  ENTRY *next;  
} pair;

typedef struct {
    pthread_t *writers;
    int *nwriters;
    char** buffer;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    pthread_mutex_t *buffer_mutex;
} man_writers;

typedef struct {
    pthread_t *readers; 
    int *nreaders;
    char** buffer;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    pthread_mutex_t *buffer_mutex;
} man_readers;

typedef struct {
    int *writing;
    int *readers;
    char** buffer;
    int *index;
    int *ninserts; // PER CONTROLLO DI MEMORIA
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    pthread_mutex_t *buffer_mutex;
    pthread_mutex_t *hash_mutex;
    pthread_cond_t *cond_hash;
} writers;

typedef struct {
    int* writing;
    int* readers;
    char** buffer;
    int *index;
    FILE *lettorilog;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
    pthread_mutex_t *buffer_mutex;
    pthread_mutex_t *hash_mutex;
    pthread_mutex_t *file_mutex;
    pthread_cond_t *cond_hash;
} readers;

typedef struct{
    pthread_t *t_manreader;
    pthread_t *t_manwriter;
    int *ninserts;
} signals;

// --------- OPERAZIONI TABELLA HASH --------- 
void aggiungi(char *s, writers *str){   
    ENTRY *e = crea_entry(s, 1);

    fun_thread_mutex_lock(str->hash_mutex,HERE);
    while(*(str->writing) || *(str->readers)>0) fun_pthread_cond_wait(str->cond_hash,str->hash_mutex,HERE);
    *(str->writing) = 1;
    fun_thread_mutex_unlock(str->hash_mutex,HERE);
    ENTRY *en = hsearch(*e,FIND);

    if(en == NULL){
        en = hsearch(*e,ENTER);
        *(str->ninserts) = *(str->ninserts) + 1;

        fun_thread_mutex_lock(str->hash_mutex,HERE);
        *(str->writing) = 0;
        fun_thread_cond_broadcast(str->cond_hash,HERE);
        fun_thread_mutex_unlock(str->hash_mutex,HERE);

        if(en==NULL) fun_termina("Errore o tabella piena.");
    }
    else{

        int *dat = (int *) en->data;
        *dat += 1;

        fun_thread_mutex_lock(str->hash_mutex,HERE);
        *(str->writing) = 0;
        fun_thread_cond_broadcast(str->cond_hash,HERE);
        fun_thread_mutex_unlock(str->hash_mutex,HERE);

        distruggi_entry(e);
    }
}

int conta (char* s, readers *str){
    ENTRY *e = crea_entry(s, 1);

    fun_thread_mutex_lock(str->hash_mutex,HERE);
    while(*(str->writing)) fun_pthread_cond_wait(str->cond_hash,str->hash_mutex,HERE);
    *(str->readers) = *(str->readers) + 1;
    fun_thread_mutex_unlock(str->hash_mutex,HERE);

    ENTRY *en = hsearch(*e,FIND);

    fun_thread_mutex_lock(str->hash_mutex,HERE);
    *(str->readers) = *(str->readers) - 1;
    if(*(str->readers)==0) fun_pthread_cond_signal(str->cond_hash,HERE);
    fun_thread_mutex_unlock(str->hash_mutex,HERE);

    if(en == NULL){
        distruggi_entry(e);
        return 0;
    }
    pair* p =(pair*) en -> data;
    distruggi_entry(e);
    return p->val;
}

// --------- THREAD CAPO SCRITTORE --------- 
void *body_writers_manager(void *arg){

    // -- DICHIARAZIONE DI VARIABILI --
    man_writers *str = (man_writers *)arg;
    int length = 0; int size_tokens = 0;
    int prod_index = 0; int i;
    char** tokens; char* copy = ""; char content[Max_sequence_length] = " ";
    
    int nput_array_copies = 0; int size_array_copies = 10;
    char** array_copies = malloc(size_array_copies*sizeof(char*));
    if(array_copies == NULL) fun_termina("Errore nella malloc");

    int nput_array_tokens = 0; int size_array_tokens = 10;
    char*** array_tokens = malloc(size_array_tokens*sizeof(char**));
    if(array_tokens == NULL) fun_termina("Errore nella malloc");


    // -- INIZIO LA LETTURA --
    int capo_s = fun_open("caposc", O_RDONLY, HERE);
    while(true){

         // -- LEGGO LA LUNGHEZZA DELLA SEQUENZA --
        ssize_t e = read(capo_s,&length,sizeof(length));
        if(e==-1){ perror("read"); pthread_exit(NULL);}
        if(e==0) break;

        //-- PULISCO L ARRAY DALLA LETTURA PRECEDENTE E LEGGO LA SEQUENZA --
        clean_my_array(content,length);
        e = read(capo_s,&content,length);
        if(e==0) break; // NON DOVREBBE SUCCEDERE, MA PER SICUREZZA.....

        // -- TOKENIZZO E AGGIUNGO UN BYTE A ZERO IN FONDO --
         if(nput_array_tokens >= size_array_tokens){
            size_array_tokens = size_array_tokens + 10;
            array_tokens = realloc(array_tokens, size_array_tokens*sizeof(char**));
            if(array_tokens==NULL) fun_termina("Errore realloc");
        }

        content[length] = '\0';
        tokens = tokenize(content,".,:; \n\r\t", &size_tokens);

        array_tokens[nput_array_tokens] = tokens;
        nput_array_tokens = nput_array_tokens + 1;


        // -- INSERISCO CIASCUN TOKEN NEL BUFFER CONDIVISO CON I LETTORI --
        for(i=0;i<size_tokens;i++){

            if(nput_array_copies >= size_array_copies){
                size_array_copies = size_array_copies + 10;
                array_copies = realloc(array_copies,size_array_copies*sizeof(char**));
                if(array_copies==NULL) fun_termina("Errore realloc");
            }
            
            copy = strdup(tokens[i]);
                
            array_copies[nput_array_copies] = copy;
            nput_array_copies = nput_array_copies + 1;

            fun_sem_wait(str->sem_free_slots, HERE);
            fun_thread_mutex_lock(str->buffer_mutex, HERE);
            str->buffer[prod_index++ % PC_buffer_len] = copy;
            fun_thread_mutex_unlock(str->buffer_mutex, HERE);
            fun_sem_post(str->sem_data_items, HERE);
            
        }
    }

    // -- MANDO IL SEGNALE DI TERMINAZIONE --
    for(i=0;i<*(str->nwriters);i++){
        fun_sem_wait(str->sem_free_slots, HERE);
        fun_thread_mutex_lock(str->buffer_mutex, HERE);
        str->buffer[prod_index++ % PC_buffer_len] = NULL;
        fun_thread_mutex_unlock(str->buffer_mutex, HERE);
        fun_sem_post(str->sem_data_items, HERE);
    }

    // -- ATTENDO LA TERMINAZIONE DEI THREAD SCRITTORI --
    for(i=0;i<*(str->nwriters);i++){
        fun_thread_join(str->writers[i],NULL,HERE);
    }

    free_arrays(array_copies, nput_array_copies);
    free_array_of_arrays(array_tokens,nput_array_tokens);

    pthread_exit(NULL);
}

// --------- THREAD CAPO LETTORE --------- 
void *body_readers_manager(void *arg){

    // -- DICHIARAZIONE DI VARIABILI --
    man_readers *str = (man_readers *)arg;
    char** tokens; char* copy;
    int length = 0; char content[Max_sequence_length] = " ";
    int prod_index = 0, size_tokens = 0, i;

    int nput_array_copies = 0; int size_array_copies = 10;
    char** array_copies = malloc(size_array_copies*sizeof(char*));
    if(array_copies == NULL) fun_termina("Errore nella malloc");

    int nput_array_tokens = 0; int size_array_tokens = 10;
    char*** array_tokens = malloc(size_array_tokens*sizeof(char**));
    if(array_tokens == NULL) fun_termina("Errore nella malloc");

    
    // -- INIZIO LA LETTURA --
    int capo_let = fun_open("capolet", O_RDONLY, HERE);
    while(true){

        // -- LEGGO LA LUNGHEZZA DELLA SEQUENZA --
        ssize_t e = read(capo_let,&length,sizeof(length));
        if(e==-1) { perror("read"); pthread_exit(NULL);}
        if(e==0) break;

        //-- PULISCO L ARRAY DALLA LETTURA PRECEDENTE E LEGGO LA SEQUENZA --
        clean_my_array(content,length);
        e = read(capo_let,&content,length);
        if(e==0) break; // NON DOVREBBE SUCCEDERE, MA PER SICUREZZA.....

        if(nput_array_tokens >= size_array_tokens){
            size_array_tokens = size_array_tokens + 10;
            array_tokens = realloc(array_tokens, size_array_tokens*sizeof(char**));
            if(array_tokens==NULL) fun_termina("Errore realloc");
        }

        // -- TOKENIZZO E AGGIUNGO UN BYTE A ZERO IN FONDO --
        content[length] = '\0';
        tokens = tokenize(content,".,:; \n\r\t", &size_tokens);

        array_tokens[nput_array_tokens] = tokens;
        nput_array_tokens = nput_array_tokens + 1;


        // -- INSERISCO CIASCUN TOKEN NEL BUFFER CONDIVISO CON I LETTORI --
        for(i=0;i<size_tokens;i++){

            if(nput_array_copies >= size_array_copies){
                size_array_copies = size_array_copies + 10;
                array_copies = realloc(array_copies,size_array_copies*sizeof(char**));
                if(array_copies==NULL) fun_termina("Errore realloc");
            }

            copy = strdup(tokens[i]);

            array_copies[nput_array_copies] = copy;
            nput_array_copies = nput_array_copies + 1;

            fun_sem_wait(str->sem_free_slots, HERE);
            fun_thread_mutex_lock(str->buffer_mutex, HERE);
            str->buffer[prod_index++ % PC_buffer_len] = copy;
            fun_thread_mutex_unlock(str->buffer_mutex, HERE);
            fun_sem_post(str->sem_data_items, HERE);

        }

    }

    // -- MANDO IL SEGNALE DI TERMINAZIONE --
    for(i=0;i<*(str->nreaders);i++){

        fun_sem_wait(str->sem_free_slots, HERE);
        fun_thread_mutex_lock(str->buffer_mutex, HERE);
        str->buffer[prod_index++ % PC_buffer_len] = NULL;
        fun_thread_mutex_unlock(str->buffer_mutex, HERE);
        fun_sem_post(str->sem_data_items, HERE);

    }

    // -- ATTENDO LA TERMINAZIONE DEI THREAD LETTORI --
    for(i=0;i<*(str->nreaders);i++){
        fun_thread_join(str->readers[i],NULL,HERE);
    }

    // -- DEALLOCO LA MEMORIA E ESCO --
    free_arrays(array_copies, nput_array_copies);
    free_array_of_arrays(array_tokens,nput_array_tokens);;

    pthread_exit(NULL);
}

// --------- THREAD SCRITTORI --------- 
void *thread_writers_body(void *arg){
    
    // -- DICHIARAZIONE DI VARIABILI --
    writers *struct_w = (writers *)arg;
    char* content = " ";

    // -- COMINCIO LA LETTURA DA BUFFER --
    while(true){

        fun_sem_wait(struct_w->sem_data_items, HERE);
        fun_thread_mutex_lock(struct_w->buffer_mutex, HERE);
        content = struct_w->buffer[*(struct_w->index) % PC_buffer_len];
        *(struct_w->index) += 1;
        fun_thread_mutex_unlock(struct_w->buffer_mutex, HERE);
        fun_sem_post(struct_w->sem_free_slots, HERE);

        if(content==NULL) break;

        aggiungi(content, struct_w);
    }
    pthread_exit(NULL);
}

// --------- THREAD LETTORI --------- 
void *thread_readers_body(void *arg){

    // -- DICHIARAZIONE DI VARIABILI --
    readers *struct_r = (readers *)arg;
    char *content; int data;
    
    // -- COMINCIO LA LETTURA DA BUFFER --
    while(true){

        fun_sem_wait(struct_r->sem_data_items, HERE);
        fun_thread_mutex_lock(struct_r->buffer_mutex, HERE);
        content = struct_r->buffer[*(struct_r->index) % PC_buffer_len];
        *(struct_r->index) += 1;
        fun_thread_mutex_unlock(struct_r->buffer_mutex, HERE);
        fun_sem_post(struct_r->sem_free_slots, HERE);

        if(content==NULL) break; // COMANDO DI TERMINAZIONE

        data = conta(content, struct_r);
        
        // -- SCRIVO LA STRINGA LETTA E IL VAL DELLA HASH TABLE
        //    SUL FILE LETTORI.LOG --
        fun_thread_mutex_lock(struct_r->file_mutex, HERE);
        fprintf(struct_r->lettorilog, "%s %d\n", content, data);
        fun_thread_mutex_unlock(struct_r->file_mutex, HERE);
        
    }

    pthread_exit(NULL);
}

// --------- THREAD SEGNALI --------- 
void *thread_signal_manager(void *arg){

    // -- IMPOSTO I SEGNALI --
    sigset_t mask_to_handle;
    siginfo_t sinfo;
    sigemptyset(&mask_to_handle);
    sigaddset(&mask_to_handle,SIGTERM);
    sigaddset(&mask_to_handle,SIGINT);

    // -- DICHIARAZIONE DI VARIABILI --
    int e;
    signals *struct_sig = (signals *)arg;
    char str_ninserts[] = "Numero di stringe distinte contenute nella hash table: 1000000\n";
    int len_str = strlen(str_ninserts);

    // -- GESTIONE DEI SEGNALI --
    while(true){

        e = sigwaitinfo(&mask_to_handle,&sinfo); //MI METTO IN ATTESA SOLO DI SIGINT E SIGTERM
        if(e<0) perror("Errore sigwaitinfo");
        switch(sinfo.si_signo) {
            case 2: // -- CASO SIGINT --
                modify_my_insert_string(str_ninserts, *(struct_sig->ninserts)); 
                e = write(2,str_ninserts,len_str);
                break;
            
            case 15: // -- CASO SIGTERM --

                // -- ATTENDO I CAPI --
                fun_thread_join(*(struct_sig->t_manreader),NULL,HERE);
                fun_thread_join(*(struct_sig->t_manwriter),NULL,HERE);

                // -- STAMPO SU STDOUT QUANTE STRINGE DISTINTE CONTIENE LA HT --
                modify_my_insert_string(str_ninserts, *(struct_sig->ninserts));
                e = write(1,str_ninserts,len_str);

                // -- DEALLOCO LA (PARZIALMENTE) LA MEMORIA E TERMINO --
                hdestroy();
                pthread_exit(NULL);
                break;
            default:
                break;
        }
    }
}

// -- MAIN --
int main(int argc, char *argv[]){
    if(argc != 3){fun_termina("Scrivi 'archivio w r' dove w sono il numero di thread scrittori e r sono il numero di thread lettori");} //ERRORE
    
    // -- IMPOSTO I SEGNALI --
    sigset_t set;   
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    // -- DICHIARO VARIABILI E CREO LA HT --
    int ht = hcreate(Num_elem);
    if(ht==0) fun_termina("Errore creazione HT");
    int nwriters = atoi(argv[1]); 
    int nreaders = atoi(argv[2]); 
    int ninserts = 0;  int i;
    
    //-- COND E MUTEX PER LETTORI E SCRITTORI --
    pthread_cond_t cond_hash;
    pthread_mutex_t hash_mu;
    fun_pthread_mutex_init(&hash_mu,NULL,HERE);
    fun_pthread_cond_init(&cond_hash,NULL,HERE);
    int num_readers = 0; int writing = 0;

    // -- DICHIARAZIONE DI VARIABILI CAPO LET --
    char* buffer_readers[PC_buffer_len];
    pthread_mutex_t mu_readers;
    fun_pthread_mutex_init(&mu_readers,NULL,HERE);
    sem_t sem_free_slots_readers, sem_data_items_readers;
    fun_sem_init(&sem_free_slots_readers,0,PC_buffer_len,HERE);
    fun_sem_init(&sem_data_items_readers,0,0,HERE);
    pthread_t thread_reader_manager; man_readers struct_reader_manager;  

    // -- DICHIARAZIONE DI VARIABILI LETTORI --
    int index_readers = 0;
    pthread_mutex_t mu_file;
    fun_pthread_mutex_init(&mu_file,NULL,HERE);
    pthread_t thread_readers[nreaders]; readers str_readers[nreaders];     
    FILE *lettorilog = fopen("lettori.log","w");
    if(lettorilog==NULL) fun_termina("Errore in apertura di lettori.log");


    // -- INIZIALIZZO THREAD LETTORI --
    for(i=0;i<nreaders;i++){
        str_readers[i].readers = &num_readers;
        str_readers[i].writing = &writing;
        str_readers[i].cond_hash = &cond_hash;
        str_readers[i].buffer = buffer_readers;
        str_readers[i].index = &index_readers;
        str_readers[i].buffer_mutex = &mu_readers;
        str_readers[i].hash_mutex = &hash_mu;
        str_readers[i].file_mutex = &mu_file;
        str_readers[i].sem_data_items = &sem_data_items_readers;
        str_readers[i].sem_free_slots = &sem_free_slots_readers;
        str_readers[i].lettorilog = lettorilog;
        fun_thread_create(&thread_readers[i],NULL,&thread_readers_body,&str_readers[i], HERE);
    }

    // -- INIZIALIZZO IL MANAGER READER --
    struct_reader_manager.readers = thread_readers;
    struct_reader_manager.nreaders = &nreaders;
    struct_reader_manager.buffer = buffer_readers;
    struct_reader_manager.buffer_mutex = &mu_readers;
    struct_reader_manager.sem_data_items = &sem_data_items_readers;
    struct_reader_manager.sem_free_slots = &sem_free_slots_readers;
    fun_thread_create(&thread_reader_manager,NULL,&body_readers_manager,&struct_reader_manager, HERE);
    
    
    // -- DICHIARAZIONE DI VARIABILI CAPO SC --
    char* buffer_writers[PC_buffer_len];
    pthread_mutex_t mu_writers;
    fun_pthread_mutex_init(&mu_writers,NULL,HERE);
    sem_t sem_free_slots_writers, sem_data_items_writers;
    fun_sem_init(&sem_free_slots_writers,0,PC_buffer_len,HERE);
    fun_sem_init(&sem_data_items_writers,0,0,HERE);
    pthread_t thread_writer_manager; man_writers struct_writer_manager;

    // -- DICHIARAZIONE DI VARIABILI LETTORI --
    pthread_t thread_writers[nwriters];
    int index_writers = 0;
	writers struct_writers[nwriters];


    // -- INIZIALIZZAZIONE DI THREAD SCRITTORI --
    for(i=0;i<nwriters;i++){
        struct_writers[i].readers = &num_readers;
        struct_writers[i].writing = &writing;
        struct_writers[i].cond_hash = &cond_hash;
        struct_writers[i].ninserts = &ninserts;
        struct_writers[i].index = &index_writers;
        struct_writers[i].buffer = buffer_writers;
        struct_writers[i].buffer_mutex = &mu_writers;
        struct_writers[i].sem_data_items = &sem_data_items_writers;
        struct_writers[i].sem_free_slots = &sem_free_slots_writers;
        struct_writers[i].hash_mutex = &hash_mu;
        fun_thread_create(&thread_writers[i],NULL,&thread_writers_body,&struct_writers[i], HERE);
    }

    // -- INIZIALIZZO IL MANAGER WRITER --
    struct_writer_manager.writers = thread_writers;
    struct_writer_manager.nwriters = &nwriters;
    struct_writer_manager.buffer = buffer_writers;
    struct_writer_manager.buffer_mutex = &mu_writers;
    struct_writer_manager.sem_data_items = &sem_data_items_writers;
    struct_writer_manager.sem_free_slots = &sem_free_slots_writers;
    fun_thread_create(&thread_writer_manager,NULL,&body_writers_manager,&struct_writer_manager, HERE);
    
    // -- DICHIARAZIONE VARIABILI (THREAD SEGNALI) --
    pthread_t thread_signal;
    signals str_signals;
    str_signals.ninserts = &ninserts;
    str_signals.t_manwriter = &thread_writer_manager;
    str_signals.t_manreader = &thread_reader_manager;

    // -- INIZIALIZZO THREAD SEGNALI --
    fun_thread_create(&thread_signal,NULL,&thread_signal_manager,&str_signals,HERE);

    // -- ATTENDO LA TERMINAZIONE DEL THREAD SEGNALI --
    fun_thread_join(thread_signal,NULL,HERE);

    // DISTRUGGI LA ROBA
    fun_pthread_cond_destroy(&cond_hash,HERE);
    fun_pthread_mutex_destroy(&mu_file,HERE);
    fun_pthread_mutex_destroy(&mu_readers,HERE);
    fun_pthread_mutex_destroy(&mu_writers,HERE);
    fun_pthread_mutex_destroy(&hash_mu,HERE);
    fclose(lettorilog);
    
}