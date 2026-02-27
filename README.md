# FTP-C

## Italiano
Un File Transfer Protocol sviluppato in C.

### Componenti
- Client: permette di effettuare operazioni di mget/mput
- Server: permette di effettuare operazioni di mget/mput

### Istruzioni
- Compilazione:
  - $ gcc server.c -o server      <- directory server
  - $ gcc client.c -o client      <- directory client

- Esecuzione:
  - $ ./server serverSocketPort
  - $ ./client serverIpAddress serverPort

---

## English
A File Transfer Protocol based on C.

### Components
- Client: allows mget/mput operations
- Server: allows mget/mput operations

### Instructions
- Compile:
  - $ gcc server.c -o server      <- server directory
  - $ gcc client.c -o client      <- client directory

- Run:
  - $ ./server serverSocketPort
  - $ ./client serverIpAddress serverPort
