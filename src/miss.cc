#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define HEAD_TO_HEAD 1
#define SIZE 64

int NC;
char **club;
char **mnem;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pen[64], pdt[64];
int pts[64], rank[64], rnd[64][64], res[64][64];
int rnd2[64][64], res2[64][64];
int  tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
char inputfile[64], *outputfile;
int n, ppv, tbr, pr1, pr2, rel1, rel2, option, _rnd, year;
int lastrng, lastrnd, lastr, lastrh[16], lastrg[16], lastrd[16], lastrs[16];
int gla, glb, rka, rkb;
char desc[64][32];
const char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

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
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
//   printf("%3d.%-30s [%-15s] extracted...\n", i, club[i], mnem[i]);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return;
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

int Find(char* s) {
  int found = 0;
  int l;

  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  l = strlen(s);
  for (int j=0; j<l-1; j++)
    if ((s[j]==32 || s[j]=='.') && s[j+1]>96) s[j+1] -= 32;
  int i = 0;
  while (i < n && !found) {
    if (NULL != strstr(mnem[id[i]], s)) found = 1;
    else i++;
  }
  if (found) return i;
 
  // try uppercase
  for (int j=0; j<l; j++)
    if (s[j]>='a' && s[j]<='z') s[j] -= 32;
  i = 0;
  while (i < n && !found) {
    if (NULL != strstr(mnem[id[i]], s)) found = 1;
    else i++;
  }
  if (found) return i;
  return -1;
}

int GetUnique(const char *prompt) {
  char name[32];
  int res;
  do {
   printf("%s", prompt); 
   do { fgets(name, 30, stdin); } while (strlen(name)<2);
   name[strlen(name)-1] = 0;
   res = Find(name);
  } while (res < 0);
  return res;
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();
  if (argc < 2) { printf("No input file specified.\n"); return 0; }
  FILE *f;
  strcpy(inputfile, argv[1]);
  printf("Reading from file %s...\n", inputfile);
  f = fopen(inputfile, "rt");
  if (NULL == f) { printf("File %s not found.\n", inputfile); return 0; }

  // Loading file
  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);
  int i, j, x, y, zi;
  char s[200], *tok[12];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &x, &y);
      rnd[i][j] = x;
      res[i][j]   = y;
      rnd2[i][j] = res2[i][j] = -1;
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &x, &y);
      rnd2[i][j] = x;
      res2[i][j]   = y;
    }
    fscanf(f, "\n");
   }
   fclose(f);
  }

  int a = GetUnique("Home  :");
  printf("  %s\n", NameOf(L,id[a],year));
  int b = GetUnique("Guest :");
  printf("  %s \n", NameOf(L,id[b],year));

  int la = -1;
  int plda[100];
  for (int i=0; i<100; i++) plda[i] = 0;
  for (int i=0; i<n; i++) {
    if (rnd[a][i]>0) {
       int ra = rnd[a][i]/1000;
       if (ra > la) la = ra;
       plda[ra] ++;
    }
    if (rnd2[a][i]>0) {
       int ra = rnd2[a][i]/1000;
       if (ra > la) la = ra;
       plda[ra] ++;
    }
    if (rnd[i][a]>0) {
       int ra = rnd[i][a]/1000;
       if (ra > la) la = ra;
       plda[ra] ++;
    }
    if (rnd2[i][a]>0) {
       int ra = rnd2[i][a]/1000;
       if (ra > la) la = ra;
       plda[ra] ++;
    }
  }

  int lb = -1;
  int pldb[100];
  for (int i=0; i<100; i++) pldb[i] = 0;
  for (int i=0; i<n; i++) {
    if (rnd[b][i]>0) {
       int rb = rnd[b][i]/1000;
       if (rb > lb) lb = rb;
       pldb[rb] ++;
    }
    if (rnd2[b][i]>0) {
       int rb = rnd2[b][i]/1000;
       if (rb > lb) lb = rb;
       pldb[rb] ++;
    }
    if (rnd[i][b]>0) {
       int rb = rnd[i][b]/1000;
       if (rb > lb) lb = rb;
       pldb[rb] ++;
    }
    if (rnd2[i][b]>0) {
       int rb = rnd2[i][b]/1000;
       if (rb > lb) lb = rb;
       pldb[rb] ++;
    }
  }
  
  int minl = (la > lb ? lb : la);

  for (int i=0; i<=la; i++)  {
    printf("%d", plda[i]); 
    if (i%10==0) printf(" | ");
    else if (i%10==5) printf(" ");
  }
  printf("\n");
  for (int i=0; i<=lb; i++) {
    printf("%d", pldb[i]); 
    if (i%10==0) printf(" | ");
    else if (i%10==5) printf(" ");
  }
  printf("\n");

  for (int i=1; i<=minl; i++) {
    if (plda[i] == 0 && pldb[i] == 0) {
      printf("Round %d available.\n", i);
    }
  }

}
