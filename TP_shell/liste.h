#ifndef _LISTE_
#define _LISTE_

// Structure de liste des processus executés en background
struct Cellule{
  struct timeval start, startm; //start date de début du processus et end date de fin (à utiliser avec gettimeofday)
  pid_t pid;
  char cmd[100];
struct Cellule * suiv;};

typedef struct Cellule * Liste;

Liste ajoutTete(Liste l,pid_t pid,char* cmd);
void affiche(Liste l);
Liste metajour(Liste l);
double calc_tmp(Liste l);

#endif
