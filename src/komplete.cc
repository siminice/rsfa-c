#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N           64
#define MAX_RR           4
#define NUM_ORD		10

#define SPECIAL         50
#define LOSS_BOTH_0     50
#define LOSS_BOTH_9     59

char **club;
char **mnem;
int  NC;

int  id[MAX_N], ng[MAX_N];
int rnd[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N];
int n, numr, year;
char inputfile[128];
int freq[100];

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

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  if (f==NULL) {
    printf("ERROR: file teams.dat not found.\n");
    return 0;
  }
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
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
//   printf("%3d.%-30s [%-15s] extracted...\n", i, club[i], mnem[i]);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (f==NULL) {
    printf("ERROR: file teams.dat not found.\n");
    return 0;
  }
  for (int i=0; i<NC; i++) {
    fgets(s, 2000, f);
    if (!s) continue;
    if (s[0]==0) continue;
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
  return 1;
}


char *NameOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetName(y);
  if (!s) return club[t];
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetNick(y);
  if (!s) return mnem[t];
  return s;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, k, x, y, r, z;
  f = fopen(filename, "rt");
  if (f==NULL) return 0;
  // Loading file
  char s[200], *tok[20];
  fgets(s, 200, f);
  char *tkn = strtok(s, " \t");
  char *tkppv = strtok(NULL, " \t");
  char *tktbr = strtok(NULL, " \t");
  n = atoi(tkn);
  int tbr = atoi(tktbr);
  numr = tbr/NUM_ORD + 1;
  for (i=0; i<n; i++) {
    fgets(s, 200, f);   
    tok[0] = strtok(s, " ");
    id[i] = atoi(tok[0]);
  }

  for (k=0; k<MAX_RR; k++)
    for (i=0; i<n; i++)
      for (j=0; j<n; j++) { rnd[k][i][j] = res[k][i][j] = -1; }

  for (k=0; k<numr; k++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      rnd[k][i][j] = r;
      res[k][i][j] = z;
    }
    fscanf(f, "\n");
  }
  }

  fclose(f);
  return 1;
}

int Check() {
  int i, j, k;
  for (i=0; i<n; i++) ng[i] = 0;
  for (i=0; i<100; i++) freq[i] = 0;
  for (i=0; i<n; i++) {
    for (k=0; k<numr; k++) {
      for (j=0; j<n; j++) {
        if (res[k][i][j] >= 0) {
          ng[i]++;
          ng[j]++;
        }
      }
    }
  }
  for (i=0; i<n; i++) freq[ng[i]]++;
  int mf = ng[0];
  for (i=1; i<n; i++) {
    if (freq[ng[i]] > freq[mf]) mf = ng[i];
  }

  int diff = 0;
  for (i=0; i<n; i++) {
    if (ng[i]>0 && ng[i] != mf) {
      fprintf(stdout, "%-16s: %d/%d\n", NickOf(L, id[i], year), ng[i], mf);
      diff++;
    }
  }
  return diff;
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  if (!Load()) return -1;
  if (argc < 2) { printf("No input file specified.\n"); return 0; }
  strcpy(inputfile, argv[1]);

  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);

  if (!LoadFile(inputfile)) { 
    printf("File %s not found.\n", inputfile); 
    return 0; 
  }
 
  int diff = Check();

  return 1;
}
