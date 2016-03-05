#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define HEAD_TO_HEAD 	 1
#define MAX_N 		64
#define MAX_RR 	 	 4
#define NUM_ORD 	10

#define SPECIAL         50
#define LOSS_BOTH_0     50
#define LOSS_BOTH_9     59

int NC, NG;
char **club;
char **mnem;
int  id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pen[MAX_N], pdt[MAX_N];
int pts[MAX_N], rank[MAX_N];
int rnd[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N];
int  tbwin[MAX_N], tbdrw[MAX_N], tblos[MAX_N], tbgsc[MAX_N], tbgre[MAX_N], tbrk[MAX_N];
char inputfile[64], *outputfile;
int n, ppv, tbr, numr, pr1, pr2, rel1, rel2, option, _rnd, year;
int lastrng, lastrnd, lastr, lastrh[16], lastrg[16], lastrd[16], lastrs[16];
int gla, glb, rka, rkb;
char desc[MAX_N][32];
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

void Dump() {
  FILE *f;
  int k, i, j;
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

  for (k=0; k<numr; k++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fprintf(f, "%6d%5d", rnd[k][i][j], res[k][i][j]);
    }
    fprintf(f, "\n");
  }
  }
  fclose(f);
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
  int r = 0;
  for (int i=h; i<=k; i++) {
    for (int j=h; j<=k; j++) {
      int t1 = rank[i];
      int t2 = rank[j];
      if (res[r][t1][t2] >= 0) {
        gm++;
        int x = res[r][t1][t2] / 100;
        int y = res[r][t1][t2] % 100;
        if (y >= SPECIAL) {
          
        }
        else {
          tbgsc[t1] += x; tbgre[t2] += x;
          tbgsc[t2] += y; tbgre[t1] += y;
          if (x>y) {tbwin[t1]++; tblos[t2]++;}
          else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
          else {tbwin[t2]++; tblos[t1]++;}
        }
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

int DeleteResult(int k, int a, int b) {
  int sc = res[k][a][b];
  if (sc>=0) {
    int x = sc/100;
    int y = sc%100;
    if (y>=SPECIAL) {
      if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
        int z = y%10;
        los[a]--; los[b]--;
        gre[a] -= z; gre[b] -= z;
        printf("%s - %s loss to both 0-%d  deleted.\n", NameOf(L,id[a],year), 
                NameOf(L,id[b],year), z);
      }
    }
    else {
      gsc[a] -= x; gre[a] -= y;
      gsc[b] -= y; gre[b] -= x;
      if (x>y) { win[a]--; los[b]--; pts[a] -= ppv; }
      else if (x==y) { drw[a]--; drw[b]--; pts[a]--; pts[b]--; }
      else { win[b]--; los[a]--; pts[b] -= ppv; }
    }
        printf("%s - %s  %d-%d  deleted.\n", NameOf(L,id[a],year), 
                NameOf(L,id[b],year), x, y);
    rnd[k][a][b] = -1;
    res[k][a][b] = -1;
  }
}

int AddResult() {
  int more = 0;
  int rd, a, b, x, y, z, i, k, kk, oldr;
  int pld[MAX_RR];
  char rep[5];

  gla = glb = -1;
  for (int k=0; k<MAX_RR; k++) pld[k] = 0;

  a = GetUnique("Home  :");
  printf("  %s\n", NameOf(L,id[a],year));
  b = GetUnique("Guest :");
  printf("  %s \n", NameOf(L,id[b],year));

  if (numr<2) {
    if (res[0][a][b] >= 0)  {
      int zi = rnd[0][a][b] % 1000;
      printf("%s [%d-%d] %s, round %d, %s %d.\n", NameOf(L,id[a],year), 
              res[0][a][b] / 100, res[0][a][b] % 100, NameOf(L,id[b],year),
              rnd[0][a][b]/1000, month[zi/50-1], zi % 50);
      rep[0] = 'n';
      printf("Replace ? (y/n/k):"); scanf("%s", rep);
      if (rep[0] == 'y' || rep[0] == 'k') {
        DeleteResult(0, a, b);
      }
      else return 1;
    }
  }
  else {
    oldr = 0;
    for (int k=0; k<numr; k++) {
    if (res[k][a][b] >= 0)  {
      int zi = rnd[k][a][b] % 1000;
      printf("%s [%d-%d] %s, round %d, %s %d.\n", NameOf(L,id[a],year), 
              res[k][a][b] / 100, res[k][a][b] % 100, NameOf(L,id[b],year),
              rnd[k][a][b]/1000, month[zi/50-1], zi % 50);
      oldr++;
      pld[k] = 1;
    }
    }

    rep[0] = 'n';
    if (oldr>0) {
      printf("New (0) -- Replace (");
      for (int k=1; k<=numr; k++) printf("%c/", '0'+k);
      printf("n):"); 
      scanf("%s", rep);
    }
    else {
      rep[0] = '0';
      kk = 0;
    }
     
    if (rep[0] == '0') { 
      if (oldr>0) {
        kk=0; while (kk<numr && res[kk][a][b]>=0) kk++;
        if (kk>=numr) return 2;
      }
    }
    else if (rep[0] >= '1' && rep[0] <= ('0'+numr)) {
      kk = rep[0] - '1';
      DeleteResult(kk, a, b);
    }
    else return 1;
  }

  printf("Score: "); scanf("%d", &z);
  if (numr<2) {
    if (rep[0]!='k') rnd[0][a][b] = _rnd;
    res[0][a][b] = z;
  }
  else if (res[kk][a][b] < 0) {
    if (rep[1]!='k') rnd[kk][a][b] = _rnd;
    res[kk][a][b] = z;
  }

  if (z >= 0) {
    y = z % 100;
    x = (int) (z/100);
    int zi = _rnd % 1000;
    if (y>=SPECIAL) {
      if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
        int yy = y%10;
        los[a]++; los[b]++;
        gre[a] += yy;
        gre[b] += yy;
      }
    }
    else {
      gsc[a] += x; gre[a] += y;
      gsc[b] += y; gre[b] += x;
      if (x>y) { win[a]++; los[b]++; pts[a] += ppv; }
        else if (x==y) { drw[a]++; drw[b]++; pts[a]++; pts[b]++; }
         else { los[a]++; win[b]++; pts[b] += ppv; }
      printf("%s - %s  [%d-%d]  added  (round %2d, %s %2d)\n\n", 
             NameOf(L,id[a],year), NameOf(L,id[b],year),
             x, y, _rnd/1000, month[zi/50-1], zi%50);
    }
  }

  // previous results
  for (int k=0; k<numr; k++) {
    if (res[k][a][b]>=0)
      printf("%s - %s  [%d-%d]  (round %2d, %s %2d)\n", 
        NameOf(L,id[a],year), NameOf(L,id[b],year),
        res[k][a][b]/100, res[k][a][b]%100, rnd[k][a][b]/1000, month[(rnd[k][a][b]%1000)/50-1], rnd[k][a][b]%50);
    if (res[k][b][a]>=0)
      printf("%s - %s  [%d-%d]  (round %2d, %s %2d)\n", 
        NameOf(L,id[b],year), NameOf(L,id[a],year),
        res[k][b][a]/100, res[k][b][a]%100, rnd[k][b][a]/1000, month[(rnd[k][b][a]%1000)/50-1], rnd[k][b][a]%50);
  }
  gla = a; glb = b;
  return 1;
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  if (!Load()) return -1;
  if (argc < 2) { printf("No input file specified.\n"); return 0; }
  FILE *f;
  strcpy(inputfile, argv[1]);
  printf("Reading from file %s...\n", inputfile);
  f = fopen(inputfile, "rt");
  if (NULL == f) { printf("File %s not found.\n", inputfile); return 0; }

  NG = 0;
  for (int j=2; j<argc; j++) {
    if (strcmp(argv[j], "-g")==0) {
          NG = atoi(argv[++j]);
    }
  }

  // Loading file
  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);
  int k, i, j, x, y, zi;
  char s[200], *tok[12];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  numr = tbr/NUM_ORD + 1;
  lastrng = 0;
  time_t  tt;
  struct  tm* t = new tm;
  time(&tt);
  t = localtime(&tt);
  int today = (t->tm_mon +1)*50 + t->tm_mday;
  lastrnd = -1;
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
//    printf("Loaded %-30s : %2d %2d %2d %2d %2d-%2d", NameOf(L,id[i],year),
//            win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
//    if (pen[i]!=0) printf(" %2dp. (%dp penalty)\n", pts[i]-pen[i], pen[i]);
//    else printf(" %2dp.\n", pts[i]);
  }

  for (k=0; k<MAX_RR; k++) 
    for (i=0; i<n; i++)
      for (j=0; j<n; j++) { rnd[k][i][j] = res[k][i][j] = -1; }

  for (k=0; k<numr; k++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &x, &y);
      rnd[k][i][j] = x;
      res[k][i][j] = y;
      if (rnd[k][i][j]>50) {
        int thisd = rnd[k][i][j]%1000;
        int storethis = 0;
        int dl = abs(today-lastrnd);
        int dc = abs(today-thisd);
        if (dc < dl) { 
          lastrng = 0; 
          storethis = 1; 
          lastrnd = thisd;
          lastr = rnd[k][i][j]/1000;
        }
        else if (dc == dl) { storethis = 1; }
        if (storethis) {
          lastrh[lastrng] = i;
          lastrg[lastrng] = j;
          lastrd[lastrng] = thisd;
          lastrs[lastrng] = res[k][i][j];
          lastrng++;
          lastr = rnd[k][i][j]/1000;
        }
      }
    }
    fscanf(f, "\n");
  }
  }
   fclose(f);
  gla = glb = -1;

  printf("\nLatest results:\n");
  for (int i=0; i<lastrng; i++) {
    printf("R%d, %2d %3s: %-16s %d-%d %s\n", lastr,
            lastrd[i]%50, month[lastrd[i]/50-1], mnem[id[lastrh[i]]], lastrs[i]/100, 
            lastrs[i]%100, mnem[id[lastrg[i]]] );
  }

  Ranking();
  Listing();

  option = 2;
  _rnd   = -1;
  char sinp[100];
  while (option) {
    switch(option) {
      case 1 : 
        AddResult(); 
        Ranking();
        Listing();
        break;
      case 2 :
        do {
          _rnd = -1;
          printf("\nNew round: ");
          scanf("%s", sinp);
          _rnd = atoi(sinp);
          if ((_rnd%1000)/50 < 1 || (_rnd%1000)/50 > 12) _rnd = -1;
        } while (_rnd!=-1 && ((_rnd%1000)/50 < 1 || (_rnd%1000)/50 > 12));
        break;
      case 3 :
        _rnd++;
        if (_rnd%50>31) _rnd += 19;
        break;
      case 4 :
        _rnd--;
        if (_rnd%50 == 0) _rnd -= 19;
        break;
      default :
        option = 1;
    }
    zi = _rnd % 1000;
    printf("\n[1] Add result [2] Change round (%d, %s %d) [3] Date++ [4] Date-- [0] Exit\n", 
            _rnd/1000, (_rnd==-1?"unknown":month[zi/50-1]), zi%50);
    scanf("%d", &option);
  }
  Dump();
  for (int i=0; i<NC; i++)
    if (L[i]) delete L[i];
  return 1;
}
