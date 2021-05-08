#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define NB_VOITURES 15

char * voitures[] = {
    "audi_a1_blanche_2019",
    "audi_q5_grise_2021",
    "audi_r8_bleue_2019",
    "audi_rs6_noire_2020",
    "audi_serie1_blanche_2020",
    "bmw_i8_grise_2020",
    "bmw_m3_blanche_2021",
    "bmw_m4_noire_2020",
	"bmw_m5_rouge_2020",
    "bmw_serie8cabriolet_noire_2019",
    "ford_bronco_grise_2021",
    "ford_focus_noire_2020",
    "ford_fusion_grise_2019",
    "ford_mustang_bleue_2019",
    "ford_ranger_bleue_2020"
};


void* effectuer_recherche(void * arg){
	int sock = (intptr_t)arg;
	struct sockaddr_in client;
	size_t client_len = sizeof(client);
	char marque[10], modele[20], couleur[15];
	int i;
    int annee_fabrication;
    char str[5];
	char * erreur_msg = "Je ne dispose pas d'une image de cette voiture";
	char reponse[70] = "../voitures/";
	char nom_voiture[70];
	
	if (recv(sock, &marque, sizeof(marque), 0)<= 0)
		perror("Erreur recvfrom marque");
	strcat(reponse,marque);
	strcat(reponse,"_");

    if (recv(sock, &modele, sizeof(modele), 0) <= 0)
		perror("Erreur recvfrom modele");
    strcat(reponse,modele);
    strcat(reponse,"_");

    if (recv(sock, &couleur, sizeof(couleur), 0) <= 0)
		perror("Erreur recvfrom couleur");
	strcat(reponse,couleur);
    strcat(reponse,"_");

    if (recv(sock, &annee_fabrication, sizeof(annee_fabrication), 0) <= 0)
		perror("Erreur recvfrom annee_fabrication");
	sprintf(str, "%d", annee_fabrication);
    strcat(reponse,str);

	strcat(nom_voiture,marque); strcat(nom_voiture,"_");
	strcat(nom_voiture,modele); strcat(nom_voiture,"_");
	strcat(nom_voiture,couleur); strcat(nom_voiture,"_");
	strcat(nom_voiture,str);

	for(i=0; i<NB_VOITURES; i++){

		if(strcmp(nom_voiture,voitures[i]) == 0){
			strcat(reponse,".bmp");
			printf("%s\n", reponse);
			break;
		}
	}

	if(send(sock, reponse, sizeof(reponse),0) < 0)
		perror("Erreur sendto");
		
		return arg;
}

/**
   Initialise la structure sockaddr_in pour que la socket qui y sera liée
   puisse communiquer avec toutes les adresses ips possibles, sur le port 'port_serveur'
   @param serveur est un pointeur vers la structure que l'on souhaite initialiser
   @param port_serveur est le port auquel la socket sera liée.
   @require serveur != NULL
*/
void init_sockaddr_in(struct sockaddr_in * serveur, unsigned short port_serveur){
  	serveur->sin_family = AF_INET;
  	serveur->sin_port = htons(port_serveur);
  	serveur->sin_addr.s_addr = htonl(INADDR_ANY);
}

/**
   Initialise une socket permettant d'accepter des communications en UDP depuis n'importe
   qu'elle adresse IP.   
   @param port_serveur est le port auquel la socket sera liée.
   @return une socket prête à l'emploi si aucune erreur de survient, quitte le programme sinon.
*/
int initialiser_socket(struct sockaddr_in * serveur , unsigned short port_serveur ){
	//struct sockaddr_in serveur;
	int sock;
	init_sockaddr_in(serveur, port_serveur);
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Erreur lors de la création de socket TCP");
		exit(EXIT_FAILURE);
	}  
	if(bind(sock, (struct sockaddr *) serveur, sizeof(struct sockaddr_in)) < 0){
		perror("Erreur lors de l'appel à la fonction bind");
		exit(EXIT_FAILURE);
	}
	if(listen(sock,5)<0)
	{
		perror("Erreur lors de l'appel à la fonction listen");
		exit(EXIT_FAILURE);
	}
  	return sock;
}


/**
   Ce programme crée un serveur dédié qui renvoie l'heure aux clients qui en font la demande
   Le programme prend en paramètre le port sur lequel les communications seront effectuées.
*/
int main(int argc, char** argv) {
	struct sockaddr_in serveur;
    struct sockaddr_in client; 
    socklen_t  len = sizeof(client);
	short port = 0;
	int sock = 0; /* socket de connexion */
	int sock_pipe =0;/* socket de communication*/
	pthread_t th[3];
	if (argc!=2) {
		fprintf(stderr,"usage: %s port\n", argv[0]);
		return EXIT_FAILURE;
	}
	port = atoi(argv[1]);

	sock = initialiser_socket(&serveur , port);
	
	
		for(int i = 0; i<3 ; i++)
		{
			sock_pipe = accept(sock ,(struct sockaddr *)&client , &len ); 
			if(pthread_create(&th[i] , NULL , effectuer_recherche, (void *)(intptr_t)sock)<0)
			{
				perror("pthread");
				return EXIT_FAILURE;
			}
		}
		
		for(int i=0 ; i<3 ; i++)
		{
			pthread_join(th[i] , NULL);
		}
		close(sock_pipe);
	
	return EXIT_SUCCESS;
}
