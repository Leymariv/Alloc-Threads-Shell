/*
 * Copyright (C) 2002, Simon Nieuviarts
 *               2010, Gregory Mounie
 */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define VARIANTE "9- Joker étendus et Temps de calcul"

int main()
{
  printf("%s\n", VARIANTE);
  while (1) {
    struct cmdline *l;
    int i, j;
    char *prompt = "ensishell>";

    l = readcmd(prompt);

    /* If input stream closed, normal termination */
    if (!l) {
      printf("exit\n");
      exit(0);
    }

    if (l->err) {
      /* Syntax error, read another command */
      printf("error: %s\n", l->err);
      continue;
    }
    if (l->in) printf("in: %s\n", l->in);
    if (l->out) printf("out: %s\n", l->out);
    if (l->bg) printf("background (&)\n");

    /* Display each command of the pipe */
    for (i=0; l->seq[i]!=0; i++) {
      char **cmd = l->seq[i];
      printf("seq[%d]: ", i);
      for (j=0; cmd[j]!=0; j++) {
	printf("'%s' ", cmd[j]);
      }
      printf("\n");
    }
    
    // Appel de traiteCmd qui va gérer la commande entrée 
    traiteCmd(l);	
  }
}
