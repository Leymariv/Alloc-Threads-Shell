#ifndef __MEM_ALLOC_H
#define __MEM_ALLOC_H

#include <stddef.h>

#define MAX_INDEX 20
#define HEAP_SIZE (1 << MAX_INDEX)

/***
 *   Definir l'algorithme à implanter
 *   Decommenter la bonne ligne
 ***/
// #define BUDDY
// #define CFF


/*** Ne rien editer apres cette ligne ***/

#ifndef CFF
#ifndef BUDDY

#error "Vous devez definir la macro (#define) pour le sujet que vous implanter, quelques lignes au dessus de la macro affichant cette erreur)"

#endif
#endif

//structure de liste

struct cellule{
  unsigned long taille_cellule;
  struct cellule *suiv;} ;

typedef struct cellule *Liste;

extern void *mem_heap; // variable globale stockant l'addresse de la zone (utile pour le shell)

int mem_init();
int mem_destroy();

void *mem_alloc(unsigned long size);
int mem_free(void *zone, unsigned long size);

int mem_show(void (*print)(void *zone, unsigned long size));

#endif
