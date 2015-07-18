/*
 * Copyright (C) 2002, Simon Nieuviarts
 *               2008, Matthieu Moy
 *               2010, Grégory Mounié
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include "readcmd.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "liste.c"
#include <fcntl.h>
#include <glob.h>
#ifdef USE_GNU_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

// Liste chainée de nos processus en BG (pour plus d'infos, cf liste.c & liste.h)
Liste listeProc = NULL;

/*
  Fonction qui gère l'execution d'un programme à partir de la commande passée en paramètre.
  
  Elle permet de remplacer (avec glob) les jokers présents dans les options de la commande.
  La variable pipe indique, dans le cas d'une commande avec un pipe, l'identifiant de la commande à executer (l->seq[pipe])
*/ 
void traiteProc(struct cmdline *l, int pipe) {

  glob_t globbuf; // Le pointeur glob_t qui va permettre le traitement des chaines de caractère avec pglob
  int i;          // Le compteur servant à parcourir le tableau des options de la commande

  // On construit le tableau gl_pathv en laissant une case d'en-tête pour le nom du processus (dans lequel on ne cherche pas de joker)
  globbuf.gl_offs = 1;
  
  // On vérifie que la commande comporte des options (là où chercher les jokers)
  if (l->seq[pipe][1] != 0) {
    // On parcourt les options de la commande...
    for (i = 1 ; l->seq[pipe][i] != 0 ; i++) {
      // ...en distinguant bien le premier cas pour le tableau globbuf
      if(i == 1) {
	glob((l->seq[pipe])[1], GLOB_NOCHECK | GLOB_DOOFFS | GLOB_TILDE_CHECK | GLOB_BRACE, NULL, &globbuf);
      }
      else {
	glob((l->seq[pipe])[i],GLOB_NOCHECK | GLOB_DOOFFS | GLOB_TILDE_CHECK | GLOB_BRACE | GLOB_APPEND, NULL, &globbuf);
      }
    }
    // On rajoute en tête du tableau gl_pathv le nom du programme dans lequel on n'a rien remplacé
    globbuf.gl_pathv[0]=l->seq[pipe][0];

    // On execute la commande avec les jokers remplacés
    if (execvp(globbuf.gl_pathv[0], &globbuf.gl_pathv[0]) == -1) {
      perror("Erreur de execvp");
    }
    globfree(&globbuf);

  }
  // Si elle n'a pas d'option on appelle simplement execvp
  else {
    if (execvp(l->seq[pipe][0], l->seq[pipe]) == -1) {
      perror("Erreur de execvp");
    
    }
  }	

}

/*
  Fonction principale du problème, elle gère la création d'un processus fils (voire 2 dans le cas d'un pipe simple), 
  la gestion des redirections entrée/sortie dans des fichiers, ainsi que l'execution des processus en background.
*/
void traiteCmd(struct cmdline *l){
  // Entiers pour les redirection entrée/sortie des fichiers
  int fin, fout;
  int tube[2];
  int status;

  // On execute pipe() avant l'appel de fork pour que le tube soit commun au père et au fils
  if (pipe(tube) == -1) { 
    perror("Erreur pipe"); 
    exit(EXIT_FAILURE); 
  }

  // On vérifie que la commande n'est pas vide
  if (l->seq[0] != 0) { 
    listeProc=metajour(listeProc);
    // Afficher la liste des processus dans le cas de la commande liste_ps
    if (!strcmp(l->seq[0][0], "liste_ps")) { 
      listeProc = metajour(listeProc);      
      affiche(listeProc);
    }
    else {
      // On fait appelle à fork() pour créer le processus correspondant à la commande
      pid_t pid = fork();

      // Si ca ne se passe pas bien...
      if (pid == -1)
	perror("Erreur de fork");

      // Sinon on est soit dans le processus fils
      else if (pid == 0) {
	// Si l'entrée processus doit lire dans un fichier (pas de pipe multiple donc pas de pipe en entrée)
	if (l->in) {
	  // Appel système pour ouvrir le fichier
	  fin = open(l->in, O_RDONLY);
	  if (fin == -1) {
	    perror("Erreur ouverture fichier");
	    exit(EXIT_FAILURE);
	  } 
	  else {  
	    dup2(fin, 0);
	  }
	}

	// On gère ici la sortie du processus : soit il y a un pipe...
	if (l->seq[1] != 0) {
	  // On ferme la sortie du tube dans le fils
	  close(tube[0]);
	  // On duplique l'entrée du tube sur la sortie standard du processus
	  dup2(tube[1], 1);
	  // On ferme l'entrée du tube
	  close(tube[1]);

	  // On remplace les jokers et execute la commande
	  traiteProc(l, 0);	
	}
	// ...soit il n'y en a pas
	else {
	  // Auquel cas, on vérifie si la sortie du processus doit écrire dans un fichier
	  if (l->out) {
	    // Appel système pour ouvrir le fichier dans lequel écrire (le créé le cas échéant)
	    fout = open(l->out, O_CREAT | O_TRUNC | O_RDWR, 0777);
	    if (fout == -1) {
	      perror("Erreur ouverture fichier");
	      exit(EXIT_FAILURE);
	    } else {  
	      dup2(fout, 1);
	    }
	  }
	  
	  // On remplace les jokers et execute la commande
	  traiteProc(l, 0);

	  // On referme le fichier de sortie si besoin
	  if (l->out) {
	    close(fout);
	  }
	}
	
	// Idem pour le fichier d'entrée
	if (l->in) {
	  close(fin);
	}
      }

      // Si on est dans le processus père
      else {
	// On regarde s'il y a un pipe, auquel cas on créé un autre processus
	if (l->seq[1] != 0) {
	  pid_t pidpipe = fork();
	  
	  if (pidpipe == -1)
	    perror("Erreur de fork");

	  // Si on est dans le deuxième processus fils, c'est à dire celui qui correspond à la deuxième commande du pipe
	  else if (pidpipe == 0){
	    // Si la sortie du processus doit écrire dans un fichier (pipe simple, donc pas besoin de s'occuper d'un deuxième pipe)
	    if (l->out) {
	      // Appel système pour ouvrir le fichier dans lequel écrire (le créé le cas échéant)
	      fout = open(l->out, O_CREAT | O_TRUNC | O_RDWR,0777);
	      if (fout == -1) {
		perror("Erreur ouverture fichier");
		exit(EXIT_FAILURE);
	      } else {  
		dup2(fout, 1);
	      }
	    }

	    // On ferme l'entrée du tube dans le fils
	    close(tube[1]);
	    // On duplique la sortie du tube sur l'entrée standard du processus
	    dup2(tube[0], 0);
	    // On ferme la sortie du tube
	    close(tube[0]);

	    // On remplace les jokers et execute la deuxième commande du pipe
	    traiteProc(l, 1);
						
	    // On referme le fichier ouvert s'il existe
	    if (l->out) {
	      close(fout);
	    }
	  }
	}
								

	// Gestion des tâches en background
	// Si pas d'execution en BG
	if(l->bg == 0){
	  // On dit au père d'attendre son fils
	  waitpid(pid,&status,0); 
	} 
	// Sinon on ajoute simplement les infos du processus (pid et nom du programme) dans la liste des processus
	else { 
	  listeProc = ajoutTete(listeProc,pid,l->seq[0][0]);
	  if (l->seq[1] != 0) // On n'oublie pas de le faire aussi pour la deuxième commande du pipe simple (pas de pipe multiple)
	    listeProc = ajoutTete(listeProc,pid,l->seq[1][0]);
	} 
      }
    }
  }
}

static void memory_error(void)
{
  errno = ENOMEM;
  perror(0);
  exit(1);
}


static void *xmalloc(size_t size)
{
  void *p = malloc(size);
  if (!p) memory_error();
  return p;
}


static void *xrealloc(void *ptr, size_t size)
{
  void *p = realloc(ptr, size);
  if (!p) memory_error();
  return p;
}

#ifndef USE_GNU_READLINE
/* Read a line from standard input and put it in a char[] */
static char *readline(char *prompt)
{
  size_t buf_len = 16;
  char *buf = xmalloc(buf_len * sizeof(char));

  printf(prompt);
  if (fgets(buf, buf_len, stdin) == NULL) {
    free(buf);
    return NULL;
  }

  do {
    size_t l = strlen(buf);
    if ((l > 0) && (buf[l-1] == '\n')) {
      l--;
      buf[l] = 0;
      return buf;
    }
    if (buf_len >= (INT_MAX / 2)) memory_error();
    buf_len *= 2;
    buf = xrealloc(buf, buf_len * sizeof(char));
    if (fgets(buf + l, buf_len - l, stdin) == NULL) return buf;
  } while (1);
}
#endif

#define READ_CHAR *(*cur_buf)++ = *(*cur)++
#define SKIP_CHAR (*cur)++

static void read_single_quote(char ** cur, char ** cur_buf) {
  SKIP_CHAR;
  while(1) {
    char c = **cur;
    switch(c) {
    case '\'':
      SKIP_CHAR;
      return;
    case '\0':
      fprintf(stderr, "Missing closing '\n");
      return;
    default:
      READ_CHAR;
      break;
    }
  }
}

static void read_double_quote(char ** cur, char ** cur_buf) {
  SKIP_CHAR;
  while(1) {
    char c = **cur;
    switch(c) {
    case '"':
      SKIP_CHAR;
      return;
    case '\\':
      SKIP_CHAR;
      READ_CHAR;
      break;
    case '\0':
      fprintf(stderr, "Missing closing \"\n");
      return;
    default:
      READ_CHAR;
      break;
    }
  }
}

static void read_word(char ** cur, char ** cur_buf) {
  while(1) {
    char c = **cur;
    switch (c) {
    case '\0':
    case ' ':
    case '\t':
    case '<':
    case '>':
    case '|':
      **cur_buf = '\0';
      return;
    case '\'':
      read_single_quote(cur, cur_buf);
      break;
    case '"':
      read_double_quote(cur, cur_buf);
      break;
    case '\\':
      SKIP_CHAR;
      READ_CHAR;
      break;
    default:
      READ_CHAR;
      break;
    }
  }
}

/* Split the string in words, according to the simple shell grammar. */
static char **split_in_words(char *line)
{
  char *cur = line;
  char *buf = malloc(strlen(line) + 1);
  char *cur_buf;
  char **tab = 0;
  size_t l = 0;
  char c;

  while ((c = *cur) != 0) {
    char *w = 0;
    switch (c) {
    case ' ':
    case '\t':
      /* Ignore any whitespace */
      cur++;
      break;
    case '&':
      w = "&";
      cur++;
      break;
    case '<':
      w = "<";
      cur++;
      break;
    case '>':
      w = ">";
      cur++;
      break;
    case '|':
      w = "|";
      cur++;
      break;
    default:
      /* Another word */
      cur_buf = buf;
      read_word(&cur, &cur_buf);
      w = strdup(buf);
    }
    if (w) {
      tab = xrealloc(tab, (l + 1) * sizeof(char *));
      tab[l++] = w;
    }
  }
  tab = xrealloc(tab, (l + 1) * sizeof(char *));
  tab[l++] = 0;
  free(buf);
  return tab;
}


static void freeseq(char ***seq)
{
  int i, j;

  for (i=0; seq[i]!=0; i++) {
    char **cmd = seq[i];

    for (j=0; cmd[j]!=0; j++) free(cmd[j]);
    free(cmd);
  }
  free(seq);
}


/* Free the fields of the structure but not the structure itself */
static void freecmd(struct cmdline *s)
{
  if (s->in) free(s->in);
  if (s->out) free(s->out);
  if (s->seq) freeseq(s->seq);
}


static struct cmdline *static_cmdline = 0;

void exitfreecmd(void)
{
  if ( static_cmdline ) {
    struct cmdline *s = static_cmdline;
    freecmd(s);
    free(s);
    static_cmdline=0;
  }
}

struct cmdline *readcmd(char *prompt)
{
  struct cmdline *s = static_cmdline;
  char *line;
  char **words;
  int i;
  char *w;
  char **cmd;
  char ***seq;
  size_t cmd_len, seq_len;

  line = readline(prompt);
  if (line == NULL) {
    if (s) {
      free(line);
      freecmd(s);
      free(s);
      assert(s == static_cmdline);
      static_cmdline = 0;
    }
    return static_cmdline = 0;
  }
#ifdef USE_GNU_READLINE
  else 
    add_history(line);
#endif

  cmd = xmalloc(sizeof(char *));
  cmd[0] = 0;
  cmd_len = 0;
  seq = xmalloc(sizeof(char **));
  seq[0] = 0;
  seq_len = 0;

  words = split_in_words(line);
  free(line);

  if (!s)
    {
      static_cmdline = s = xmalloc(sizeof(struct cmdline));
      atexit( exitfreecmd );
    }
  else
    freecmd(s);
  s->err = 0;
  s->in = 0;
  s->out = 0;
  s->seq = 0;
  s->bg = 0;

  i = 0;
  while ((w = words[i++]) != 0) {
    switch (w[0]) {
    case '<':
      /* Tricky : the word can only be "<" */
      if (s->in) {
	s->err = "only one input file supported";
	goto error;
      }
      if (words[i] == 0) {
	s->err = "filename missing for input redirection";
	goto error;
      }
      s->in = words[i++];
      break;
    case '>':
      /* Tricky : the word can only be ">" */
      if (s->out) {
	s->err = "only one output file supported";
	goto error;
      }
      if (words[i] == 0) {
	s->err = "filename missing for output redirection";
	goto error;
      }
      s->out = words[i++];
      break;
    case '&':
      /* Tricky : the word can only be "&" */
      if (cmd_len == 0) {
	s->err = "misplaced ampersand";
	goto error;
      }
      if (s->bg == 1) {
	s->err = "only one ampersand supported";
	goto error;
      }
      s->bg = 1;
      break;
    case '|':
      /* Tricky : the word can only be "|" */
      if (cmd_len == 0) {
	s->err = "misplaced pipe";
	goto error;
      }

      seq = xrealloc(seq, (seq_len + 2) * sizeof(char **));
      seq[seq_len++] = cmd;
      seq[seq_len] = 0;

      cmd = xmalloc(sizeof(char *));
      cmd[0] = 0;
      cmd_len = 0;
      break;
    default:
      cmd = xrealloc(cmd, (cmd_len + 2) * sizeof(char *));
      cmd[cmd_len++] = w;
      cmd[cmd_len] = 0;
    }
  }

  if (cmd_len != 0) {
    seq = xrealloc(seq, (seq_len + 2) * sizeof(char **));
    seq[seq_len++] = cmd;
    seq[seq_len] = 0;
  } else if (seq_len != 0) {
    s->err = "misplaced pipe";
    i--;
    goto error;
  } else
    free(cmd);
  free(words);
  s->seq = seq;
  return s;
 error:
  while ((w = words[i++]) != 0) {
    switch (w[0]) {
    case '<':
    case '>':
    case '|':
      break;
    default:
      free(w);
    }
  }
  free(words);
  freeseq(seq);
  for (i=0; cmd[i]!=0; i++) free(cmd[i]);
  free(cmd);
  if (s->in) {
    free(s->in);
    s->in = 0;
  }
  if (s->out) {
    free(s->out);
    s->out = 0;
  }
  return s;
}
