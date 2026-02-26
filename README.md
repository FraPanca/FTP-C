# FTP-C
A File Transfer Protocol based on C.

==== COMPONENTI ====
 - Client su cui è possibile effettuare operazioni di mget/mput
 - Server su cui è possibile effettuare operazioni di mget/mput


==== ISTRUZIONI ====
 - Compilazione:
	- $ gcc server.c -o server		<- directory server
	- $ gcc client.c -o client		<- directory client
	
- Esecuzione
  - $ ./server serverSocketPort
	- $ ./client serverIpAddress serverPort
