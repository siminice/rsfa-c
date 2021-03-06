#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int NC, NG;
char **club;
char **mnem;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pen[64], pdt[64];
int pts[64], rank[64], rnd[64][64], res[64][64];
int rnd2[64][64], res2[64][64];
int  tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
char inputfile[64], *outputfile;
int n, ppv, tbr, pr1, pr2, rel1, rel2, option, _rnd, year, a, b;
int gla, glb, rka, rkb;
char desc[64][32];

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
  if (!f) return 0;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
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
  return 1;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  f = fopen(filename, "rt");
  if (f==NULL) return 0;
  // Loading file
  char s[200], *tok[20];
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
      fscanf(f, "%d %d", &r, &z);
      rnd[i][j] = r;
      res[i][j]   = z;
      rnd2[i][j] = -1;
      res2[i][j] = -1;
    }
    fscanf(f, "\n");
  }

  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        rnd2[i][j] = r;
        res2[i][j] = z;
      }
      fscanf(f, "\n");
    }
  }

  gla = glb = -1;
  fclose(f);
  return 1;
}

void Dump() {
  FILE *f;
  int i, j;
  outputfile = new char[strlen(inputfile)+5];
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

//  for (i=0; i<n; i++) { if (strcmp(mnem[id[i]], s)==0) return i; }
  for (i=0; i<n; i++) {
		if (strstr(mnem[id[i]], s)==mnem[id[i]]) {
			return i;
		}
	}

  // try all capitalized
  j=0; while (j<strlen(s) && s[j]!=' ' && s[j]!='.') {
   if (s[j]>='a' && s[j]<='z') s[j]-=32;
   j++;
  }
  for (i=0; i<n; i++) { if (strcmp(mnem[id[i]], s)==0) return i; }
  for (i=0; i<n; i++) { if (strstr(mnem[id[i]], s)==mnem[id[i]]) return i; }
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

int sup(int i, int j, int tbr=0) {
  int gm1 = win[i]+drw[i]+los[i];
  int gm2 = win[j]+drw[j]+los[j];
  if (gm1==0 && gm2>0) return 0;
  if (gm2==0 && gm1>0) return 1;
  if (gm1==0 && gm2==0) return 0;
  if (tbr%10==1) {
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    return sup(i, j, 0);
  }
  else {
    int p1 = pts[i] - pen[i];
    int p2 = pts[j] - pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (i > j) return 0;
    return 1;
  }
}

void Tiebreak(int h, int k) {
  for (int i=h; i<=k; i++) {
    int j = rank[i];
    tbwin[j] = tbdrw[j] = tblos[j] = tbgsc[j] = tbgre[j] = 0;
    tbrk[i] = rank[i];
  }
  int gm = 0;
  for (int i=h; i<=k; i++) {
    for (int j=h; j<=k; j++) {
      int t1 = rank[i];
      int t2 = rank[j];
      if (res[t1][t2] >= 0) {
        gm++;
        int x = res[t1][t2] / 100;
        int y = res[t1][t2] % 100;
        tbgsc[t1] += x; tbgre[t2] += x;
        tbgsc[t2] += y; tbgre[t1] += y;
        if (x>y) {tbwin[t1]++; tblos[t2]++;}
        else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
        else {tbwin[t2]++; tblos[t1]++;}
      }
    }
  }
  
  int sorted;
  do {
    sorted = 1;
    for (int i=h; i<k; i++)
      if (sup(tbrk[i+1], tbrk[i], (gm>=(k-h+1)*(k-h)) )) {
        sorted = 0;
        int aux = tbrk[i];
        tbrk[i] = tbrk[i+1];
        tbrk[i+1] = aux;
      }
  } while (!sorted);
  for (int i=h; i<=k; i++)
    rank[i] = tbrk[i];
}

void Ranking() {
  // BubbleSort
  int i, j;
  for (i=0; i<n; i++) rank[i] = i;
  int sorted;
  if (NG>0) {
    int nn = (n+NG-1)/NG;
    do {  
      sorted = 1;
      for (int g=0; g<NG; g++) {
        for (i=0; i<(g==NG-1?n-g*nn-1:nn-1); i++) {
          if (sup(rank[g*nn+i+1], rank[g*nn+i], (tbr%10!=2? 0 : tbr))) {
            sorted = 0;
            int aux = rank[g*nn+i];
            rank[g*nn+i] = rank[g*nn+i+1];
            rank[g*nn+i+1] = aux;
          }
        }
      }
    } while (!sorted);   
    return;
 }
  do {
    sorted = 1;
    for (i=0; i<n-1; i++)
      if (sup(rank[i+1], rank[i])) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
  } while (!sorted);
  if (tbr) { 
   i = 0;
    while (i<n-1) {
      j = i;
      while (j+1<n && (pts[rank[j+1]]-pen[rank[j+1]] == pts[rank[i]]-pen[rank[i]])) j++;
      if (j>i) Tiebreak(i,j);
      i = j+1;
    }
  }
}


void Listing() {
  printf("\nStandings:\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    int grnk = i+1;
    if (NG>1) grnk = i%((n+NG-1)/NG)+1;    
    if (x==gla || x==glb) printf("\033[1m");
    printf("%2d.%-30s%2d%3d%3d%3d%4d-%2d", i+1,
     NameOf(L,id[x],year), win[x]+drw[x]+los[x], 
     win[x], drw[x], los[x], gsc[x], gre[x]);
     if (pen[x]>0) printf("%3d (-%dp pen)" , pts[x]-pen[x], pen[x]);
     else if (pen[x]<0) printf("%3d (+%dp bonus)" , pts[x]-pen[x], -pen[x]);
     else printf("%3d", pts[x]);
     printf(" %s\033[0m\n", desc[x]);
//     if (x==gla || x==glb) printf(" <\n"); else printf("\n");

    if (NG==0) {
     if (i==pr1-1)
        printf("------------------------------------------------------\n");
     if (i==pr1+pr2-1 && pr2>0)
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-(rel1+rel2)-1 && rel2>0)
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-rel1-1 && rel1>0)
        printf("------------------------------------------------------\n");
    }

    else if (NG>1) {
     if (i%((n+NG-1)/NG)==pr1/NG-1)
        printf("------------------------------------------------------\n");
    }

    if (NG>1 && (i+1)%((n+NG-1)/NG)==0) {
      printf("\n");
    }

  }
  gla = glb = -1;
}


int RankOf(int t) {
  for (int i=0; i<n; i++)
    if (rank[i]==t) return i;
  return -1;
}



int Swap() {
  int more = 0;
  int x, y, z;
  char name[64];

//	if (a<0 || b<0) {
  	do {
  	  printf("\n\nSwap 1: ");
  	  do { fgets(name, 30, stdin); } while (strlen(name)<2);
  	  name[strlen(name)-1] = 0;
  	  a = Find(name);
			printf("%s\n", NameOf(L, id[a], year));
  	} while (a<0);
  	do {
  	  printf("\n\nSwap 2: ");
  	  do { fgets(name, 30, stdin); } while (strlen(name)<2);
  	  name[strlen(name)-1] = 0;
  	  b = Find(name);
			printf("%s\n", NameOf(L, id[b], year));
  	} while (b<0);
//	}

  char opt[3];
  printf("\n\nSwap %s and %s? [y/n] ", NameOf(L, id[a], year), NameOf(L, id[b], year));
  scanf("%s", opt);
  if (opt[0]!='y' && opt[0]!='Y') return 1;

	int aux = id[a]; id[a] = id[b];  id[b] = aux;
	aux = win[a]; win[a] = win[b]; win[b] = aux;
	aux = drw[a]; drw[a] = drw[b]; drw[b] = aux;
	aux = los[a]; los[a] = los[b]; los[b] = aux;
	aux = gsc[a]; gsc[a] = gsc[b]; gsc[b] = aux;
	aux = gre[a]; gre[a] = gre[b]; gre[b] = aux;
	aux = pts[a]; pts[a] = pts[b]; pts[b] = aux;
	aux = pen[a]; pen[a] = pen[b]; pen[b] = aux;
	aux = pdt[a]; pdt[a] = pdt[b]; pdt[b] = aux;
	char saux[64];
	strcpy(saux, desc[a]); strcpy(desc[a], desc[b]); strcpy(desc[b], saux);

	aux =  rnd[a][b];  rnd[a][b] =  rnd[b][a];  rnd[b][a] = aux;
	aux =  res[a][b];  res[a][b] =  res[b][a];  res[b][a] = aux;
	aux = rnd2[a][b]; rnd2[a][b] = rnd2[b][a]; rnd2[b][a] = aux;
	aux = res2[a][b]; res2[a][b] = res2[b][a]; res2[b][a] = aux;

	for (int i=0; i<n; i++) if (i!=a && i!=b) {
		aux =  rnd[i][a];  rnd[i][a] =  rnd[i][b];  rnd[i][b] = aux;
		aux =  res[i][a];  res[i][a] =  res[i][b];  res[i][b] = aux;
		aux = rnd2[i][a]; rnd2[i][a] = rnd2[i][b]; rnd2[i][b] = aux;
		aux = res2[i][a]; res2[i][a] = res2[i][b]; res2[i][b] = aux;

		aux =  rnd[a][i];  rnd[a][i] =  rnd[b][i];  rnd[b][i] = aux;
		aux =  res[a][i];  res[a][i] =  res[b][i];  res[b][i] = aux;
		aux = rnd2[a][i]; rnd2[a][i] = rnd2[b][i]; rnd2[b][i] = aux;
		aux = res2[a][i]; res2[a][i] = res2[b][i]; res2[b][i] = aux;
	}
  gla = a;
	glb = b;
  Ranking();
  Listing();

  return 2;
}


int main(int argc, char* argv[]) {
	int a, b;
  if (!Load()) return -1;
  if (argc < 2) { printf("No input file specified.\n"); return 0; }
  strcpy(inputfile, argv[1]);
  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);
  NG = 0;
  for (int j=2; j<argc; j++) {
    if (strcmp(argv[j], "-g")==0) {
          NG = atoi(argv[++j]);
    }
  }

  if (!LoadFile(inputfile)) {
    printf("File %s not found.\n", inputfile);
    return 0;
  }

  Ranking();
  Listing();

  Swap();

  Ranking();
  Listing();

  Dump();
  for (int i=0; i<NC; i++)
    if (L[i]) delete L[i];
  return 1;
}
