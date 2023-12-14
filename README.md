
# Progetto, breve relazione

## -- SERVER.PY --
Il programma server.py, dopo aver generato il processo che esegue archivio.c assieme alle pipe caposc 
capo let, si occupa di mettersi in ascolto per una connessione. 

Una volta che uno dei thread del server accetta una connessione, 
il client connesso comunica se si tratta di una **connessione di tipo
A** o **tipo B**, il thread server riceve la risposta e gestisce la connessione basandosi su questa
informazione, **scrivendo la lunghezza della sequenza** e **la sequenza stessa** nella **pipe associata**.

Quando viene mandato il segnale SIGINT il server si occupa **prima di mandare il segnale
SIGTERM ad archivio.c**, poi di **chiudere le pipe** e poi **esegue una wait**, terminando quindi
solo dopo archivio.c. 

## -- CLIENT 1 e CLIENT 2 --
Client 1 utilizza una **connessione di tipo A**, quindi **viene aperta (e chiusa) una nuova socket e
una nuova connessione per ogni riga del file** che gli è stato passato.

Client 1, a differenza di Client 2, **non comunica quando arriva a EOF**, poichè per come viene
gestita la connessione di tipo A da Server.py si tratta di un'informazione irrilevante.

Client 2 **mantiene un'unica connessione durante il passaggio di ogni riga**, quindi **viene creata
un'unica socket e un'unica connessione**, per questo motivo è **importante comunicare la lettura di EOF**
che viene fatta **mandando un byte a 0** quando il server si aspetta di ricevere la lunghezza della sequenza.
Il server, leggendo che la prossima sequenza ha lunghezza 0, capisce che Client 2 ha raggiunto EOF
e chiude la connessione.

## -- ARCHIVIO.C --
Archivio.c è strutturato in questo modo:
## -- THREAD MAIN -- 
Si occupa di **generare tutti i thread**, quindi genera il capo lettore, capo scrittore, i thread scrittori, thread lettori e il
thread segnali, una volta aver finito di generare i thread **si mette in attesa
della terminazione del thread segnali** poichè **l'unica maniera pulita del programma di terminare
è attraverso il segnale SIGTERM**.

## -- THREAD CAPO LET CAPO S --
I thread capi leggono la loro corrispettiva pipe, per far questo utilizzano la funzione "clean_my_array", che si tratta di una funzione
che **"pulisce" l'array "content" dalla lettura precedente** utilizzando la lunghezza della sequenza che deve leggere, ho pensato
fosse una soluzione più semplice rispetto a creare array con malloc per la lettura, che poi sarebbero dovuti essere liberati.

I puntatori "array_tokens" e "array_copy" per la **deallocazione di memoria**.

Non c'è una forte motivazione per la scelta dei semafori rispetto a condition variables, poichè
la situazione di "Buffer vuoto" e "Buffer pieno" può essere segnalata con successo da entrambi,
è stata una scelta guidata dalla maggiore semplicità dei semafori.

I capi scrivono sul buffer bloccando e sbloccando una **mutex condivisa con i relativi consumatori**.

## -- THREAD LETTORI E SCRITTORI --
I thread consumatori, utilizzano gli **stessi semafori e mutex dei capi**, ad eccezione della mutex
per lettori.log per i lettori e della mutex per la hashmap che è l'unica mutex condivisa sia tra
lettori che tra scrittori.

Alla lettura di "\0" (valore di terminazione inserito dai capi) **i thread consumatori terminano**.

## -- THREAD SEGNALI --
Il thread segnali è l'unico thread che non blocca i segnali in arrivo, attendendo SIGTERM o SIGINT
durante la sigwait(). 

Utilizza "modify_my_insert_string" che si occupa di **scrivere in una stringa il numero di inserimenti
fatti nella hashmap**, utilizzo questa funzione poichè questa informazione la devo scrivere con una funzione
signal-safe come la write.

Quando riceve un SIGTERM **scrive la quantità di stringe inserite** ed esegue una **join dei due capi**,
il che è sufficiente poichè **i capi eseguono la join con i relativi consumatori prima di terminare**.
Una volta terminato, il thread segnali **restituisce il controllo a thread main il quale terminando
chiude l'esecuzione di archivio.c**.
