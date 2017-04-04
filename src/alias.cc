#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "alias.hh"

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
  node *l = list;
  if (l==NULL) {
    list = n;
  } else {
    while (l->next!=NULL) l = l->next;
    l->next = n;
  }
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

void Aliases::List() {
  node *n = list;
  node *q;
  while (n != NULL) {
    q = n->next;
    char yend[12];
    sprintf(yend, "....");
    if (q!=NULL) sprintf(yend, "%d", q->data->year - 1);
    printf("[%d-%s: %s ~ %s]", n->data->year, yend, n->data->name, n->data->nick);
    if (n->next != NULL) printf(" - ");
    n = n->next;
  }
  printf("\n");
}

AliasCollection::AliasCollection() {
  NC = 0;
  L = NULL;
}

int AliasCollection::Load(int n, const char *filename) {
  char s[2000], *tok[20], *ystr, *name, *nick;
  node *l;
  NC = n;
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  FILE *f = fopen("alias.dat", "rt");
  if (!f) return 0;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    if (feof(f)) continue;
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
      node *n = new node(a, NULL);
      if (k==0) {
        L[i]->list = n;
        l = n;
      } else {
        l->next = n;
        l = l->next;
      }
//      L[i]->Append(a);
      k++;
    }
    s[0] = 0;
  }
  fclose(f);
  return 1;
}

int AliasCollection::Save(const char *filename) {
  char backupname[100];
  sprintf(backupname, "%s.old", filename);
  rename(filename, backupname);
  FILE *f = fopen(filename, "wt");
  if (f==NULL) return 0;
  for (int i=0; i<NC; i++) {
      for (node *p=L[i]->list; p!=NULL; p=p->next) {
        if (p!=L[i]->list) {
          fprintf(f, "*");
        }
        if (p->data->nick==NULL) {
          fprintf(f, "%d %s", p->data->year, p->data->name);
        } else {
          fprintf(f, "%d %s~%s", p->data->year, p->data->name, p->data->nick);
        }
      }
      fprintf(f, "\n");
  }
  fclose(f);
  return 1;
}

void AliasCollection::List() {
  printf("%d\n", NC);
  for (int i=0; i<NC; i++) {
    printf("%4d:", i);
    L[i]->List();
  }
}

int AliasCollection::Add(alias *a) {
  Aliases **oldL = L;
  L = new Aliases*[NC+1];
  for (int i=0; i<NC; i++) {
    L[i] = oldL[i];
  }
  L[NC] = new Aliases();
  L[NC]->Append(a);
  NC++;
  return 1;
}
