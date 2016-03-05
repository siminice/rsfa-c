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
  char s[500], *tok[20], *ystr, *name, *nick;
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
   memmove(mnem[i], s, 15);
   for (int j=0; j<30; j++)
     club[i][j] = 32;
   memmove(club[i], s+15, 30);
//   printf("%3d.%-30s [%-15s] extracted...\n", i, club[i], mnem[i]);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 500, f);
    if (!s) continue;
    if (strlen(s)<3) continue;
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
//      printf("Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
      k++;
    }
    s[0] = 0;
  }
  fclose(f);
}

char *NameOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetName(y);   
  if (!s) return club[t];  
  return s;
}
    

int Find(char* s) {
  int i, j;
  
  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  for (int i=0; i<strlen(s)-1; i++)
    if ((s[i]==32 || s[i]=='.') && s[i+1]>96) s[i+1] -= 32;
  for (i=0; i<NC; i++) { if (strcmp(mnem[i], s)==0) return i; }
  for (i=0; i<NC; i++) { if (strstr(mnem[i], s)==mnem[i]) return i; }
  // try all capitalized
  j=0; while (j<strlen(s) && s[j]!=' ' && s[j]!='.') {
   if (s[j]>='a' && s[j]<='z') s[j]-=32;
   j++;
  }
  for (i=0; i<NC; i++) { if (strcmp(mnem[i], s)==0) return i; }
  for (i=0; i<NC; i++) { if (strstr(mnem[i], s)==mnem[i]) return i; }
  return -1;
}

int GetUnique(const char *prompt) {
  char name[32];
  int res;
  do {
   printf("%s", prompt);
   do { fgets(name, 32, stdin); } while (strlen(name)<2);
   name[strlen(name)-1] = 0;
   res = Find(name);
  } while (res < 0);
  return res;
}

int main(int argc, char* argv[]) {
  FILE *f;
  char *s = new char[40];
  int n, y, i, j;
  Load();
  do {printf("Year:"); scanf("%d", &y);} while (y<1850 && y>2100);
  printf("No.of teams:"); scanf("%d", &n);
  int *id = new int[n];
  for (i=0; i<n; i++) {
    printf("Team #%d ID:", i+1); 
    j = GetUnique(" ");
    printf("  %s\n", NameOf(L, j, y));
    id[i] = j;
  }
  printf("\n\n\n");
  printf("%4d %2d", y, n);
  for (i=0; i<n; i++) {
    printf(" %3d", id[i]);
  }
  printf("\n\n\n");
  delete[] id;
  return 0;
}
