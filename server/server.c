#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


#define MAX_DIM_BUFF 256
#define DIM_OPERATION 5

// gestione per evitare processi figli su UNIX
void handler(int signo) {
    int stato;
    
    printf("[SERVER] : (P0) Esecuzione gestore di SIGCHLD\n");
    wait(&stato);
}


int main(int argc, char **argv) {
	 const int on = 1;
    int port, listen_sd, conn_sd, fd;
    int i, nread, size, numFiles, receivedBytes;
    char dirName[FILENAME_MAX + 1], fileName[FILENAME_MAX + 1], path[FILENAME_MAX + 1], buff[MAX_DIM_BUFF], operation[DIM_OPERATION];
    
    struct sockaddr_in cliaddr, servaddr;
    struct hostent *host;
    
    DIR *dir = NULL;
    struct dirent *entry;

	    
	 // controllo argomenti
    if(argc != 2) {
        printf("[SERVER] : Errore -> Gli argomenti da inserire sono 1: numero di porta\n");
        exit(1);
    }
    
    // controllo sul numero di porta
    nread = 0;
    while(argv[1][nread] != '\0') {
    	if ((argv[1][nread] < '0') || (argv[1][nread] > '9')) {
      	printf("[SERVER] : Errore -> Il numero di porta non è un intero\n");
         exit(2);
      }
      
      nread++;
    }
        
    port = atoi(argv[1]);
    if (port < 1024 || port > 65535) {
    		printf("[SERVER] : Errore -> Il numero di porta deve essere compreso tra 1024 e 65535\n");
     		exit(2);
    }
    

    // init indirizzo server
    memset((char *) &servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    // creazione socket di ascolto
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd < 0) {
        perror("[SERVER] : Errore -> Impossibile creare la socket");
        exit(3);
    }

	 // impostazione delle opzioni della socket di ascolto
    if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("[SERVER] : Errore -> Impossibile impostare le opzioni sulla socket");
        exit(3);
    }

	 // bind della socket di ascolto
    if (bind(listen_sd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("[SERVER] : Errore -> Impossibile fare la bind sulla socket");
        exit(3);
    }
    
	 // creazione coda di ascolto
    if (listen(listen_sd, 5) < 0) {
        perror("[SERVER] : Errore -> Impossibile creare la coda di ascolto");
        exit(4);
    }
    
    printf("[SERVER] : In ascolto sulla porta %d...\n", port);

	 // gestione dei figli per evitare processi zombie (sistemi UNIX)
    signal(SIGCHLD, handler);

	
    // ricezione richieste ed elaborazione risposte
    while(1) {
    
    	  // accettazione della richiesta e creazione della socket di connessione
        size = sizeof(cliaddr);       
        if ((conn_sd = accept(listen_sd, (struct sockaddr *) &cliaddr, &size)) < 0) {
            if (errno == EINTR) {
                perror("[SERVER] : Errore -> Non è stato possibile comunicare con il client");
                continue;
            } else {
            	exit(5);
        		}
        }
	
		  // creazione processi figli
		  switch(fork()) {
		  		
		  		case 0:
		  			// chiusura socket lister
		         close(listen_sd);

		         host = gethostbyaddr((char *) &cliaddr.sin_addr, sizeof(cliaddr.sin_addr), AF_INET);
		         if (host == NULL) printf("\n[SERVER] : (child) L'host del client e' %s\n", inet_ntoa(cliaddr.sin_addr));
		         else printf("\n[SERVER] : (child) L'host del client e' %s\n", host->h_name);

    		         
		         // trasferimento dei file
		         read(conn_sd, operation, sizeof(operation));
		         if(strcmp(operation, "mput") != 0 && strcmp(operation, "mget") != 0) {
            		printf("[SERVER] : (child) Errore -> Inserito un comando non valido %s.\n", operation);
           			continue;
        			}
        			
        			printf("[SERVER] : (child) Inizia il trasferimento\n");

		         if(strcmp(operation, "mget") == 0) { // caso client mget  
		         
		         		if((nread = read(conn_sd, dirName, sizeof(dirName))) < 0) {
		         			 perror("[SERVER] : (child) Errore -> Il direttorio inserito non è presente nel file system");
						       continue;
		         		}
		         		
		             	// apertura del direttorio
						   if((dir = opendir(dirName)) == NULL) {
						       perror("[SERVER] : (child) Errore -> Il direttorio inserito non è presente nel file system");
						       continue;
						   } 
						   
						   // estrapolo il numero di file totali nel direttorio
						   numFiles = 0;
						   while ((entry = readdir(dir)) != NULL) {
						   	if (entry->d_type == DT_REG) numFiles++;
						   }
					  		rewinddir(dir); 
					  		
					  		if(write(conn_sd, &numFiles, sizeof(int)) < 0) {
								perror("[SERVER] : (child) Errore -> Impossibile inviare il numero dei file presenti nel direttorio");
								break;
							}         


							// ciclo sui file del direttorio
						   while((entry = readdir(dir)) != NULL) {
						            
						   	if(entry->d_type == DT_REG) {
						   		 printf("[SERVER] : (child) Invio file %s\n", entry->d_name);
						   		 
						   		 // creazione del path
						   		 snprintf(path, sizeof(path), "%s/%s", dirName, entry->d_name);
						   	            	            	
						   		 if((fd = open(path, O_RDONLY)) < 0) {
								        perror("[SERVER] : (child) Errore -> Impossibile aprire il file");
								        break;
								    }

								    if(write(conn_sd, entry->d_name, FILENAME_MAX) < 0) {
								        perror("[SERVER] : (child) Errore -> Impossibile inviare il nome del file");
										close(fd);
								        break;
								    }
								    
								    // estrapolo la dimensione totale del file
								    size = lseek(fd, 0, SEEK_END);
								    lseek(fd, 0, SEEK_SET);
								    
								    if(write(conn_sd, &size, sizeof(int)) < 0) {
								        perror("[SERVER] : (child) Errore -> Impossibile inviare la dimensione del file");
										close(fd);
								        break;
								    }

									 // invio del file
								    while((nread = read(fd, buff, sizeof(buff))) > 0) {
								        if(write(conn_sd, buff, nread) < 0) {
								            perror("[SERVER] : (child) Errore -> Impossibile inviare il file");
											close(fd);
								            break;
								        }
								    }

								    printf("[SERVER] : (child) File %s inviato correttamente\n\n", entry->d_name);

								    close(fd);
						   	}                
						   }

						   closedir(dir);
		             
		         } else { // caso client mput
							
							// lettura del numero dei file che verranno inviati
							if((nread = read(conn_sd, &numFiles, sizeof(int))) < 0) {
								 perror("[SERVER] : (child) Errore -> Leggere il numero dei file");
								 break;
							}
							
							
							for(i=0; i<numFiles; i++) {
								//lettura del nome del file
								if((nread = read(conn_sd, fileName, FILENAME_MAX)) < 0) {
									 perror("[SERVER] : (child) Errore -> Impossibile leggere il nome del file");
								 	 break;
								}
									       
								if((fd = open(fileName, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
								  	perror("[SERVER] : (child) Errore -> Impossibile aprire il file");
								   continue;
								}
								printf("[SERVER] : (child) Ricezione file %s\n", fileName);
								
								
								// lettura della dimensione del file
								if((nread = read(conn_sd, &size, sizeof(int))) < 0) {
									 perror("[SERVER] : (child) Errore -> Impossibile leggere la dimensione del file");
									 close(fd);
								 	 break;
								}
								
								// lettura del file
								receivedBytes = 0;
								while(receivedBytes < size) {
									// se i byte rimanenti da leggere sono di più di MAX_DIM_BUFF byte, allora il buffer sarà della sua dimensione.
									// in caso contrario verranno letti solo i byte rimanenti (< MAX_DIM_BUFF)
									if((nread = read(conn_sd, buff, (size - receivedBytes > sizeof(buff)) ? sizeof(buff) : (size - receivedBytes))) <= 0) {
										 perror("[SERVER] : (child) Errore -> Il file non è stato ricevuto correttamente");
										 close(fd);
										 break;
									}
									receivedBytes += nread;
									
									if(write(fd, buff, nread) < 0) {
								      perror("[SERVER] : (child) Errore -> Impossibile scrivere il file sul file system");
									  close(fd);
								      break;
								   }
						  		}
								
								close(fd);
							}
		         }		
		     
		         printf("[SERVER] : Trasferimento terminato\n");
		         close(conn_sd);
		         
		         exit(0);
		         break;
		  			
		  		case -1:
		  			perror("[SERVER] : Errore -> Non è stato possibile eseguire la fork");
		  			close(conn_sd);
		  			break;
		  			
		  		default:
		  			close(conn_sd);		
		  			break;
		  	
		  }
    }
}
