#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

#define HEAD_TO_HEAD 1

const char *shortmonth[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *month[] = {"", "January", "February", "March", "April", "May", "June",
                     "July", "August", "September", "October", "November", "December"};

const char *dow[] = {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char **club;
char **mnem;
int  NC, NG;
int  n, ppv, tbr, pr1, pr2, rel1, rel2, option, _rnd, year;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pen[64], pdt[64];
int  pts[64], rank[64], rnd[64][64], res[64][64];
int  rnd2[64][64], res2[64][64];
int  tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
int lastrng, lastrnd, lastr, lastrh[16], lastrg[16], lastrd[16], lastrs[16];
int updates, updatez, updater;
int plda[100], pldb[100];
char desc[64][32];
char compet[128];

int gla, glb, rka, rkb;


int Month(char *m) {
  for (int i=1; i<=12; i++)
   if (strstr(m, month[i])==m) return i;
  return 0;
}


int adjust(int z) {
  if (z%50) return z;
  if (z==100) return 31;
  if (z==150) return 128;
  if (z==200) return 181;
  if (z==250) return 230;
  if (z==300) return 281;
  if (z==350) return 330;
  if (z==400) return 381;
  if (z==450) return 431;
  if (z==500) return 480;
  if (z==550) return 531;
  if (z==600) return 580;
  return z;
}

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
int extras(const char *str)
{
    int len = 0;

    while (*str != '\0') {
        if ((*str & 0xc0) == 0x80) {
            len++;
        }
        str++;
    }
    return len;
}

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("riku.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;

  for (int i=0; i<NC; i++) {
    fgets(s, 200, f);
    s[strlen(s)-1] = 0;
    if (strchr(s, ',')==NULL) {
      mnem[i] = new char[16];
      club[i] = new char[32];
      memmove(mnem[i], s, 15); mnem[i][15] = 0;
      int k = 15; while (mnem[i][k-1]==' ') k--; mnem[i][k]=0;
      for (int j=0; j<30; j++) club[i][j] = 32;
      memmove(club[i], s+15, 30);
    }
    else {
      tok[0] = strtok(s, ",");
      tok[1] = strtok(NULL, "\n");
      mnem[i] = new char[strlen(tok[0])+1];
      club[i] = new char[strlen(tok[1])+1];
      strcpy(mnem[i],tok[0]);
      strcpy(club[i],tok[1]);
    }
  }
  fclose(f);


  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    fgets(s, 2000, f);
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

char *GetTag(char *s) {
  char* b1 = strchr(s, '>');
  char *b2 = strrchr(s, '<');
  if (b1==NULL || b2==NULL) return s;
  b2[0] = 0;
  return b1+1;
}

int sup(int i, int j, int tbr=0) {
  int gm1 = win[i]+drw[i]+los[i];
  int gm2 = win[j]+drw[j]+los[j];
  int p1 = pts[i] - pen[i];
  int p2 = pts[j] - pen[j];
  if (gm1==0 && p1==0 && gm2>0) return 0;
  if (gm2==0 && p2==0 && gm1>0) return 1;
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
  char *utfname = new char[64];
  printf("\nStandings:\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    int grnk = i+1;
    if (NG>1) grnk = i%((n+NG-1)/NG)+1;
    if (x==gla || x==glb) printf("\033[1m");
    printf("%2d.", i+1);
    sprintf(utfname, "%s", NameOf(L,id[x],year));
    printf("%-*s", 30+extras(utfname), utfname);
    printf("%2d%3d%3d%3d%4d-%2d",
     win[x]+drw[x]+los[x],
     win[x], drw[x], los[x], gsc[x], gre[x]);
     if (pen[x]>0) printf("%4d (-%dp pen)" , pts[x]-pen[x], pen[x]);
     else if (pen[x]<0) printf("%4d (+%dp bonus)" , pts[x]-pen[x], -pen[x]);
     else printf("%4d", pts[x]);
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
  delete[] utfname;
}


int RankOf(int t) {
  for (int i=0; i<n; i++)
    if (rank[i]==t) return i;
  return -1;
}

int Find(char* s) {
  if (s==NULL) return -1;
  int l = strlen(s);
  if (l<2) return -1;
  while (l>0 && (s[l-1]==' ' || s[l-1]=='\t')) l--;
  s[l] = 0;
  for (int i=0; i<n; i++)
    if (strcmp(mnem[id[i]], s)==0) return i;
  char *hellip = strstr(s, "&hellip;");
  if (hellip!=NULL) {
    hellip[0] = 0;
  }
  for (int i=0; i<n; i++)
    if (strstr(mnem[id[i]], s) == mnem[id[i]]) return i;
  return -1;
}

void Dump(char *infile) {
  FILE *f;
  int i, j;
  char outputfile[100];

  strcpy(outputfile, infile);
  strcat(outputfile, ".old");
  rename(infile, outputfile);
  f = fopen(infile, "wt");
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

int LoadFile(char *filename) {
  printf("\n\033[33;1mReading from file %s \033[0m...\n", filename);
  FILE *f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }

  // Loading file
  char fncopy[64]; 
  strcpy(fncopy, filename);
  char *ystr = strtok(fncopy, ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);
  int i, j, x, y, zi;
  char s[200], *tok[20];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
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
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &x, &y);
      rnd[i][j] = x;
      res[i][j]   = y;
      rnd2[i][j] = res2[i][j] = -1;
      if (rnd[i][j]>50) {
        int thisd = rnd[i][j]%1000;
        int storethis = 0;
        int dl = abs(today-lastrnd);
        int dc = abs(today-thisd);
        if (dc < dl) {
          lastrng = 0;
          storethis = 1;
          lastrnd = thisd;
          lastr = rnd[i][j]/1000;
        }
        else if (dc == dl) { storethis = 1; }
        if (storethis) {
          lastrh[lastrng] = i;
          lastrg[lastrng] = j;
          lastrd[lastrng] = thisd;
          lastrs[lastrng] = res[i][j];
          lastrng++;
          lastr = rnd[i][j]/1000;
        }
      }
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &x, &y);
      rnd2[i][j] = x;
      res2[i][j]   = y;
      if (rnd2[i][j]>50) {
        int thisd = rnd2[i][j]%1000;
        int storethis = 0;
        int dl = abs(today-lastrnd);
        int dc = abs(today-thisd);
        if (dc < dl) {
          lastrng = 0;
          storethis = 1;
          lastrnd = thisd;
          lastr = rnd2[i][j]/1000;
        }
        else if (dc == dl) storethis = 1;
        if (storethis) {
          lastrh[lastrng] = i;
          lastrg[lastrng] = j;
          lastrd[lastrng] = thisd;
          lastrs[lastrng] = res2[i][j];
          lastrng++;
          lastr = rnd2[i][j]/1000;
        }
      }
    }
    fscanf(f, "\n");
  }
  fclose(f);
  }
  gla = glb = -1;

  printf("\nLatest results:\n");
  char utfname[64];
  for (int i=0; i<lastrng; i++) {
    printf("R%d, %2d %3s: ", lastr, lastrd[i]%50, shortmonth[lastrd[i]/50]);
    sprintf(utfname, "%s", NickOf(L, id[lastrh[i]], year));
    printf("%-*s ", 16+extras(utfname), utfname);
    printf("%d-%d %s\n", lastrs[i]/100, lastrs[i]%100, NickOf(L, id[lastrg[i]], year));
  }

  Ranking();
  Listing();
  return 1;
}

void Missing(int a, int b) {
  for (int i=0; i<100; i++)
    plda[i] = pldb[i] = 0;
  int la = -1;
  int lb = -1;
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
  int nav = 0;
  for (int i=1; i<=minl; i++) {
    if (plda[i] == 0 && pldb[i] == 0) nav++;
  }

  if (nav==0) return;

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

  printf("\n\n  Available rounds:");
  for (int i=1; i<=minl; i++) {
    if (plda[i] == 0 && pldb[i] == 0) 
      printf(" %d", i);
  }
  printf("\n");

}

int AddResult(int a, int b, int x, int y, int z) {
  int more = 0;
  int i, oldr, ox, oy;
  char rep[5];

  gla = glb = -1;

  rep[0] = 'n';

  if (updates==0) {
    if (lastrng>0) {
      if (z>(lastrd[0]%1000)+2) _rnd = 1000*(lastr+1)+z;
      else _rnd = 1000*lastr+z;
    }
  }

  if (tbr<10) {
    if (res[a][b] >= 0)  {
      if (res[a][b] == 100*x+y && rnd[a][b]%1000 >= z-1 && rnd[a][b]%1000 <= z+1) return 1;
      int zi = rnd[a][b] % 1000;
      printf("\n%s\033[1m[R%-2d] %2d %s: %s [%d-%d] %s \033[0m\n", compet,
               _rnd/1000, z%50, shortmonth[z/50], 
               mnem[id[a]], x, y, mnem[id[b]]);
      printf("-----------------------------------------------------------\n");
      printf("[R%-2d] %2d %s: %s [%d-%d] %s   ... already exists\n",
               rnd[a][b]/1000, zi%50, shortmonth[zi/50],
               mnem[id[a]], res[a][b]/100, res[a][b]%100, mnem[id[b]]);
      rep[0] = 'n';
      printf("Replace ? (y/n/k[r]):"); scanf("%s", rep);
      if (rep[0] == 'y' || rep[0] == 'k') {
        oy = res[a][b] % 100;
        ox = (int) (res[a][b]/100);
        gsc[a] -= ox; gre[a] -= oy;
        gsc[b] -= oy; gre[b] -= ox;
        if (ox>oy) { win[a]--; los[b]--; pts[a] -= ppv; }
          else if (ox==oy) { drw[a]--; drw[b]--; pts[a]--; pts[b]--; }
           else { los[a]--; win[b]--; pts[b] -= ppv; }
        printf("%s [%d-%d] %s deleted.\n", NameOf(L,id[a],year), ox, oy, NameOf(L,id[b],year));
      }
      else return 1;
    }
  }
  else {
    oldr = 0;
    if (res[a][b] >= 0)  {
      if (res[a][b]  == 100*x+y && rnd[a][b]%1000  >= z-1 && rnd[a][b]%1000  <= z+1) return 1;
      if (res2[a][b] == 100*x+y && rnd2[a][b]%1000 >= z-1 && rnd2[a][b]%1000 <= z+1) return 1;
      printf("\n%s\033[1m[R%-2d] %2d %s: %s [%d-%d] %s \033[0m\n", compet,
               _rnd/1000, z%50, shortmonth[z/50], 
               mnem[id[a]], x, y, mnem[id[b]]);
      printf("-----------------------------------------------------------\n");
      int zi = rnd[a][b] % 1000;
      printf("[R%-2d] %2d %s: %s [%d-%d] %s   ... already exists\n", 
               rnd[a][b]/1000, zi%50, shortmonth[zi/50], 
               mnem[id[a]], res[a][b]/100, res[a][b]%100, mnem[id[b]]);
      oldr += 1;
    }
    if (res2[a][b] >= 0)  {
      if (res2[a][b] == 100*x+y && rnd2[a][b]%1000 >= z-1 && rnd2[a][b]%1000 <= z+1) return 1;
      int zi = rnd2[a][b] % 1000;
      printf("[R%-2d] %2d %s: %s [%d-%d] %s   ... already exists\n", 
               rnd2[a][b]/1000, zi%50, shortmonth[zi/50], 
               mnem[id[a]], res2[a][b]/100, res2[a][b]%100, mnem[id[b]]);
      oldr += 2;
    }
    rep[0] = 'n';
    if (oldr>0) {
      printf("New (y) -- Replace (1/2/n):");
      scanf("%s", rep);
    }
    else {
      printf("\n%s\033[1m[R%-2d] %2d %s: %s [%d-%d] %s \033[0m\n", compet,
               _rnd/1000, z%50, shortmonth[z/50], 
               mnem[id[a]], x, y, mnem[id[b]]);
      printf("-----------------------------------------------------------\n");
      printf("New (y) -- Replace (1/2/n):");
      scanf("%s", rep);
    }
    if (rep[0] == '1' && oldr%2==1) {
      oy = res[a][b] % 100;
      ox = (int) (res[a][b]/100);
      gsc[a] -= ox; gre[a] -= oy;
      gsc[b] -= oy; gre[b] -= ox;
      if (ox>oy) { win[a]--; los[b]--; pts[a] -= ppv; }
        else if (ox==oy) { drw[a]--; drw[b]--; pts[a]--; pts[b]--; }
         else { los[a]--; win[b]--; pts[b] -= ppv; }
      res[a][b] = -1;
      rnd[a][b] = -1;
      printf("%s [%d-%d] %s  deleted.\n", NameOf(L,id[a],year), ox, oy, NameOf(L,id[b],year));
    }
    if (rep[0] == '2' && oldr>=2) {
      oy = res2[a][b] % 100;
      ox = (int) (res2[a][b]/100);
      gsc[a] -= ox; gre[a] -= oy;
      gsc[b] -= oy; gre[b] -= ox;
      if (ox>oy) { win[a]--; los[b]--; pts[a] -= ppv; }
        else if (ox==oy) { drw[a]--; drw[b]--; pts[a]--; pts[b]--; }
         else { los[a]--; win[b]--; pts[b] -= ppv; }
      res2[a][b] = -1;
      rnd2[a][b] = -1;
      printf("%s [%d-%d] %s  deleted.\n", NameOf(L,id[a],year), ox, oy, NameOf(L,id[b],year));
    }
    else if (rep[0]!='y' && (rep[0] < '0' || rep[0] > '2')) return 1;
  }

//  printf("Score: "); scanf("%d", &z);

  Missing(a, b);

  printf("\n%s[R%-2d] %2d %s: %s [%d-%d] %s \n", compet, _rnd/1000, z%50, shortmonth[z/50], 
           mnem[id[a]], x, y, mnem[id[b]]);
  printf("-----------------------------------------------------------\n");
  if (rep[0]!='y' && rep[0]!='Y') {  
    printf("Add ? (y/n/k):"); scanf("%s", rep);
  }
  if (rep[0]!='y' && rep[y]!='Y' && rep[0]!='0') return 1;
  updatez = z;
  if (rep[1]=='+') {
    updates++;
    _rnd += 1000;
  }
  else if (rep[1]=='-') {
    updates++;
    _rnd -= 1000;
  }
  else if (rep[1]=='r' || rep[1]=='R') {
    printf("\nEnter new round:"); scanf("%d", &_rnd);
    updates++;
    updater = _rnd;
    updatez = _rnd%1000;
  }
  if (tbr<10 || res[a][b] < 0) {
    if (rep[0]!='k') rnd[a][b] = 1000*(_rnd/1000)+updatez;
    res[a][b] = 100*x+y;
  }
  else if (res2[a][b] < 0) {
    rnd2[a][b] = 1000*(_rnd/1000)+updatez;
    res2[a][b] = 100*x+y;
  }

  if (100*x+y >= 0) {
    gsc[a] += x; gre[a] += y;
    gsc[b] += y; gre[b] += x;
    if (x>y) { win[a]++; los[b]++; pts[a] += ppv; }
      else if (x==y) { drw[a]++; drw[b]++; pts[a]++; pts[b]++; }
       else { los[a]++; win[b]++; pts[b] += ppv; }
    printf("%s - %s  [%d-%d]  added  (round %2d, %s %2d)\n\n",
           NameOf(L,id[a],year), NameOf(L,id[b],year),
           x, y, _rnd/1000, shortmonth[z/50], z%50);
    updates++;
  }

  // previous results
  if (res[a][b]>=0)
    printf("%s - %s  [%d-%d]  (round %2d, %s %2d)\n",
      NameOf(L,id[a],year), NameOf(L,id[b],year),
      res[a][b]/100, res[a][b]%100, rnd[a][b]/1000, shortmonth[(rnd[a][b]%1000)/50], rnd[a][b]%50);
  if (res[b][a]>=0)
    printf("%s - %s  [%d-%d]  (round %2d, %s %2d)\n",
      NameOf(L,id[b],year), NameOf(L,id[a],year),
      res[b][a]/100, res[b][a]%100, rnd[b][a]/1000, shortmonth[(rnd[b][a]%1000)/50], rnd[b][a]%50);
  if (res2[a][b]>=0)
    printf("%s - %s  [%d-%d]  (round %2d, %s %2d)\n",
      NameOf(L,id[a],year), NameOf(L,id[b],year),
      res2[a][b]/100, res2[a][b]%100, rnd2[a][b]/1000, shortmonth[(rnd2[a][b]%1000)/50], rnd2[a][b]%50);
  if (res2[b][a]>=0)
    printf("%s - %s  [%d-%d]  (round %2d, %s %2d)\n",
      NameOf(L,id[b],year), NameOf(L,id[a],year),
      res2[b][a]/100, res2[b][a]%100, rnd2[b][a]/1000, shortmonth[(rnd2[b][a]%1000)/50], rnd2[b][a]%50);
  gla = a; glb = b;
  Ranking();
  Listing();
  return 1;
}





int Extract(char *filename) {
  FILE *f = fopen(filename, "rt");
  if (f==NULL) return 2;
  char s[1024];
  int a, b, x, y, m, m1, z, z1, r, yr;
  char *tk, *tkw, *tkm, *tkd, *tkz, *tkf, *tkj, *tkyr, *tks, *t1, *t2, *tkx, *tky;
  char home[64], guest[64], score[64], scr[64], dow[64], format[12];

  updates = 0;
  r = 0;
  while (!feof(f)) {
    fgets(s, 1024, f);
    if (strstr(s, "  match ")!=NULL) {
      do {
        fgets(s, 1024, f);
        tk = strstr(s, "date ");
      }
      while (!feof(f) && tk==NULL);

      tkf = strstr(tk, "format");
      if (tkf!=NULL) tk = tkf;
      tkj = strstr(tk, ">");
      if (tkj==NULL) continue;
      tkj  = tkj+1;
      tkd  = strstr(tkj, "<");
      tkz  = strtok(tkj, "/");
      tkm  = strtok(NULL, ",/<\n");
      if (tkz && strlen(tkz)>0) z1  = atoi(tkz); else z1 = 0;
      if (z1>0) z = z1;
      if (tkm && strlen(tkm)>0)  m1 = atoi(tkm); else m1 = 0;
      if (m1>0) m = m1;

      do { 
        fgets(s, 1024, f); 
        tk = strstr(s, "team-a");
      } while (tk==NULL && !feof(f));
      if (tk!=NULL) {
        for (int ii=0; ii<2; ii++) fgets(s, 1024, f);
        s[strlen(s)-1] = 0;
        t2 = s; while (t2[0]==' ') t2++;
        strcpy(home, t2);
      }
      a = Find(home);
      if (a<0) printf("\033[31;1mCannot find team %s\033[0m...\n", home);
/*
      for (int ii=0; ii<11; ii++)
        fgets(s, 1024, f);
*/
      do {
        fgets(s, 1024, f);
        tk = strstr(s, "score-time");
      } while (tk==NULL && !feof(f));

      for (int ii=0; ii<3; ii++)
        fgets(s, 1024, f);

      if (strstr(s, "PSTP")!=NULL) continue;
      if (strstr(s, "CANC")!=NULL) continue;
      if (strstr(s, "SUSP")!=NULL) continue;
      if (strchr(s, ':')!=NULL) continue;
      char *sk = GetTag(s);
      tkx = strtok(sk, " -");
      tky = strtok(NULL, " -\n");
      if (tkx==NULL || tky==NULL) continue;
      x = atoi(tkx);
      y = atoi(tky);

      do { 
        fgets(s, 1024, f); 
        tk = strstr(s, "team-b");
      } while (tk==NULL && !feof(f));
      if (tk!=NULL) {
        for (int ii=0; ii<2; ii++) fgets(s, 1024, f);
        s[strlen(s)-1] = 0;
        t2 = s; while (t2[0]==' ') t2++;
        strcpy(guest, t2);
      }
      b = Find(guest);
      if (b<0) printf("\033[31;1mCannot find team %s\033[0m...\n", guest);

//      printf("\n[R%-2d] %2d %s, %s [%s] %s\n", _rnd/1000, z, shortmonth[m], home, score, guest);
//      printf("-----------------------------------------------------------\n");
      if (a>=0 && b>=0) AddResult(a, b, x, y, 50*m+z);
    }
  }
  return 1;
}

int main(int argc, char **argv) {
  char inputfile[64], outputfile[64], updatesfile[64];

  setlocale(LC_ALL, "");

  Load();
  if (argc < 3) { 
    printf("Not all input files specified.\n"); 
    printf("Usage: autoupdate a.2007 first.html\n");
    return 0; 
  }

  NG = 0;
  for (int j=3; j<argc; j++) {
    if (strcmp(argv[j], "-g")==0) {
          NG = atoi(argv[++j]);
    }
  }

  strcpy(compet, "");
  FILE *f;
  strcpy(inputfile, argv[1]);
  strcpy(updatesfile, argv[2]);
  LoadFile(inputfile);
  Extract(updatesfile);
  if (updates>0) {
    Dump(inputfile);
    int dumby;
    scanf("%d", &dumby);
  }
  for (int i=0; i<NC; i++)
    if (L[i]) delete L[i];
  return 1;
  
}
