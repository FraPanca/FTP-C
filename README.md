# FTP-C

## Italiano

Un semplice File Transfer Protocol client/server in C su socket POSIX, con trasferimento massivo di file su TCP e un server concorrente basato su processi.

**Stack:** C · Socket TCP (BSD sockets) · `fork()` / gestione processi su UNIX

### Descrizione

Implementazione di un protocollo di trasferimento file client/server su TCP, ispirato ai comandi `mget`/`mput` degli storici client FTP. Il client si connette al server e, per ogni operazione, apre una nuova connessione TCP: con `mput` invia tutti i file regolari presenti in una directory locale, con `mget` richiede e riceve tutti i file regolari presenti in una directory remota sul server.

Il server è concorrente: per ogni connessione viene creato un processo figlio con `fork()`, mentre un signal handler su `SIGCHLD` evita l'accumulo di processi zombie.

### Come si esegue

Compilazione:
```
gcc server/server.c -o server      # dalla cartella server
gcc client/client.c -o client      # dalla cartella client
```

Esecuzione:
```
./server serverSocketPort
./client serverIpAddress serverPort
```

Il client chiede a console l'operazione da eseguire (`mput` o `mget`, terminare con EOF) e il nome della directory su cui operare; è possibile eseguire più operazioni in sequenza nella stessa esecuzione del client.

### Funzionalità principali

- Trasferimento file client/server su TCP in due modalità: `mput` (carica tutti i file regolari di una directory locale sul server) e `mget` (scarica tutti i file regolari di una directory presente sul server)
- Server concorrente basato su processi: ogni connessione è gestita da un processo figlio (`fork()`), con signal handler su `SIGCHLD` per evitare processi zombie
- Trasferimento binario a blocchi di dimensione fissa (256 byte), con verifica dei byte effettivamente ricevuti rispetto alla dimensione dichiarata del file
- Filtro automatico sui soli file regolari (`DT_REG`), escludendo automaticamente sottodirectory e altri tipi di voci
- Sessione client interattiva: è possibile eseguire più operazioni (`mput`/`mget`) in sequenza fino a EOF da input

### Struttura del progetto

```
FTP-C/
├── client/
│   ├── client.c    # Client: invia (mput) o richiede (mget) i file di una directory
│   ├── dir1/        # Directory di esempio da caricare (mput)
│   └── dir2/        # Directory di esempio da caricare (mput)
├── server/
│   ├── server.c     # Server concorrente: gestisce mput/mget per ogni connessione
│   └── dir3/         # Directory di esempio da scaricare (mget)
```

### Note

Progetto didattico in C su socket POSIX, non pensato per un uso in produzione: non è presente alcuna autenticazione né cifratura del traffico. Il client legge i comandi da console con `gets()`, funzione deprecata e rimossa dallo standard C11 per il rischio di buffer overflow: in un contesto reale andrebbe sostituita con `fgets()`. I file ricevuti (sia in `mget` lato client sia in `mput` lato server) vengono scritti nella directory corrente del processo, senza ricreare la struttura di sottocartelle di origine.

### Licenza

MIT

---

## English

A simple client/server File Transfer Protocol in C over POSIX sockets, supporting bulk file transfer over TCP with a process-based concurrent server.

**Stack:** C · TCP sockets (BSD sockets) · `fork()` / UNIX process management

### Description

Implementation of a client/server file-transfer protocol over TCP, inspired by the classic `mget`/`mput` FTP commands. The client connects to the server and opens a new TCP connection for each operation: with `mput` it sends every regular file in a local directory, with `mget` it requests and receives every regular file in a remote directory on the server.

The server is concurrent: a child process is created via `fork()` for each connection, while a `SIGCHLD` signal handler prevents zombie processes from accumulating.

### How to run

Compile:
```
gcc server/server.c -o server      # from the server directory
gcc client/client.c -o client      # from the client directory
```

Run:
```
./server serverSocketPort
./client serverIpAddress serverPort
```

The client prompts on the console for the operation to perform (`mput` or `mget`, EOF to quit) and the name of the directory to operate on; multiple operations can be performed in sequence within the same client run.

### Key features

- Client/server file transfer over TCP in two modes: `mput` (upload every regular file in a local directory to the server) and `mget` (download every regular file from a directory on the server)
- Process-based concurrent server: each connection is handled by a child process (`fork()`), with a `SIGCHLD` signal handler to prevent zombie processes
- Binary transfer in fixed-size chunks (256 bytes), verifying the actual bytes received against the declared file size
- Automatic filtering of regular files only (`DT_REG`), excluding subdirectories and other entry types
- Interactive client session: multiple `mput`/`mget` operations can be run in sequence until EOF

### Project structure

```
FTP-C/
├── client/
│   ├── client.c    # Client: sends (mput) or requests (mget) the files of a directory
│   ├── dir1/        # Sample directory to upload (mput)
│   └── dir2/        # Sample directory to upload (mput)
├── server/
│   ├── server.c     # Concurrent server: handles mput/mget for each connection
│   └── dir3/         # Sample directory to download (mget)
```

### Notes

Educational project in C over POSIX sockets, not intended for production use: there is no authentication or traffic encryption. The client reads console commands with `gets()`, a function deprecated and removed from the C11 standard for its buffer-overflow risk — in a real-world context it should be replaced with `fgets()`. Received files (both via `mget` on the client and `mput` on the server) are written to the process's current working directory, without recreating the source subdirectory structure.

### License

MIT
