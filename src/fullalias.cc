#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int NC;
char **club;
char **mnem;

struct alias {
  int   year;
  char *name;
  char *nick;
  alias(int, char*, char*);
  ~alias();
};

struct node {
  alias *data;
  node  *next;
  node(alias*, node*);
  ~node();
};

struct Aliases {
  node *list;
  Aliases();
  ~Aliases();
  void Append(alias *a);
  char* GetName(int y);
  char* GetNick(int y);
};

//-------------------------------------

alias::alias(int y, char *s, char *n) {
  year = y;
  name = (char*) malloc(strlen(s)+1);
  strcpy(name, s);
  if (n != NULL) {
    nick = (char*) malloc(strlen(n)+1);
    strcpy(nick, n);
  }
  else nick = NULL;
};

alias::~alias() {
  if (name) delete name;
  if (nick) delete nick;
};

node::node(alias *a, node *n) {
  data = a;
  next = n;
};

node::~node() {
  if (next) delete next;
  delete data;
};

Aliases::Aliases() {
  list = NULL;
};

Aliases::~Aliases() {
 delete list;
}

void Aliases::Append(alias *a) {
  node *n = (node*) malloc(sizeof(node));
  n->data = a;
  n->next = list;
  list = n;
};

char* Aliases::GetName(int y) {
  if (!list) return NULL;
  node *n = list;
  char *s = n->data->name;
  int x = n->data->year;
  while (y < x && n->next != NULL) {
    n = n->next;
    s = n->data->name;
    x = n->data->year;
  }
  return s;
}

char* Aliases::GetNick(int y) {
  if (!list) return NULL;
  node *n = list;
  char *s = n->data->nick;
  int x = n->data->year;
  while (y < x && n->next != NULL) {
    n = n->next;
    s = n->data->nick;
    x = n->data->year;
  }
  return s;
}

Aliases **L;

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[32];
    memmove(mnem[i], s, 15); mnem[i][15] = 0;
    for (int j=14; j>0 && mnem[i][j]==' '; j--) mnem[i][j]=0;
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    if (!s) continue;
    if (strlen(s) < 3) continue;
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, "*");
    for (int j=1; j<20; j++)
      tok[j] = strtok(NULL, "*");
    int k=0;
    while(tok[k]) {
      ystr = strtok(tok[k], " ");
      name = strtok(NULL, "~");
      nick = strtok(NULL, "@");
      int y = atoi(ystr);
      alias *a = new alias(y, name, nick);
      L[i]->Append(a);
      k++;
    }
    s[0] = 0;
  }
  fclose(f);
}


void Dump(int rw) {
  FILE *f;
  char nck[100];
  if (rw) f = fopen("alias.full", "wt");
  for (int i=0; i<NC; i++) {
    if (L[i]->list==NULL) {
      if (rw) fprintf(f,"1800 %s~%s\n", club[i], mnem[i]);
      else printf("1800 %s~%s\n", club[i], mnem[i]);
    }
    else {
      // reverse
      Aliases *rev = new Aliases;
      for (node *p=L[i]->list; p!=NULL; p=p->next) {
         rev->Append(p->data);
      }      
      for (node *p=rev->list; p!=NULL; p=p->next) {
        if (p!=rev->list) {
          if (rw) fprintf(f, "*"); else printf("*");
        }
        if (p->data->nick==NULL) {
          if (rw) fprintf(f, "%d %s", p->data->year, p->data->name);
          else printf("%d %s", p->data->year, p->data->name);
        }
        else {
          if (rw) fprintf(f, "%d %s~%s", p->data->year, p->data->name, p->data->nick);
          else printf("%d %s~%s", p->data->year, p->data->name, p->data->nick);
        }
      }
      if (rw) fprintf(f, "\n"); else printf("\n");
      delete rev;
    }
  }
  if (rw) {
    fclose(f);
    rename("alias.full", "alias.dat");
  }
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  int rewrite = 0;
  if (argc > 1 && strcmp(argv[1], "-r")==0) rewrite = 1;
  Load();
  Dump(rewrite);
  return 1;
}
