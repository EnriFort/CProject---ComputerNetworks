/* ENRICO FORTUNA
 * 1872458 
 * fortuna.1872458@studenti.uniroma1.it */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#define PORT 5000
#define INF 9999
#define BUF_SIZE 100

/* Funzione che calcola il percorso minimo */
void min(int sockfd);

int main () {	
	
	int sockfd, sock_client, len;
	struct sockaddr_in server_addr, client_addr; // Struttura che contiene i dettagli del server
	
	len = sizeof(struct sockaddr_in);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf ("Errore nella creazione della socket.\n");
		return 1;
	}
	
	printf("Socket creata correttamente\n");
	
	bzero((char *) &server_addr, sizeof(server_addr));
	
	server_addr.sin_addr.s_addr = INADDR_ANY; // Si rimane in ascolto su ogni indirizzo
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	
	/* Togliere commento per permettere al server di essere eseguito più volte di seguito 
	int yes=1;
	if ( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1 ) {
		perror("setsockopt");
		exit(1);*/
	}

	if ( bind(sockfd, (struct sockaddr *) &server_addr, 
		sizeof(server_addr)) < 0 ) { 
		printf ("Errore durante il bind.\n");
		return 1;
	}	
	if ( listen(sockfd, 1) < 0) {
		printf ("Errore durante la listen.\n");
		return 1;
	}
	printf ("In ascolto.\n");

	sock_client = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&len);

	if (sock_client < 0) {
		 printf ("Errore durante accept.\n");
		return 1;
	}
	printf("Client connesso...");
	min(sock_client);
	close(sockfd);
	return 0;
}

void min(int sockfd) {
	
	/* Messaggio ricevuto dal client */
	char *buff = (char*)malloc(BUF_SIZE*sizeof(char));
	
	/* Messaggio di risposta del server */
	char *msg_to_client = malloc(sizeof(char));
	
	for (;;) {
		bzero(buff, BUF_SIZE);
		if ( read(sockfd, buff, BUF_SIZE) < 0 ) {
			 printf ("Errore ricezione.\n");
			exit(1);
		}
		/* Controllo il messaggio del client */	
		if (strlen(buff) > 0) { // Se il messaggio ricevuto non è vuoto:
			
			/* 1. Se riceve il messaggio "exit" dal client, termina */
			if (strcmp(buff, "exit\n") == 0) {
				close (sockfd);
				exit(0);
			}
			/* 2. Se riceve il comando min:ID_NODO_SRC,ID_NODO_DES */
			else if (buff[0] == 'm' && buff[1] == 'i'
				&& buff[2] == 'n' && buff[3] == ':') {
					
				/* Se per esempio il messaggio è min:2,6 */	
			
				char* p = strtok(buff, ":"); // spezzo la stringa in "min" e "2,6"
				p = strtok(NULL," ");        // ora p contiene "2,6"
				
				char* nodo = strtok(p, ","); // spezzo p in "2" e "6"
				int nodo_src = atoi(nodo);   // nodo = "2"
				nodo = strtok(NULL," ");     
				int nodo_dst = atoi(nodo);	 // nodo = "6"
				
				FILE* file = fopen("rete.txt", "r");
				int n_nodi = 0;
				char *riga = malloc(BUF_SIZE*sizeof(char));
				/* Numero nodi = numero righe */
				while(fgets(riga, BUF_SIZE, file)){
					if (strlen(riga) > 1) {  // salta le righe vuote, se ci sono
						n_nodi++;
					}
				}
				rewind(file); // rimetto il cursore all'inizio del file
				
				/* Creo la matrice dei costi: 
				inizialmente setto tutti i costi a infinito, 
				poi li aggiornerò succesivamente */
				int matr[n_nodi][n_nodi];	
				for (int i = 0; i < n_nodi; i++) {
					for (int j = 0; j < n_nodi; j++) {
						matr[i][j] = INF;
					}
				}
					
				int save, pes, vic, len, i, flag_src = 0, flag_dst = 0;
				char *vicino, *peso;
				memset(riga, 0, strlen(riga));
				
				while (fgets(riga, BUF_SIZE, file)) {
					int lunghezza_riga = strlen(riga);
					/* Se la riga non è vuota */
					if (lunghezza_riga > 1) {
						/* In ogni riga il primo carattere indica il nodo */
						
						save = riga[0] - '0';// converto in int 	
						len = 1;
						i = 2;               // Parto dal secondo carattere 
						
						/* I flag mi servono per controllare se esistono
						i nodi chiesti dal client */
						if(save == nodo_src)
							flag_src = 1;
						if (save == nodo_dst) 
							flag_dst = 1;
							
						save = save -1;
						while(i < lunghezza_riga - 1) {	
							
							vicino = NULL;
							peso = NULL;

							while(riga[i] != ',') {
								/* Fino a quando non trovo la virgola scorro la
								riga e mi creo la stringa contenente il nodo vicino */				
								vicino = (char*)realloc(vicino, len*sizeof(char)+1);
								vicino[len-1] = riga[i];
								len++;
								i++;
							}
							
							vicino[len-1] = '\0';
							len = 1;
							i++; // salto la virgola 
						
							while(riga[i] != ' ') {
								/* Fino a quando non trovo lo spazio scorro la
								riga e mi creo la stringa contenente il peso dell'arco */	
								peso = (char*)realloc(peso, len*sizeof(char)+1);
								peso[len-1] = riga[i];
								len++;
								i++;
							}
							peso[len-1] = '\0';
							i++; // salto lo spazio
							len = 1;
							
							/* Converto le stringhe in interi */
							vic = atoi(vicino) - 1;
							pes = atoi(peso);
							
							/* Aggiorno il la matrice dei costi */
							matr[save][vic] = pes;
							free(vicino);
							free(peso);	
						}
						memset(riga, 0, strlen(riga));	
					}
				}
				free(riga);
				fclose(file);
				
				/* Se il nodo sorgente e destinazione richiesti
				dal client esistono cerco il percorso minimo */ 
				if (flag_src == 1 && flag_dst == 1) {
											
					/* Per calcolare il percorso a costo minimo
					utilizzo un'implementazione dell'algoritmo di Dijkstra */
					
					int startnode = nodo_src - 1; // nel grafo i nodi partono da 1, nell'algoritmo da 0
					int dist[n_nodi];             // vettore delle distanze
					int pred[n_nodi];             // pred[] contiene i predecessori di ogni nodo
					
					/* Conta indica il numero di nodi visti finora */
					int vis[n_nodi], conta, dist_min, succ, j; 
									
					/* Inizializzazione di pred[], dist[] e vis[] */
					for(int i = 0; i < n_nodi; i++) {
						dist[i] = matr[startnode][i];
						pred[i] = startnode;
						vis[i] = 0;
					}
					dist[startnode] = 0;
					vis[startnode] = 1;
					conta = 1;
					
					while(conta < n_nodi -1) {
						
						dist_min = INF;
						/* Succ contiene il nodo alla distanza minima */
						for(int i = 0; i < n_nodi; i++) {
							if(dist[i] < dist_min && !vis[i]) {
								/* Aggiorno la distanza minima */
								dist_min = dist[i];
								succ = i;
							}
							/* Controlla se esiste un percorso migliore tramite succ */			
							vis[succ] = 1;
							for(int i = 0; i < n_nodi; i++){
								if(!vis[i]){
									if(dist_min + matr[succ][i] < dist[i]) {
										dist[i] = dist_min + matr[succ][i];
										pred[i] = succ;
									}
								}
							}
						}
						conta++;
					}
				 
					/* Se non esiste un percorso */
					if (dist[nodo_dst-1] == INF) {
						
						char risposta[] = "percorso non esistente\n";
						msg_to_client = realloc(msg_to_client, strlen(risposta)*sizeof(char));
						strcpy(msg_to_client, risposta);
						
					}
					else {
					
					
						/* Nell'algoritmo i nodi partono da 0 invece che 1, quindi decremento:
						se per esempio il nodo destinazione è 6, nell'algoritmo invece è 5 */
						j = nodo_dst - 1; 
						char *path = calloc(1,sizeof(char)); // stringa contenente il percorso
						int k = 0;
						char c[] = "1,";
						/* Mi creo la stringa path */ 
						do {
							k++;
							j=pred[j];
							c[0] = (j+1) + '0';      // converto il nodo intero in carattere
							path = (char*)realloc(path, k*sizeof(char));
							strcat(path, c);		 
						} while(j != startnode);
						
						path[strlen(path)-1] = '\0'; // tolgo la virgola finale
						
						int begin, end, count;
						count = strlen(path);
						char real_path[count + 1];   // la stringa che contiene il percorso al contrario (quello giusto)
						end = strlen(path) - 1;
						/* Inverto la stringa*/
						for (begin = 0; begin < count; begin++) {
							real_path[begin] = path[end];
							end--;
						}
						real_path[begin] = '\0';
						free(path);
						
						char *buf;
						size_t sz;
						sz = snprintf(NULL, 0, "il percorso a costo minimo tra %d e %d è [", nodo_src, nodo_dst);
						buf = (char *)malloc(sz + 1); 
						snprintf(buf, sz+1, "il percorso a costo minimo tra %d e %d è [", nodo_src, nodo_dst);
						
						char *risposta;
						sz = snprintf(NULL, 0, ",%d] con costo: %d \n", nodo_dst, dist[nodo_dst-1]);
						risposta = (char *)malloc(sz + 1);
						snprintf(risposta, sz+1, ",%d] con costo: %d \n", nodo_dst, dist[nodo_dst-1]);
						
						/* Messaggio di risposta del server */
						msg_to_client = realloc(msg_to_client, (strlen(path) + strlen(buf) + strlen(risposta))*sizeof(char));
						
						strcpy(msg_to_client, buf);
						strcat(msg_to_client, real_path);
						strcat(msg_to_client, risposta);
						
						free(buf);
						free(risposta);
					}
				} 
				/* Se non esiste uno dei due nodi richiesti dal client */
				else {
					
					char risposta[] = "uno o entrambi i nodi non esistono\n";
					msg_to_client = realloc(msg_to_client, strlen(risposta)*sizeof(char));
					strcpy(msg_to_client, risposta);
					
				}
				
			}
			
			/* Se il server riceve un messaggio non corretto */
			else {
				char risposta[] = "ERR-Formato non corretto\n";
				msg_to_client = realloc(msg_to_client, strlen(risposta)*sizeof(char));
				strcpy(msg_to_client, risposta);
			}
		}
		/* Mando la risposta al client */
		if ( write(sockfd, msg_to_client, strlen(msg_to_client)) < 0 ) {
			 printf ("Errore invio messaggio.\n");
			exit(0);	
		}
	}
	free(msg_to_client);
	free(buff);
}
			
		
