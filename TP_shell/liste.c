#include<stdio.h>
#include<stdlib.h>
#include "liste.h"
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <sys/time.h>
#include <time.h>

// Ajout d'un maillon en tête de la liste
Liste ajoutTete (Liste l,pid_t pid,char* cmd) {
  Liste nvMaillon = calloc(1,sizeof(*nvMaillon));
  nvMaillon->pid = pid;
  strcpy(nvMaillon->cmd,cmd);
  nvMaillon->suiv = l;
  //On gére la date de début du processus
  struct timeval tv;
  gettimeofday(&tv, NULL);
  nvMaillon->start.tv_sec=tv.tv_sec;
  nvMaillon->startm.tv_usec=tv.tv_usec;

  return nvMaillon;
}

// Affiche la liste des processus en cours d'execution (background) 
void affiche (Liste l){
  Liste temp = l;

  printf("-----------------------------------\n");
  printf("--      Liste des processus      --\n");
  printf("-----------------------------------\n");

  if (temp == NULL)
    printf("Pas de processus en cours d'execution\n");

  while (temp != NULL){
    printf("Processus: %s de pid %d \n",temp->cmd,temp->pid);
    temp = temp->suiv;
  }
  printf("-- Fin de la liste des processus --\n");
}

// Met à jour la liste des processus en cours en supprimant les processus qui se sont terminés
Liste metajour(Liste l) {
  int status; // Variable status pour le waitpid
  Liste prec, temp = NULL, list;
  
  // Si la liste est vide on ne fait rien
  if (l == NULL) return NULL;
  
  // Mise en place d'une sentinelle pour gérer les cas
  list = ajoutTete(l, 0, "sentinelle"); 
  prec = list;
  while (prec->suiv != NULL){
    // Si le processus s'est terminé on le supprime de la liste
    if(waitpid(prec->suiv->pid,&status,WNOHANG) == -1 ){   
      printf("\nLe processus %s est fini. ",prec->suiv->cmd);
      // On gére la date de fin du processus et on retourne le temps d'execution du processus
      double tmp = calc_tmp(l);
      printf("Il a duré: %lf s\n",tmp);

      // On racorde le maillon suivant
      temp = prec->suiv;
      prec->suiv = prec->suiv->suiv;
      // On libère la mémoire
      free(temp);
    }
    else prec=prec->suiv;  
  }

  //on enléve la sentinelle
  list = list->suiv; 
  return list;
}

// Fonction qui retourne le temps de calcul d'un processus (double)
double calc_tmp(Liste l){
  struct timeval end;
  double tvr_u,tvr,endr_u,endr;
  gettimeofday(&end,NULL);
  
  // Creation de notre double tvr qui est le réel de concaténation de start.tv_sec et de (0,start.tv_usec)  
  // Autrement dit: ("%ld.%ld",l->start.tv_sec,l->startm.tv_usec)
  tvr_u=l->startm.tv_usec;
  tvr_u=tvr_u/100000;
  tvr=l->start.tv_sec;
  tvr=tvr+tvr_u;
  // ...de même pour endr
  endr_u=end.tv_usec;
  endr_u=endr_u/100000;
  endr=end.tv_sec;
  endr=endr+endr_u;

  // On retorune le résultat, temps mis pour l'execution
  return (endr-tvr);
}
