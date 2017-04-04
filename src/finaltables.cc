#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int NC;

char **club;
char **mnem;
int  id[64], gms[64], win[64], drw[64], los[64], gsc[64], gre[64], pts[64], pen[64], pdt[64];
int  rank[64], rnd[64][64], rnd2[64][64], res[64][64], res2[64][64];
int  n, tbr, pr1, pr2, rel1, rel2, ppv, year;
char desc[64][32];
char inputfile[64], outputfile[64], syntax[64];

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
  
char *NameOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetName(y);   
  if (!s) return club[t];  
  return s;
}
    
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


int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[12];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = drw[i] = los[i] = 0;
    gsc[i] = gre[i] = pts[i] = 0;
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
    for (int j=0; j<n; j++) 
     rnd[i][j] = rnd2[i][j] = res[i][j] = res2[i][j] = -1;
  }
  fclose(f);
  return 1;
}

void Dump() {
  FILE *f;
  int i, j;
  strcpy(outputfile, inputfile);
  strcat(outputfile, ".old");
  rename(inputfile, outputfile);
  f = fopen(inputfile, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<n; i++) {
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
    if (pen[i]!=0) fprintf(f, "%4d%4d", pen[i], pdt[i]);
    if (strlen(desc[i])>0) {
      if (pen[i]==0)  fprintf(f, "%4d%4d", pen[i], pdt[i]);
      fprintf(f, " %s", desc[i]);
    }
    fprintf(f, "\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", rnd[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++)
        fprintf(f, "%6d%5d", rnd2[i][j], res2[i][j]);
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

void FinalTables() {
  int i = 0;
  char s[256], *tk[20];
  do {
    fgets(s, 200, stdin);
    tk[0] = strtok(s, " -()\t\n");
    for (int l=1; l<20; l++) tk[l] = strtok(NULL, " -()\t\n");
    int k = 19;
    while (k>=0 && tk[k]==NULL) k--;
    int h = 1;
    while (h<k && atoi(tk[h])==0) h++;
    if (k-h < 5) continue;
    
    for (int j=h; j<=k; j++) {
      switch (syntax[j-h]) {
        case 'g' : gms[i] = atoi(tk[j]); break;
        case 'w' : win[i] = atoi(tk[j]); break;
        case 'd' : drw[i] = atoi(tk[j]); break;
        case 'l' : los[i] = atoi(tk[j]); break;
        case 's' : gsc[i] = atoi(tk[j]); break;
        case 'r' : gre[i] = atoi(tk[j]); break;
        case 'p' : pts[i] = atoi(tk[j]); break;
      }
    }   
    if (gms[i]>0 && pts[i]==0) pts[i] = win[i]*ppv + drw[i];
    i++;
  } while (i<n);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("ERROR: no input file specified.\n");
    printf(" Usage: finaltables a.2000 gwdlsrp\n");
    return 1;
  }

  FILE *f;
  int n, ppv, tbr, pr1, pr2, rel1, rel2, i, j;
  Load();

  strcpy(inputfile, argv[1]);
  strcpy(syntax, "gwdlsrp");
  
  if (!LoadFile(inputfile)) return 2;

  int k=0; while (k<strlen(inputfile) && inputfile[k]!='.') k++;
  int y = atoi(inputfile+k+1);

  if (argc > 2) {
    strcpy(syntax, argv[2]);
  }

  FinalTables();
  Dump();

  return 0;
}
