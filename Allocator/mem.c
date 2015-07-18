#include "mem.h"
#include <stdio.h>
#include <stdlib.h>


void *mem_heap=0;
Liste liste_memoire=NULL; //pointeur vers la tête de la liste
unsigned long taille_struct_liste=sizeof(*liste_memoire);

//fonctions et structures internes au fichier





/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------  fonctions externes (qui seront utilisé en dehors de ce fichier)
  -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
int mem_init(){

  mem_heap = malloc(HEAP_SIZE);                                               //allocation de l'espace mémoire qu'on va faire pointer vers le premier maillon de notre liste circulaire.
  if (mem_heap == null)return 1;                               //cas ou l'allocation n'a pas fonctionné
  liste_memoire = mem_heap;
  lite_memoire->suiv=liste_memoire;                                    //on boucle sur un seul maillon
  liste_memoire->taille_cellule = HEAP_SIZE;                          //on met la taille de ce maillon a HEAP_SIZE
  return 0;
}


int mem_destroy(){                                                            //faire un free de mem_heap
  free(mem_heap);
  return 0;
}
                                                       

void *mem_alloc(unsigned long size){

  /*
    ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- a chaque fois on "divise" notre espace mémoire libre  en  multiple de sizeof(struct *liste_memoire) pour être sur que la zone mémoire qu'on a trouvé fasse au moins une taille de   size+sizeof(struct *liste_memoire) => Dans ce cas, on alloue size +eventuellement l'arrondi au multiple supérieur de sizeof(struct *liste_memoire),on sauvegarde l'adresse mémoire   vers laquelle pointe liste_memoire = liste_memoire suiv, on met liste_memoire à l'adresse mémoire suivante le size,et on parcourt la liste pour raccrocher la fin à liste_mem_suiv   ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  */ 

  Liste tempc = liste_memoire;//courrant
  Liste tempp=tempc;//precedent
  unsigned long taille_struct_liste=sizeof(*liste_memoire); //taille de notre stucture liste
  unsigned long espace_alloue=0; // size arrondi au multiple de taille_struct_liste supérieur
 
 
  /*gestion des arrondis dans la découpe
    ----------------------------------------------------------------------------------------------------------------------------------------------------------*/  
  if (size % taille_struct_liste==0) { //si size est déja un multiple de 16 (sizeof(*liste_memoire))
      espace_alloue=size;
    }
    else{ //sinon "(size/sizeof(*liste_memoire)+1)*sizeof(*liste_memoire)" est l'espace mémoire jusqu'au plus proche multiple de sizeof(*liste_memoire).
      espace_alloue = (unsigned long) (((size/taille_struct_liste)+1)*taille_struct_liste);
    }
    /*----------------------------------------------------------------------------------------------------------------------------------------------------*/
  
    /*cas ou notre liste est nulle ou cas d'une taille nulle à allouer  
      -----------------------------------------------------------------------------------------------------------------------------------------------------*/
    if(tempc==null || size ==0){
      return (void*) null;
    }
    /*----------------------------------------------------------------------------------------------------------------------------------------------------*/
  
    /*cas ou il n'y a qu'une case mémoire dans notre liste  
      -----------------------------------------------------------------------------------------------------------------------------------------------------*/
    if (tempc->suiv == liste_memoire){                      
      if(tempc->taille_cellule - espace_alloue < taille_struct_liste){  // et qu'elle est trop petite pour allouer la mémoire voulue et le nouveau maillon
	printf("impossible d'allouer de la mémoire supplémentaire,il n'y a plus assez de place") //alors on return
	  return (void*) null ;
      }
      else{                        //cas ou on alloue de la mémoire dans le seul maillon présent:
	tempc=(void*)(tempc + espace_alloue);//on déplace la tête de la liste à l'adresse de tempc+espace alloue
	tempc->taille_cellule=tempc->taille_cellule-espace_alloue;
	tempc->suiv=tempc;                  //on fait pointer la tête de liste vers elle même (liste circulaire).
      }
    }

    
    /*-----------------------------------------------------------------------------------------------------------------------------------------------------*/

    /*cas général de plusieurs maillon
      -----------------------------------------------------------------------------------------------------------------------------------------------------------*/
    else{      
      while( tempc->taille_cellule < espace_alloue || temp->suiv != liste_memoire){  //Si on a plusieurs cellules, on parcourt notre liste tant que la taille à allouer est plus grande que celle de la cellule courante ou qu'on est arrivé à l'avant dernier maillon.
	tempp=tempc;
	tempc=tempc->suiv;
      }
      //on a trouvé la cellule qui avait la bonne taille et qui est à l'adresse tempc
      if(tempc -> taille_cellule >= espace_alloue) {  //si on a la place d'allouer la mémoire demandée 
	
	if (tempc->taille_cellule - espace_alloue >= taille_struct_liste) {  //si en plus  on a la place de rajouter un maillon à la fin de l'espace mémoire 
	  tempp->suiv=(void*)(tempc + espace_alloue); //on recolle le maillon précédent a l'adresse mémoire suivant l'espace alloué.
	  tempp->suiv->taille_cellule=tempc->taille_cellule - espace_alloue;
	  tempp->suiv->suiv=tempc->suiv;  // on recolle le maillon de l'adresse mémoire suivant l'espace alloué au maillon suivant l'espace mémoire alloué.
	}
	tempp->suiv=tempc->suiv;
      }
      else{
	return (void*) null;
      }
    }
    return tempc;
    }                             
 
  int mem_free(void *zone, unsigned long size){
    /*---------------------------------------------------------------------------------------------------------------------------------------------------------
     *libére la zone libre concerné qui est a l'adresse zone et raccroche le maillon de la liste précédent au maillon suivant suivant. (on laisse la struct_suiv)
     ----------------------------------------------------------------------------------------------------------------------------------------------------------*/


 /*gestion des arrondis dans la découpe
   ----------------------------------------------------------------------------------------------------------------------------------------------------------*/  
    if (size % taille_struct_liste==0) { //si size est déja un multiple de 16 (sizeof(*liste_memoire))
      espace_alloue=size;
    }
    else{ //sinon "(size/sizeof(*liste_memoire)+1)*sizeof(*liste_memoire)" est l'espace mémoire jusqu'au plus proche multiple de sizeof(*liste_memoire).
      espace_alloue = (unsigned long) (((size/taille_struct_liste)+1)*taille_struct_liste);
    }
    /*----------------------------------------------------------------------------------------------------------------------------------------------------*/


    Liste tempc = liste_memoire;//courrant
    Liste tempp=tempc->suiv;//precedent
    Liste free_temp= NULL; //maillon qui va être libéré
 
    //cas pas de zone libre
    if(liste_memoire == null){
      liste_memoire = zone;
      liste_memoire->taille_cellule= espace_alloue;
      liste_memoire->suiv= liste_memoire;
    }
    //cas ou on sort de notre zone mémoire
    else if ( zone < mem_heap || size > HEAP_SIZE ||  size <= 0 || zone > (mem_heap + HEAP_SIZE)) {
      return 1;
    }
    //cas plusieurs maillon
    else {
      while ( tempc <= zone) { //tant que le maillon suivant de notre liste pointe vers une adresse memoire inférieure à celle de zone
	tempp=tempc;                 //on stocke l'adresse courante du pointeur avant de l'écraser par celle de la cellule suivante
	tempc=tempc->suiv;
      }
      free_temp=zone;
      free_temp->taille_cellule = espace_alloue;
      free(free_temp);
      tempp->suiv=zone;
      tempp->suiv->taille_cellule=espace_alloue;
      tempp->suiv->suiv=tempc;
      return 0;    
    }
  }

  
int mem_show(void (*print)(void *zone, unsigned long size)){
  /* 
   *parcort de la liste et a chaque fois on fait un print(liste_mem,liste_mem->size);
   */
  Liste tempc = liste_memoire;//courrant
  while(tempc->suiv != liste_memoire){
    print(tempc,tempc -> taille_cellule);
    tempc=tempc->suiv;
  }
  return 0;
}              

