#include "mem.h"
#include <stdlib.h>
#include <stdio.h>




/* Déclaration des varibles
 * ------------------------
 * > mem_heap : pointeur vers le début de la zone mémoire allouée
 * > pLibre : pointeur de type Liste qui pointera tout au long du code vers un maillon de notre liste
 * > taille_min : taille de notre strucure Liste
 * ------------------------
 */
void *mem_heap=NULL;
Liste *pLibre=NULL;
//unsigned long taille_min=(sizeof(unsigned long) + sizeof(void *));
unsigned long taille_min=(sizeof(Liste));




/* Fonction d'initialisation
 * -------------------------
 * Cette fonction permet d'allouer la mémoire avec malloc et d'initialiser notre liste
 * ------------------------
 */
int mem_init() {

  // Alloue la mémoire et enregistre l'adresse dans mem_heap */
  mem_heap = malloc(HEAP_SIZE);
  // Retourne 1 si l'allocation s'est mal passée
  if (mem_heap==NULL) return 1;

  // Initialise les listes enregistrant la zone libre
  pLibre=mem_heap;

  // On prend des précaution si le pointeur est NULL
  if (pLibre != NULL) {
    pLibre->taille = (unsigned long) (HEAP_SIZE);
    pLibre->suivant = pLibre; // Pointe vers lui même (Liste cirutclaire)
  }

  return 0;
}




/* Fonction desallocation
 * -------------------------
 * Cette fonction permet de désallouer notre mémoire
 * ------------------------
 */
int mem_destroy() {
  
  // supprime le pointeur vers la liste et donc toute la mémoire allouée
  free(mem_heap);
  
  return 0;
}




/* Fonction d'allocation
 * ---------------------
 * Fonction retournant l'adresse où on alloue une zone mémoire de taille tailleZone
 * ---------------------
 */
void *mem_alloc(unsigned long tailleZone) {

  /* Déclaration des variables
   * -------------------------
   * > pLibre_tmp : pointeur de type liste permettant de parcourir notre liste
   * > add_tmp : pointeur "sans type" permettant de travailler directement avec adresse en octet
   * > tailleZoneMin : taille multiple supérieur de la taile de la zone que l'on veut allouer
   * -------------------------
   */
  Liste *pLibre_tmp=NULL;
  void *add_tmp = NULL;
  unsigned long tailleZoneMin;


  // Arrondie la taille de la zone à allouée à un multiple supérieur de la taille mininum (16 octets)
  if ((tailleZone % taille_min)==0) {
    tailleZoneMin = tailleZone;
  } else{
    tailleZoneMin= ((tailleZone/taille_min)+1)*taille_min; 
  }


  // On test si pLibre est pas NULL ou si la taille de la zone à allouer n'est pa null
  if (pLibre == NULL || tailleZone == 0) {
    return (void *) NULL;

  } else {

    pLibre_tmp=pLibre->suivant;

    // Recherche une zone libre dans la mémoire de taille supérieur à tailleZoneMin
    while ((pLibre->suivant)->suivant != pLibre_tmp && (pLibre->suivant)->taille < tailleZoneMin) {
      pLibre = pLibre->suivant;
    }
    
    pLibre_tmp = pLibre->suivant;

    // Test s'il y a assez de place dans la zone pointée par pLibre_tmp
    if (pLibre_tmp->taille >= tailleZoneMin) {
      
      // Test s'il restera un minimun de zone libre dans la zone que l'on va allouer pour enregister un maillon
      if ((pLibre_tmp->taille - tailleZoneMin) >= taille_min) {
	
	// On distingue le cas où il y a un seul maillon
	if (pLibre_tmp==pLibre_tmp->suivant) {
	  
	  // Cas un seul maillon
	  add_tmp = (void*) pLibre_tmp;
	  pLibre = (void *) (add_tmp + tailleZoneMin);
	  pLibre->taille = (pLibre_tmp->taille - tailleZoneMin);
	  pLibre->suivant = pLibre;

	} else {

	  // Cas plusieurs maillons
	  add_tmp = (void *) pLibre_tmp;
	  pLibre->suivant = (void *) (add_tmp + tailleZoneMin);
	  pLibre = pLibre->suivant;
	  pLibre->taille = (pLibre_tmp->taille - tailleZoneMin);
	  pLibre->suivant = pLibre_tmp->suivant;
	  
	}
	
	// Retourne l'adresse de la zone mémoire allouée
	return (void *) pLibre_tmp;

      } else {
	// Plus de place pour enregistrer la taille qui reste en mémoire et le pointeur de la zone libre suivante	


	// On distingue le cas où il y a un seul maillon
	if (pLibre == pLibre->suivant) {
	  
	  // Cas un seul maillon
	  pLibre = (void *) NULL;

	} else {

	  // Cas plusieurs maillons
	  pLibre->suivant = (pLibre->suivant)->suivant;
	}

	// Retourne l'adresse de la zone mémoire allouée 
	return (void *) pLibre_tmp;
      }

    } else {

      // Plus de place en mémoire
      return (void *) NULL;
    }
  }
}




/* Fonction de désallocation
 * ---------------------
 * Fonction retournant 0 ou 1 si la fonction s'est exécutée correctement
 *    - 0 : déroulement correct
 *    - 1 : déroulement incorrect
 * ---------------------
 */
int mem_free(void *zone, unsigned long size) {

  /* Déclaration des variables
   * -------------------------
   * > pLibre_tmp : pointeur de type liste permettant de parcourir notre liste
   * > add_tmp : pointeur "sans type" permettant de travailler directement avec adresse en octet
   * > tailleZone : taille multiple supérieur de la taille de la zone que l'on veut allouer
   * > pLibre_tmp_1: pointeur de type liste permettant de parcourir une deuxieme liste
   * > p_new : pointeur de type liste pour enregistrer la nouvelle zone libre
   * -------------------------
   */  
  Liste *pLibre_tmp= pLibre;
  Liste *pLibre_tmp_1=NULL;
  Liste *p_new= zone;
  void *add_tmp = (void *) pLibre;
  unsigned char zone_trouvee = 0;
  unsigned long tailleZone;

  // Arrondie la taille de la zone à allouée à un multiple supérieur de la taille mininum
  if ((size % taille_min)==0) {
    tailleZone = size;
  } else{
    tailleZone = ((size/taille_min)+1)*taille_min; 
  }

  // Cas ou il n'y a pas de zone libre
  if (pLibre == NULL) {

    pLibre = zone;
    pLibre->taille = size;
    pLibre->suivant = pLibre;


    // Cas limites (on sort de notre zone mémoire, taille inférieure à 0...)
  } else if ( zone < mem_heap || size > HEAP_SIZE ||  size <= 0 || zone > (mem_heap + HEAP_SIZE)) {

    return 1;

    // cas général
  } else {

    // On parcourt la liste pour fusionner les zones libres contigues
    while (pLibre_tmp->suivant!=pLibre && !(zone_trouvee)) {

      // on regarde s'il y a une structure libre juste derriere (à droite) notre zone 
      if (pLibre_tmp->suivant==(tailleZone+zone)) { 

	// Fusionne la zone libre pointée par zone avec celle adjacente en mémoire
	p_new->taille = (((pLibre_tmp->suivant)->taille)+tailleZone);
	p_new->suivant=(pLibre_tmp->suivant)->suivant;
	pLibre_tmp->suivant=p_new;
	// On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
	pLibre = p_new;
	// Equivalent d'un boolean pour sortir du while une fois la fusion faite
	zone_trouvee=1;

	// Nouveau pointeur temporaire
	pLibre_tmp_1 = p_new;

	
	/* Maintenant que l'on à fusionner la zone à droite, on regarde si on ne peut pas fusionner la zone à gauche*/
	
	// On parcourt la liste pour fusionner les zones libres contigues a gauche de la zone pointée par zone jusqu'à trouver une zone contigue ou avoir tout parcouru
	while ((add_tmp + pLibre_tmp_1->taille) != zone && pLibre_tmp_1->suivant!=p_new) {

	  pLibre_tmp_1 = pLibre_tmp_1->suivant;
	  add_tmp = (void *) pLibre_tmp_1;
	}
	
	// On vérifie bien qu'on a trouvé la zone et pas seulement que l'on a parcouru toute la liste
	if ((add_tmp + pLibre_tmp_1->taille) == zone) {
	  
	  pLibre_tmp->suivant = p_new->suivant;
	  (pLibre_tmp_1->taille) += (p_new->taille);
	  // On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
	  pLibre = pLibre_tmp_1;
	}
	  

      } else if ((add_tmp + pLibre_tmp->taille) == zone) { 
	//on regarde cette fois-ci s'il y a une zone mémoire libre à gauche de notre zone 

	// On a juste à changer la taille de notre zone
	(pLibre_tmp->taille)+=tailleZone; 

	// Boolean qui permet d'arreter notre while
	zone_trouvee=1;
	// On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
	pLibre = pLibre_tmp;

	// Nouveau pointeur temporaire
	pLibre_tmp_1 = pLibre_tmp;
	
	// On regarde s'il y a une zone mémoire à fusionner à droite en parcourant toute la liste
	while ((pLibre_tmp_1->suivant != (tailleZone+zone)) && (pLibre_tmp_1->suivant != pLibre_tmp)) {

	  pLibre_tmp_1 = pLibre_tmp_1->suivant ;

	}

	// On vérifie bien qu'on a trouvé la zone et pas seulement que l'on a parcouru toute la liste
	if (pLibre_tmp_1->suivant == (tailleZone+zone)) {
	    
	  (pLibre_tmp->taille) += (pLibre_tmp_1->suivant)->taille;
	  pLibre_tmp_1->suivant = (pLibre_tmp_1->suivant)->suivant;
	  // On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
	  pLibre = pLibre_tmp;
	}

      }

      // On parcourt notre liste
      pLibre_tmp=pLibre_tmp->suivant;
      add_tmp = (void *) pLibre_tmp;
    }

    
    /* La boucle while a permis de tester toute la liste sauf le premier élément qui était pointé par pListe  */

    // on regarde s'il y a une structure libre juste derriere notre zone (à gauche)
    if (pLibre_tmp->suivant==(zone+tailleZone)) {
    
      p_new->taille=(((pLibre_tmp->suivant)->taille)+tailleZone);

      // On distingue le cas où il y a un seul maillon du cas général
      if (pLibre_tmp->suivant == (pLibre_tmp->suivant)->suivant) {
      
	// cas un seul maillon
	p_new->suivant = p_new;

      } else {

	// cas général
	p_new->suivant=(pLibre_tmp->suivant)->suivant;
      }

      pLibre_tmp->suivant=p_new;
      // Equivalent d'un boolean pour sortir du while une fois la fusion faite
      zone_trouvee=1;
      // On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
      pLibre = p_new;
 
      // Nouveau pointeur temporaire
      pLibre_tmp_1 = p_new;

      // On regarde s'il y a une zone mémoire à fusionner à droite en parcourant toute la liste
      while ((add_tmp + pLibre_tmp_1->taille) != zone && pLibre_tmp_1->suivant!=p_new) {

	pLibre_tmp_1 = pLibre_tmp_1->suivant;
	add_tmp = (void *) pLibre_tmp_1;
	
      }

      // On vérifie bien qu'on a trouvé la zone et pas seulement que l'on a parcouru toute la liste
      if ((add_tmp + pLibre_tmp_1->taille) == zone) {
	  
	pLibre_tmp->suivant = p_new->suivant;
	(pLibre_tmp_1->taille) += (p_new->taille);
	// On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
	pLibre = pLibre_tmp_1;
      }


	/* Maintenant que l'on à fusionner la zone à droite, on regarde si on ne peut pas fusionner la zone à gauche*/
	
	// On parcourt la liste pour fusionner les zones libres contigues a gauche de la zone pointée par zone jusqu'à trouver une zone contigue ou avoir tout parcouru
    } else if ((add_tmp + pLibre_tmp->taille) == zone) { 

      (pLibre_tmp->taille)+=tailleZone; 
      
      zone_trouvee=1;
      // On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
      pLibre = pLibre_tmp;
      pLibre_tmp_1 = pLibre_tmp;
	
      // On regarde sil y a une zone mémoire à fusionner à droite
      while ((pLibre_tmp_1->suivant != (tailleZone+zone)) && (pLibre_tmp_1->suivant != pLibre_tmp)) {

	pLibre_tmp_1 = pLibre_tmp_1->suivant ;
      }

      // On vérifie bien qu'on a trouvé la zone et pas seulement que l'on a parcouru toute la liste
      if (pLibre_tmp_1->suivant == (tailleZone+zone)) {
	    
	(pLibre_tmp->taille) += (pLibre_tmp_1->suivant)->taille;
	pLibre_tmp_1->suivant = (pLibre_tmp_1->suivant)->suivant;
	// On définit le pointeur de notre liste vers le nouveau maillon que l'on vient de créer
	pLibre = pLibre_tmp;
      }

    }

    /* Si jamais on n'a pas pu fusionner il faut rajouter la nouvelle zone dans la liste */
    if (!(zone_trouvee)) {
      p_new->suivant=pLibre->suivant;
      pLibre->suivant=p_new;
      p_new->taille=tailleZone;
    }
  }
    return 0;
}




/* Fonction de d'affichage
 * ---------------------
 * Fonction utilisée par le shell pour afficher les zones libres
 * ---------------------
 */
int mem_show(void (*print)(void *zone, unsigned long size)) {

  /* Déclaration des variables
   * -------------------------
   * > i : un compteur qui affiche le numéro des zones
   * > pLibre_tmp : pointeur de type liste qui permet de parcourir la liste
   * -------------------------
   */  
  int i = 1;
  Liste *pLibre_tmp = pLibre;

  // Distingue le cas d'une liste vide et le cas général
  if (pLibre != NULL) {
    
    printf("** Affichage des Zones libres **\n");
    printf(" - Zones libres n°%d\n", i);
    print((void *) pLibre_tmp, pLibre_tmp->taille);  
    pLibre_tmp = pLibre_tmp->suivant;
    i++;     

    // Boucle while pour parcourir toute la liste
    while(pLibre_tmp != pLibre) {
      printf(" - Zones libres n°%d\n", i);
      // Utilise la fonction dans le programme de test
      print((void *) pLibre_tmp, pLibre_tmp->taille);   
      
      pLibre_tmp = pLibre_tmp->suivant;
      i++;      

      printf("\n");
    }
  } else {

    // si la liste est vide cela signifie qu'il n'y a plus de place en mémoire
    printf("** Affichage des Zones libres **\n\n");
    printf("** Il n'y a plus de Zone Libre **\n");

  }

  return 0;
}
