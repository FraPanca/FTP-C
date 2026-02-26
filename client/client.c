#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>


#define MAX_DIM_BUFF 256
#define DIM_OPERATION 5


int main(int argc, char *argv[]) {
	 int port, sd, fd;
    int i, nread, size, numFiles, receivedBytes;
    char dirName[FILENAME_MAX + 1], fileName[FILENAME_MAX + 1], path[FILENAME_MAX + 1], buff[MAX_DIM_BUFF], operation[DIM_OPERATION];
    
    struct hostent *host;
    struct sockaddr_in servaddr;
    
    DIR *dir = NULL;
    struct dirent *entry;

    
    // controllo argomenti
    if(argc != 3) {
        printf("[CLIENT] : Errore -> Gli argomenti da inserire sono 2: indirizzo ip del server e numero di porta del server\n");
        exit(1);
    }
	
	
    // init indirizzo client
    memset((char *) &servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    host = gethostbyname(argv[1]);

    
    // controllo sul numero di porta
    nread = 0;
    while(argv[2][nread] != '\0') {
        if((argv[2][nread] < '0') || (argv[2][nread] > '9')) {
            printf("[CLIENT] : Errore -> Il numero di porta non è un intero\n");
            exit(2);
        }
        
        nread++;
    }
    port = atoi(argv[2]);

    if(port < 1024 || port > 65535) {
        printf("[CLIENT] : Errore -> Il numero di porta deve essere compreso tra 1024 e 65535\n");
        exit(2);
    }
    
    
    if(host == NULL) {
        printf("[CLIENT] : Errore -> L'host %s non è presente in /etc/hosts\n", argv[1]);
        exit(2);
    } else {
        servaddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr))->s_addr;
        servaddr.sin_port = htons(port);
    }


	 printf("\n[CLIENT] : Inserisci l'operazione (mput / mget), EOF per terminare: ");
    while (gets(operation)) {
        if(strcmp(operation, "mput") != 0 && strcmp(operation, "mget") != 0) {
            printf("[CLIENT] : Errore -> Inserito un comando non valido.\n");
            printf("\n[CLIENT] : Inserisci l'operazione (mput / mget), EOF per terminare: ");
            continue;
        }
		  
		  // creazione della socket e connessione col server
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if(sd < 0) {
            perror("[CLIENT] : Errore -> Impossibile creare la socket");
            exit(1);
        }

        if(connect(sd, (struct sockaddr *) &servaddr, sizeof(struct sockaddr_in)) < 0) {
            perror("[CLIENT] : Errore -> Impossibile stabilire una connessione col server");
            exit(1);
        }
        printf("[CLIENT] : Connessione col server stabilita correttamente.\n");


        // invio del tipo di operazione che verrà eseguita
        write(sd, operation, sizeof(operation));
        

        printf("[CLIENT] : Inserisci il nome del direttorio: ");
    	  gets(dirName);

        if(strcmp(operation, "mput") == 0) {
        		// apertura del direttorio
            if((dir = opendir(dirName)) == NULL) {
                perror("[CLIENT] : Errore -> Il direttorio inserito non è presente nel file system");
                printf("\n\n[CLIENT] : Inserisci l'operazione (mput / mget), EOF per terminare: ");
                continue;
            } 
            
            // estrapolo il numero di file totali nel direttorio
            numFiles = 0;
            while ((entry = readdir(dir)) != NULL) {
            	if (entry->d_type == DT_REG) numFiles++;
            }
        		rewinddir(dir); 
        		
        		if(write(sd, &numFiles, sizeof(int)) < 0) {
		         perror("[CLIENT] : Errore -> Impossibile inviare il numero dei file presenti nel direttorio");
		         break;
		      }         


				// ciclo sui file del direttorio
            while((entry = readdir(dir)) != NULL) {
                     
            	if(entry->d_type == DT_REG) {
            		 printf("[CLIENT] : Invio file %s\n", entry->d_name);
            		 
            		 // creazione del path
            		 snprintf(path, sizeof(path), "%s/%s", dirName, entry->d_name);
            	            	            	
            		 if((fd = open(path, O_RDONLY)) < 0) {
		                 perror("[CLIENT] : Errore -> Impossibile aprire il file");
		                 break;
		             }

		             if(write(sd, entry->d_name, FILENAME_MAX) < 0) {
		                 perror("[CLIENT] : Errore -> Impossibile inviare il nome del file");
						 close(fd);
		                 break;
		             }
		             
		             // estrapolo la dimensione totale del file
		             size = lseek(fd, 0, SEEK_END);
		             lseek(fd, 0, SEEK_SET);
		             
		             if(write(sd, &size, sizeof(int)) < 0) {
		                 perror("[CLIENT] : Errore -> Impossibile inviare la dimensione del file");
						 close(fd);
		                 break;
		             }

						 // invio del file
		             while((nread = read(fd, buff, sizeof(buff))) > 0) {
		                 if(write(sd, buff, nread) < 0) {
		                     perror("[CLIENT] : Errore -> Impossibile inviare il file");
							 close(fd);
		                     break;
		                 }
		             }

		             printf("[CLIENT] : File %s inviato correttamente\n\n", entry->d_name);

		             close(fd);
            	}                
            }

            closedir(dir);
            
        } else {
        		// invio del nome del direttorio remoto
        		if(write(sd, dirName, sizeof(dirName)) < 0) {
		          perror("[CLIENT] : Errore -> Impossibile inviare il nome del direttorio");
		          break;
		      }
		      
		      // lettura del numero dei file che verranno inviati
		      if((nread = read(sd, &numFiles, sizeof(int))) < 0) {
		      	 perror("[CLIENT] : Errore -> Leggere il numero dei file");
		          break;
		      }
		      
		      
		      for(i=0; i<numFiles; i++) {
		      	//lettura del nome del file
		      	if((nread = read(sd, fileName, FILENAME_MAX)) < 0) {
		      		 perror("[CLIENT] : Errore -> Impossibile leggere il nome del file");
		          	 break;
		      	}
		      	          
		         if((fd = open(fileName, O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0) {
		           	perror("[CLIENT] : Errore -> Impossibile aprire il file");
		            continue;
		         }
		         printf("[CLIENT] : Ricezione file %s\n", fileName);
		         
		         
		         // lettura della dimensione del file
		         if((nread = read(sd, &size, sizeof(int))) < 0) {
		         	 perror("[CLIENT] : Errore -> Impossibile leggere la dimensione del file");
					 close(fd);
		          	 break;
		         }
		         
		         // lettura del file
		         receivedBytes = 0;
		         while(receivedBytes < size) {
		         	// se i byte rimanenti da leggere sono di più di MAX_DIM_BUFF byte, allora il buffer sarà della sua dimensione.
						// in caso contrario verranno letti solo i byte rimanenti (< MAX_DIM_BUFF)
		         	if((nread = read(sd, buff, (size - receivedBytes > sizeof(buff)) ? sizeof(buff) : (size - receivedBytes))) <= 0) {
		         		 perror("[CLIENT] : Errore -> Il file non è stato ricevuto correttamente");
						 close(fd);
				          break;
		         	}
		         	receivedBytes += nread;
		         	
		         	if(write(fd, buff, nread) < 0) {
		               perror("[CLIENT] : Errore -> Impossibile scrivere il file sul file system");
					   close(fd);
		               break;
		            }

					close(fd);
		     		}	
		      }
        }		
        
        printf("[CLIENT] : Trasferimento terminato\n");
        close(sd);

        printf("\n[CLIENT] : Inserisci l'operazione (mput / mget), EOF per terminare: ");
    }
    
    printf("\n[CLIENT] : Termino l'esecuzione.\n");
    exit(0);
}
