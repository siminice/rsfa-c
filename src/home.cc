#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT      0
#define HEAD_TO_HEAD 1
#define RATIO        2
#define NUMWINS      4
#define DRAW8        5
#define DRAW10       6
#define GDIFF2       7
#define GDIFF3       8
#define AWAY3        9
#define NUMORD      10

const char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char sep1[100], sep2[100];
char **club;
char **mnem;
int  NC;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pts[64], pen[64], pdt[64];
int  lid[64], lwin[64], ldrw[64], llos[64], lgsc[64], lgre[64], lpts[64], lpen[64], lpdt[64];
int  rank[64], round[64][64], round2[64][64], res[64][64], res2[64][64];
int  allres[64][64], allres2[64][64];
int  tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
int  day[12], home[12], guest[12];
int  n, r, lastr, lastrnd, tbr, pr1, pr2, rel1, rel2, ppv, year;
int  NG;
char desc[64][32];
bool winter;
int  fr, frd, lr, lrd, decorate;

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

int idof(int x) {
  int j=0;
  while (j<n && lid[j]!=x) j++;
  if (j>=n) {
    printf("Error: Could not match %s in preloaded file.\n", mnem[x]);
    return -1;
  }
  return j;
}

//--------------------------------------

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  if (f==NULL) return 0;
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
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return 0;
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

int after(int dt, int r) {
  if (winter) return (dt<r);
  if (dt==0) return 1;
  if (dt>400) {
    if (r<400) return 1;
    else return (dt<=r);
  }
  if (r>400) return 0;
  else return (dt<=r);
}

int sup(int i, int j, int tbr=0) {
  int gm1 = win[i]+drw[i]+los[i];
  int gm2 = win[j]+drw[j]+los[j];
  int p1 = pts[i] - pen[i];
  int p2 = pts[j] - pen[j];
  if (gm1==0 && p1==0 && gm2>0) return 0;
  if (gm2==0 && p2==0 && gm1>0) return 1;
  if (tbr%NUMORD==HEAD_TO_HEAD) {  
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    if (tbwin[i] > tbwin[j]) return 1;
    if (tbwin[i] < tbwin[j]) return 0;
    if (res[i][j]/100 + res[j][i]%100 > res[j][i]/100 + res[i][j]%100) return 1;
    if (res[i][j]/100 + res[j][i]%100 < res[j][i]/100 + res[i][j]%100) return 0;
    if (res[i][j]%100 < res[j][i]%100) return 1;
    if (res[i][j]%100 > res[j][i]%100) return 0;
    return sup(i, j, 0);
  }
  else if (tbr%NUMORD==RATIO) {  
    int p1 = ppv*win[i]+drw[i];
    int p2 = ppv*win[j]+drw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    double gc1 = 0.0;
    if (gre[i]>0) gc1 = (double) gsc[i]/ (double) gre[i];
    double gc2 = 0.0;
    if (gre[j]>0) gc2 = (double) gsc[j]/ (double) gre[j];
    if (gc1>gc2) return 1;
    if (gc1<gc2) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (i > j) return 0;
  }
  else {
    int p1 = pts[i];
    int p2 = pts[j];
    if (pen[i]!=0 && after(pdt[i],lastr)) p1 -= pen[i];
    if (pen[j]!=0 && after(pdt[j],lastr)) p2 -= pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;   
    if (gsc[i] < gsc[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
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
      if (res2[t1][t2] >= 0) {
        gm++;
        int x = res2[t1][t2] / 100;
        int y = res2[t1][t2] % 100;
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
      if (sup(tbrk[i+1], tbrk[i], (tbr%NUMORD!=RATIO ? gm>=(k-h+1)*(k-h) : RATIO))) {  
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
          if (sup(rank[g*nn+i+1], rank[g*nn+i], (tbr%NUMORD!=RATIO? 0 : tbr))) {
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
    for (i=0; i<n-1; i++) {
      if (sup(rank[i+1], rank[i], (tbr%NUMORD!=RATIO? 0 : tbr))) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
    }
  } while (!sorted);
  if (tbr%NUMORD==1) {
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
//  printf("\nStandings:\n");
  printf("\n");
  char sdsc[100];
  for (int i=0; i<n; i++) {
    int x = rank[i];
    sdsc[0] = 0;
    if (desc[x]) strcpy(sdsc, desc[x]);
    char *spar = strchr(sdsc, '(');
    if (!decorate && spar) spar[0] = 0;
    int grnk = i+1;
    if (NG>1) grnk = i%((n+NG-1)/NG)+1;
    if (win[x]+drw[x]+los[x]==0 && win[rank[0]]+drw[rank[0]]+los[rank[1]]>0) {
      printf(" -.%-52s", NameOf(L,id[x],year));
      printf(" %s\n", sdsc);
    }
    else {
     printf("%2d.%-30s%2d%3d%3d%3d%4d%c%-3d", grnk,
     NameOf(L,id[x],year), win[x]+drw[x]+los[x], 
     win[x], drw[x], los[x], gsc[x], (tbr%10!=RATIO?'-':':'), gre[x]);
     if (tbr%10==RATIO) printf(" %4.3f ", (gre[x]>0?((double)gsc[x])/((double)gre[x]):gsc[x]));
     if (pen[x]>0 && after(pdt[x],lastr)) printf("%3d (-%dp pen)" , pts[x]-pen[x], pen[x]);
     else if (pen[x]<0 && after(pdt[x],lastr)) printf("%3d (+%dp bonus)" , pts[x]-pen[x], -pen[x]);
     else printf("%3d", pts[x]);
     printf(" %s\n", sdsc);
    }
    if (NG==0) {
     if (i==pr1-1) 
        printf("%s\n", sep1);
     if (i==pr1+pr2-1 && pr2>0) 
        printf("%s\n", sep2);
     if (i==n-(rel1+rel2)-1 && rel2>0) 
        printf("%s\n", sep2);
     if (i==n-rel1-1 && rel1>0)
        printf("%s\n", sep1);
    }
    else if (NG>1) {
     if (i%((n+NG-1)/NG)==pr1/NG-1) 
        printf("%s\n", sep1);
    }

    if (NG>1 && (i+1)%((n+NG-1)/NG)==0) {
      printf("\n");
    }
  }
  printf("\n");
}

int AddResult(int a, int b, int x, int y) {
  if (res[a][b]<0 && allres[a][b]>=0)
    res[a][b] = allres[a][b];
  else res2[a][b] = allres2[a][b];
  gsc[a] += x; gre[a] += y;
  if (tbr%NUMORD==GDIFF2) {
    if (x-y>=2) pts[a]++;
  }
  if (x>y) { 
    win[a]++; pts[a] += ppv; 
  }
  else if (x==y) { 
    drw[a]++; pts[a]++; 
    if (tbr==DRAW10 && drw[a]>10) pts[a]--;
    if (tbr==DRAW8 && drw[a]>8) pts[a]--;
  }
  else { 
    los[a]++;
  }
//  printf("%-15s%d-%d %s\n", mnem[id[a]], x, y, mnem[id[b]]);
  return 1;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) return 0;
  // Loading file
  char s[500], *tok[12];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    lwin[i] = atoi(tok[2]);
    ldrw[i] = atoi(tok[3]);
    llos[i] = atoi(tok[4]);
    lgsc[i] = atoi(tok[5]);
    lgre[i] = atoi(tok[6]);
    lpts[i] = atoi(tok[7]);
    win[i] = drw[i] = los[i] = 0;
    gsc[i] = gre[i] = pts[i] = 0;
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  lrd = lr = -1;
  fr = frd = 1000;
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      allres[i][j] = z;
      res[i][j] = -1;
      res2[i][j] = -1;
      int td = round[i][j]%1000;
      int tr = round[i][j]/1000;
      if (round[i][j]>0 && tr == 0 && fr!=1 && td < frd) frd = td;
      if (tr == 1 && td < frd) {frd = td; fr = 1;}
      if (round[i][j]>0 && tr == 0 && td > lrd) lrd = td;
      if (tr > lr) lr = tr;
      if (td > lrd && frd < 1000 && lrd >0) {
        if (frd>lrd && td<frd) lrd = td;
        if (frd<lrd) lrd = td;
      }
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        round2[i][j] = r;
        allres2[i][j] = z;
        res2[i][j] = -1;
        if (round2[i][j] > 0 && round2[i][j]/1000 == 0 && round2[i][j]%1000 > lrd) lrd = round2[i][j]%1000;
        if (round[i][j]/1000 >lr || (round[i][j]/1000==lr && round[i][j]%1000 > lrd)) {
          lrd = round2[i][j]%1000;
          lr = round2[i][j]/1000;
        }
      }
      fscanf(f, "\n");
    }
  }
  fclose(f);
  strcpy(sep1,"-------------------------------------------------------");
  strcpy(sep2,"- - - - - - - - - - - - - - - - - - - - - - - - - - - -");
  if (tbr%NUMORD==RATIO) strcat(sep1,"--------");
  if (tbr%NUMORD==RATIO) strcat(sep2,"- - - - ");
  return 1;
}

void Synoptical() {
  char *symbol = new char[4];
  symbol[3] = 0;
  printf("\n                ");
  for (int i=0; i<n; i++) {
    memmove(symbol, NickOf(L, id[rank[i]], year), 3);
    printf("%3s ", symbol);
  }
  printf("\n");
  for (int i=0; i<n; i++) {
    printf("%-15s", NickOf(L, id[rank[i]], year));
    for (int j=0; j<n; j++)
      if (i==j) printf(" xxx");
       else if (res[rank[i]][rank[j]] >=0)
         printf("%2d-%d", res[rank[i]][rank[j]]/100, res[rank[i]][rank[j]]%100);
        else printf("  - ");
    printf("\n");
  }
  if (tbr<10) return;
  printf("\n");
  for (int i=0; i<n; i++) {
    printf("%-15s", NickOf(L, id[rank[i]], year));
    for (int j=0; j<n; j++)
      if (i==j) printf(" xxx");
       else if (res2[rank[i]][rank[j]] >=0)
         printf("%2d-%d", res2[rank[i]][rank[j]]/100, res2[rank[i]][rank[j]]%100);
        else printf("  - ");
    printf("\n");
  }
}

int consecutive(int r, int last) {
  if (last%50>=30 && r%50==1) return 0;
  if (last>0 && (r-last>1 || r<last)) return 1;
  return 0;
}

void DisplayData(int full, bool winter, int fd, int ld) {
  int ng;
  lastr = -1;
  int fmo = 7; int lmo = 20;
  if (frd < lrd) winter = 1;

  if (winter) { fmo=1; lmo=13; }
/*
  if (ld<0)  ld = fd+649;
  if (ld<fd) ld = ld+650;
*/
  if (ld<0) ld = fd + 649;
  for (int d=fd; d<=ld; d++) {
    r = d%650;
    ng = 0;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (round[i][j]%1000==r && allres[i][j]>=0) {
          if (ng==0) {
            if (consecutive(r,lastr)) {
              if (full) { Ranking(); Listing(); }
              else printf("\n");
            }
            if (round[i][j]/1000>0) printf("Round %d ", round[i][j]/1000);
            printf("[%s %d]\n", month[r/50], d%50);
          }
          AddResult(i, j, allres[i][j]/100, allres[i][j]%100);
          printf("%-15s%d-%d %s\n", NickOf(L, id[i], year), allres[i][j]/100, 
                  allres[i][j]%100, NickOf(L, id[j], year));
          ng++;
        }
        if (tbr>=10 && round2[i][j]%1000==r && allres2[i][j]>=0) {
          if (ng==0) {
            if (consecutive(r,lastr)) {
              if (full) { Ranking(); Listing(); }
              else printf("\n");
            }
            if (round2[i][j]/1000>0) printf("Round %d ", round2[i][j]/1000);
            printf("[%s %d]\n", month[r/50], d%50);
          }
          AddResult(i, j, allres2[i][j]/100, allres2[i][j]%100);
          printf("%-15s%d-%d %s\n", NickOf(L, id[i], year), allres2[i][j]/100, 
                  allres2[i][j]%100, NickOf(L, id[j], year));
          ng++;
        }
      }
    }
    if (ng>0) lastr = r;
  }

  if (win[0]+drw[0]+los[0]==0) {
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (allres[i][j]>=0 && round[i][j]<0)
          AddResult(i,j, allres[i][j]/100, allres[i][j]%100);
        if (allres2[i][j]>=0 && round2[i][j]<0)
          AddResult(i,j, allres2[i][j]/100, allres2[i][j]%100);
      }
    }
  }

  if (win[0]+drw[0]+los[0]==0) {
    for (int i=0; i<n; i++) {
      win[i] = lwin[i];
      drw[i] = ldrw[i];
      los[i] = llos[i];
      gsc[i] = lgsc[i];
      gre[i] = lgre[i];
      pts[i] = lpts[i];
    }
  }

  Ranking();
  Listing();
  if (full) Synoptical();
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  if (!Load()) {
    printf("ERROR: called from invalid drectory.\n"); 
    return -1;     
  }
  if (argc < 2) { 
    printf("ERROR: No input file specified.\n"); 
    return -1; 
  }
  if (!LoadFile(argv[1])) {
    printf("ERROR: file %s not found.\n", argv[1]);
    return -2;
  }
  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  if (ystr) year = atoi(ystr); else year = 0;
  bool full = true;
  winter = false;
  int fd = -1;
  int ld = -1;
  NG = 0;
  decorate = 1;
  if (argc > 2) {
    int i=2; 
    do {
      if (strcmp(argv[i],"-")==0) full = false;
      else if (argv[i][0] == '-') {
        if (argv[i][1] == 'w') {
          winter = true;
          fd = 1;
          ld = 650;
        }
        else if (strcmp(argv[i],"-fd")==0) {
          fd = atoi(argv[++i]);
//          ld = fd + 649;
        }
        else if (strcmp(argv[i],"-ld")==0) {
          ld = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-g")==0) {
          NG = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-d")==0) {
          decorate = 0;
        }
        else
          printf("Unknown option: %s\n", argv[i]);
      }
      else printf("Illegal command line argument: %s\n", argv[i]);
      i++;
    } while (i<argc);
  }
  if (fd < 0) fd = frd;
//  if (ld < 0) ld = lrd;
  DisplayData(full, winter, fd, ld);
  for (int i=0; i<NC; i++) {
    if (L[i]) delete L[i];
    delete[] club[i];
    delete[] mnem[i];
  }
  delete[] club;
  delete[] mnem;
  delete[] L;
  return 0;
}
