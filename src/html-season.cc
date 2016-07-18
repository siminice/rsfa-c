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

#define MAX_LEVELS 12
#define MAX_RR	    4
#define MAX_N      64

#define SPECIAL		50
#define LOSS_BOTH_0	50
#define LOSS_BOTH_9	59

#define MAX_NAMES       20000
#define DB_ROWS         1024
#define DB_COLS         60
#define DB_CELL         20
#define EV_COLS         30

#define EV_GOAL          0
#define EV_OWNGOAL       1
#define EV_PKGOAL        2
#define EV_PKMISS        3
#define EV_YELLOW        4
#define EV_RED           5
#define EV_YELLOWRED     6

#define MAX_GOALS	100
#define MAX_EXEQUO	100

char edb[DB_ROWS][EV_COLS][DB_CELL];
char **pmnem, **pname, **ppren, **pdob, **pcty, **ppob, **pjud;
int  pgol[MAX_NAMES], ppen[MAX_NAMES], prnk[MAX_GOALS][MAX_EXEQUO], nrnk[MAX_GOALS];

const char *month[] = {"", "Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
                     "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"};
const char *romonth[] = {"", "ianuarie", "februarie", "martie", "aprilie", "mai", "iunie",
                     "iulie", "august", "septembrie", "octombrie", "noiembrie", "decembrie"};
int borna[256];
char **club;
char **mnem;
int  NC, NP;
int  id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pts[MAX_N], pen[MAX_N], pdt[MAX_N];
int  round[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N], allres[MAX_RR][MAX_N][MAX_N];
int  rank[MAX_N];
int  pos[MAX_N][365];
char desc[MAX_N][32];

int  lid[MAX_N], lwin[MAX_N], ldrw[MAX_N], llos[MAX_N], lgsc[MAX_N], lgre[MAX_N], lpts[MAX_N], lpen[MAX_N], lpdt[MAX_N];
int  hwin[MAX_N], hdrw[MAX_N], hlos[MAX_N], hgsc[MAX_N], hgre[MAX_N], hpts[MAX_N];
int  gwin[MAX_N], gdrw[MAX_N], glos[MAX_N], ggsc[MAX_N], ggre[MAX_N], gpts[MAX_N];
int  prank[MAX_N], hrank[MAX_N], grank[MAX_N];
int  tbwin[MAX_N], tbdrw[MAX_N], tblos[MAX_N], tbgsc[MAX_N], tbgre[MAX_N], tbrk[MAX_N];
int  n, rr, r, lastr, lastrnd, tbr, pr1, pr2, rel1, rel2, ppv, year, numr;
int  split_after, split_top, split_points, cancel_gdiff;
int  NG;
bool winter;
int num_winter;
int *start_winter, *end_winter;

int  fd, ld, fr, frd, lr, lrd, decorate, calendar, dates_unknown;
FILE *of;
char dvz;
int  seria;
char sf[128];
int  played[650];
int  sched[650][24];
int  num_sched, first_d, first_r, last_d, last_r;

//---------------------------
char *hexlink = new char[32];
void makeHexlink(int i) {
  sprintf(hexlink, "%x%x%x%x%x%x",
    ((unsigned char)pmnem[i][0]),
    ((unsigned char)pmnem[i][1]),
    ((unsigned char)pmnem[i][2]),
    ((unsigned char)pmnem[i][3]),
    ((unsigned char)pmnem[i][4]),
    ((unsigned char)pmnem[i][5]));
  hexlink[12]=0;
}
//---------------------------

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

//------------------------------------

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

  num_winter = 0;
  f = fopen("winter.dat", "rt");
  if (f!=NULL) {
    fscanf(f, "%d\n", &num_winter);
    start_winter = new int[num_winter];
    end_winter = new int[num_winter];
    for (int i=0; i<num_winter; i++) {
      fscanf(f, "%d %d\n", start_winter+i, end_winter+i);
    }
    fclose(f);
  }

  f = fopen("webteams.dat", "rt");
  if (f==NULL) return 0;
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[48];
    club[i] = new char[48];
    strncpy(mnem[i], strtok(s, ","), 32);
    strncpy(club[i], strtok(NULL, ",\n"), 32);
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
  if (dt>=fd) {
    if (r<fd) return 1;
    else return (dt<r);
  }
  if (r>=fd) return 0;
  else return (dt<r);
}

int sup(int r, int i, int j, int tbr=0) {
  int gm1 = win[i]+drw[i]+los[i];
  int gm2 = win[j]+drw[j]+los[j];
  int p1 = pts[i];
  int p2 = pts[j];
  if (r <= split_after) {
    p1 -= pen[i];
    p2 -= pen[j];
  }
  if (gm1==0 && p1==0 && gm2>0) return 0;
  if (gm2==0 && p2==0 && gm1>0) return 1;
  if (tbr%NUMORD==HEAD_TO_HEAD) {
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbwin[i] > tbwin[j]) return 1;
    if (tbwin[i] < tbwin[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    return sup(r, i, j, 0);
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
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (i > j) return 0;
  }
  else {
    int p1 = pts[i];
    int p2 = pts[j];
    if (r<=split_after && pen[i]!=0 && after(pdt[i],lastr)) p1 -= pen[i];
    if (r<=split_after && pen[j]!=0 && after(pdt[j],lastr)) p2 -= pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (i > j) return 0;
    return 1;
  }
}

int hsup(int i, int j) {
  int p1 = hpts[i];
  int p2 = hpts[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (hgsc[i] - hgre[i] > hgsc[j] - hgre[j]) return 1;
    if (hgsc[i] - hgre[i] < hgsc[j] - hgre[j]) return 0;
    if (hwin[i] > hwin[j]) return 1;
    if (hwin[i] < hwin[j]) return 0;
    if (hgsc[i] > hgsc[j]) return 1;
    if (hgsc[i] < hgsc[j]) return 0;
    if (i > j) return 0;
    return 1;
}

int gsup(int i, int j) {
  int p1 = gpts[i];
  int p2 = gpts[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (ggsc[i] - ggre[i] > ggsc[j] - ggre[j]) return 1;
    if (ggsc[i] - ggre[i] < ggsc[j] - ggre[j]) return 0;
    if (gwin[i] > gwin[j]) return 1;
    if (gwin[i] < gwin[j]) return 0;
    if (ggsc[i] > ggsc[j]) return 1;
    if (ggsc[i] < ggsc[j]) return 0;
    if (i > j) return 0;
    return 1;
}

void Tiebreak(int r, int k, int l) {
  for (int i=k; i<=l; i++) {
    int j = rank[i];
    tbwin[j] = tbdrw[j] = tblos[j] = tbgsc[j] = tbgre[j] = 0;
    tbrk[i] = rank[i];
  }
  int gm = 0;
  for (int i=k; i<=l; i++) {
    for (int j=k; j<=l; j++) {
      int t1 = rank[i];
      int t2 = rank[j];
      for (int h=0; h<rr; h++) {
      if (res[h][t1][t2] >= 0) {
        gm++;
        int x = res[h][t1][t2] / 100;
        int y = res[h][t1][t2] % 100;
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
    for (int i=k; i<l; i++)
      if (sup(r, tbrk[i+1], tbrk[i], (tbr%NUMORD!=RATIO ? gm>=(l-k+1)*(l-k) : RATIO))) {
        if (!(r > split_after && i+1==split_top)) {
          sorted = 0;
          int aux = tbrk[i];
          tbrk[i] = tbrk[i+1];
          tbrk[i+1] = aux;
        }
      }
  } while (!sorted);
  for (int i=k; i<=l; i++)
    rank[i] = tbrk[i];
}


void Ranking(int r) {
  // BubbleSort
  int i, j;
  for (i=0; i<n; i++) prank[rank[i]] = i;
//  for (i=0; i<n; i++) rank[i] = i;
  int sorted;
  if (NG>0) {
    int nn = (n+NG-1)/NG;
    do {
      sorted = 1;
      for (int g=0; g<NG; g++) {
        for (i=0; i<(g==NG-1?n-g*nn-1:nn-1); i++) {
          if (sup(r, rank[g*nn+i+1], rank[g*nn+i], (tbr%NUMORD!=RATIO? 0 : tbr))) {
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
      if (sup(r, rank[i+1], rank[i], (tbr%NUMORD!=RATIO? 0 : tbr))) {
        if (!(r>split_after && i+1==split_top)) {
          sorted = 0;
          int aux = rank[i];
          rank[i] = rank[i+1];
          rank[i+1] = aux;
        }
      }
    }
  } while (!sorted);
  if (tbr%NUMORD==1) {
   i = 0;
    while (i<n-1) {
      j = i;
      while (j+1<n && (pts[rank[j+1]]-pen[rank[j+1]] == pts[rank[i]]-pen[rank[i]])) j++;
      if (j>i) Tiebreak(r, i,j);
      i = j+1;
    }
  }
}

void HRanking() {
  int i, j;
  for (i=0; i<n; i++) hrank[i] = i;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<n-1; i++) {
      if (hsup(hrank[i+1], hrank[i])) {
        sorted = 0;
        int aux = hrank[i];
        hrank[i] = hrank[i+1];
        hrank[i+1] = aux;
      }
    }
  } while (!sorted);
}

void GRanking() {
  int i, j;
  for (i=0; i<n; i++) grank[i] = i;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<n-1; i++) {
      if (gsup(grank[i+1], grank[i])) {
        sorted = 0;
        int aux = grank[i];
        grank[i] = grank[i+1];
        grank[i+1] = aux;
      }
    }
  } while (!sorted);
}

void FindFirstRoundDate() {
  first_r = 2000;
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (allres[0][i][j]>=0 && round[0][i][j]>0) {
        int r = round[0][i][j]/1000;
        int z = round[0][i][j]%1000;
        if (r==1 && z<first_r) first_r = z;
      }
    }
  }
  fprintf(stderr, "First round date: %d\n", first_r);
}

int LastRoundNumber() {
  int lr = 0;
  for (int k=0; k<rr; k++) {
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (allres[k][i][j]>=0 && round[k][i][j]>0) {
        int r = round[k][i][j]/1000;
        if (r>lr) lr = r;
      }
    }
  }
  }
  return lr;
}

void CollectSched(int calendar) {
  int k;
  num_sched = 0;
  last_r = 0;
  for (int h=0; h<rr; h++) {
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (i!=j && allres[h][i][j]>=0) {
        int r = round[h][i][j]/1000;
        int z = round[h][i][j]%1000;
        if (h>0 && r<numr) r += h*numr;
        played[z] = 1;
        if (r>last_r) last_r = r;
        if (round[h][i][j] > 0) {
        if (calendar) {
          if (sched[z][0]==0) num_sched++;
          if (sched[z][0]>=19) {
             fprintf(stderr, "ERROR: too many matches on %d.\n", z);
             return;
          }
          k = ++sched[z][0];
          sched[z][k] = 100*i+j;
        }
        else {
          if (sched[r][0]==0) num_sched++;
          if (sched[r][0]>=19) {
             fprintf(stderr, "ERROR: too many matches in round %d.\n", r);
             return;
          }
          k = ++sched[r][0];
          sched[r][k] = 100*i+j;
        }
        }
      }
    }
  }
  }

    last_d = ld%650;
    while (last_d > 0 && played[last_d]==0) last_d--;
    if (last_d==0) {
      last_d = 631;
      while (last_d > 0 && played[last_d]==0) last_d--;
    }
    first_d = fd;
    while (first_d < 631 && played[first_d]==0) first_d++;
}

int AddResult(int h, int a, int b) {
  res[h][a][b] = allres[h][a][b];
  if (res[h][a][b] < 0) return 0;
  int x = allres[h][a][b]/100;
  int y = allres[h][a][b]%100;
  if (y>=SPECIAL) {
    if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
      int z = y%10;
      gre[a] += z;
      gre[b] += z;
      los[a]++; los[b]++;
      hlos[a]++; glos[b]++;
    }
    return 2;
  }
  gsc[a]  += x; gre[a]  += y;
  hgsc[a] += x; hgre[a] += y;
  gsc[b]  += y; gre[b]  += x;
  ggsc[b] += y; ggre[b] += x;
  if (x>y) {
    win[a]++; los[b]++; pts[a] += ppv;
    hwin[a]++; glos[b]++; hpts[a] += ppv;
  }
  else if (x==y) {
    drw[a]++; drw[b]++; pts[a]++; pts[b]++;
    hdrw[a]++; gdrw[b]++; hpts[a]++; gpts[b]++;
  }
  else {
    los[a]++; win[b]++; pts[b] += ppv;
    hlos[a]++; gwin[b]++; gpts[b] += ppv;
  }
  return 1;
}

void AdjustStats() {
  for (int i=0; i<n; i++) {
    if (split_points==1) {
      pts[i] = (pts[i]-pen[i]+1)/2;
    } else if (split_points==2) {
      pts[i] = 0;
    }
    if (cancel_gdiff>0) {
      gsc[i] = gre[i] = 0;
      win[i] = drw[i] = los[i] = 0;
    }
  }
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  char s[512], *tok[20];

//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) return 0;
  // Loading file

  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  rr = tbr/NUMORD + 1;
  if (n%2==0) numr = rr*2*(n-1);
  else numr = rr*2*n;

  dates_unknown = 1;
  /* clear all  data */
  for (int h=0; h<rr; h++) {
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        round[h][i][j] = -1;
        allres[h][i][j] = -1;
        res[h][i][j] = -1;
      }
    }
  }

  for (i=0; i<n; i++) {
    rank[i] = hrank[i] = grank[i] = prank[i] = i;
    fgets(s, 512, f);
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
    if (tok[10]) {
       if (strchr(tok[10], '1')) strcpy(desc[i], "(1)");
       else if (strchr(tok[10], '-')) strcpy(desc[i], "(-)");
       else if (strchr(tok[10], '+')) strcpy(desc[i], "(+)");
       else strcpy(desc[i],"");
    }
    else strcpy(desc[i],"");

  }
  lrd = lr = -1;
  fr = frd = 1000;
  for (int h=0; h<rr; h++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[h][i][j] = r;
      if (r > 0) dates_unknown = 0;
      allres[h][i][j] = z;
      res[h][i][j] = -1;
      int tr = round[h][i][j]/1000;
      int td = round[h][i][j]%1000;
      // autocorrect multiple round robin ?
/*
      if (h>0 && tr <= numr/rr) {
        round[h][i][j] += 1000*h*(numr/rr);
        tr += 1000*h*(numr/rr);
      }
*/
      if (round[h][i][j]>0 && tr == 0 && fr!=1 && td < frd) frd = td;
      if (tr == 1 && td < frd) {frd = td; fr = 1;}
      if (round[h][i][j]>0 && tr == 0 && td > lrd) lrd = td;
      if (tr > lr) lr = tr;
      if (td > lrd && frd < 1000 && lrd >0) {
        if (frd>lrd && td<frd) lrd = td;
        if (frd<lrd) lrd = td;
      }
    }
    fscanf(f, "\n");
  }
  }
  fclose(f);
  return 1;
}

void LoadPlayers() {
  NP = 0;
  int c;
  for (c=0; c<256; c++) borna[c] = -1;
  FILE *f = fopen("players.dat", "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: player database not found!\n");
    exit(0);
  }
  char s[100], *t[10];
  fgets(s, 100, f);
  sscanf(s, "%d", &NP);
  pname = new char*[MAX_NAMES];
  pmnem = new char*[MAX_NAMES];
  ppren = new char*[MAX_NAMES];
  pdob  = new char*[MAX_NAMES];
  pcty  = new char*[MAX_NAMES];
  ppob  = new char*[MAX_NAMES];
  pjud  = new char*[MAX_NAMES];

  for (int i=0; i<NP; i++) {
    fgets(s, 100, f);
//    fprintf(stderr, "... parsing %s", s);
    t[0] = strtok(s, ",\t\n");
     pmnem[i] = new char[7];
     strcpy(pmnem[i], t[0]);
    t[1] = strtok(NULL, ",\t\n");
     pname[i] = new char[strlen(t[1])+1];
     strcpy(pname[i], t[1]);
    t[2] = strtok(NULL, ",\t\n");
     ppren[i] = new char[strlen(t[2])+1];
     strcpy(ppren[i], t[2]);
    t[3] = strtok(NULL, ",\t\n");
     pdob[i] = new char[12];
     strcpy(pdob[i], t[3]);
    t[4] = strtok(NULL, ",\t\n");
     pcty[i] = new char[4];
     strcpy(pcty[i], t[4]);
    t[5] = strtok(NULL, ",\t\n");
     if (t[5]!=NULL) {
       ppob[i] = new char[strlen(t[5])+1];
       strcpy(ppob[i], t[5]);
     }
     else {
       ppob[i] = new char[2];
       strcpy(ppob[i], "");
     }
    t[6] = strtok(NULL, ",\t\n");
     if (t[6]!=NULL) {
       pjud[i] = new char[strlen(t[6])+1];
       strcpy(pjud[i], t[6]);
     }
     else {
       pjud[i] = new char[2];
       strcpy(pjud[i], "");
     }
     pgol[i] = 0;
     ppen[i] = 0;
     if (i==0 || (i>0 && pmnem[i-1][0]!=pmnem[i][0])) {
       c = ((unsigned char) pmnem[i][0]);
       borna[c] = i;
     }
  }
  fclose(f);

  c = ((unsigned char)pmnem[NP-1][0]);
  borna[c+1] = NP;
  for (c=255; c>0; c--)
    if (borna[c]>=0 && borna[c-1]<0) borna[c-1] = borna[c];
}


void LoadEvents() {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "events-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<rr*n*(n-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<EV_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<EV_COLS; j++) {
      if (tk[j]!=NULL) strcpy(edb[i][j], tk[j]);
      else strcpy(edb[i][j], " ");
    }
  }
  fclose(f);
}

int FindMnem(char *mnem) {
  int c = ((unsigned char)mnem[0]);
  for (int i=borna[c]; i<borna[c+1]; i++)
    if (strcmp(pmnem[i], mnem)==0) return i;
  return -1;
}

int MatchID(int r, int a, int b) {
  int mid = r*n*(n-1) + a*(n-1)+b-(b>a?1:0);
  return mid;
}

void GetEvents(int r, int a, int b) {
  int mid = MatchID(r, a, b);
  int ep, et;
  char s[DB_CELL], *sp, *sm;
  for (int i=0; i<EV_COLS; i++) {
    strcpy(s, edb[mid][i]);
    sp = strtok(s, "'`\"/,\n");
    sm = strtok(NULL, "'`\"/,\n");
    if (sp!=NULL) ep = FindMnem(sp); else ep=-1;
    if (ep>=0 && ep<MAX_NAMES) {
      if (edb[mid][i][6]==39) pgol[ep]++;
      if (edb[mid][i][6]=='"') { pgol[ep]++; ppen[ep]++; }
    }
  }
}

void TopScorers() {
  for (int g=0; g<MAX_GOALS; g++) nrnk[g] = 0;
  for (int p=0; p<NP; p++) {
    int ng = pgol[p];
    if (nrnk[ng]<MAX_EXEQUO-1) {
      prnk[ng][nrnk[ng]] = p;
      nrnk[ng]++;
    }
  }
}

void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);
  else sprintf(ss, "%d/%02d", y-1, y%100);
}

void HTMLHeader(int i) {
  char ss[32];
  SeasonName(year, ss);
  fprintf(of, "<HTML>\n<TITLE>Divizia %c", dvz);
  if (seria>0) fprintf(of, "%d", seria);
  fprintf(of, " %s", ss);
  if (calendar) {
    fprintf(of, " - %d %s", i%50, romonth[i/50]);
  }
  else {
    fprintf(of, "- etapa %d", i);
  }
  fprintf(of, "</TITLE>\n");
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

}

void HTMLFooter() {
	if (dvz=='A') {
	  fprintf(of, "<UL><LI><A HREF=\"catalog-%d.html\">Loturi echipe</A></LI></UL>\n", year);
	}
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

void NavTable(int r) {
  char ss[32];
  SeasonName(year, ss);

//  fprintf(of, "<SECTION>\n");
//  fprintf(of, "<DIV class=\"navcol\">\n");

  fprintf(of, "<A HREF=\"index.html\"><IMG HEIGHT=\"32\" SRC=\"home.gif\"></IMG></A>\n");
  fprintf(of, "<B><FONT SIZE=+2>Divizia %c", dvz);
  if (seria>0) fprintf(of, "%d", seria);
  fprintf(of, " %s", ss);
  if (calendar) {
    fprintf(of, "<BR>  %d %s", r%50, romonth[r/50]);
  }
  else {
    fprintf(of, "<BR>  Etapa %d", r);
  }
  fprintf(of, "</B></FONT>\n");
  fprintf(of, "<BR>\n<TABLE BORDER=\"0\">\n");
  fprintf(of, "<TR>");
  if (calendar) {
    fprintf(of, "<TD><A HREF=\"%s-r%d.html\"><IMG HEIGHT=\"32\" SRC=\"structure.gif\"></IMG></A></TD>", sf, last_r);
  }
  else {
    fprintf(of, "<TD><A HREF=\"%s-d%d.html\"><IMG HEIGHT=\"32\" SRC=\"calendar.gif\"></IMG></A></TD>", sf, last_d);
  }

      int pr = r-1;
      while (pr>0 && played[pr]==0) pr--;
      if (pr==0) {
        pr = 649;
        while (pr>0 && played[pr]==0) pr--;
      }
      int nr = r+1;
      while (nr<649 && played[nr]==0) nr++;
      if (nr==649) {
        nr = 50;
        while (nr<649 && played[nr]==0) nr++;
      }

  fprintf(of, "<TD>");
  if (calendar) {
    if (r!=first_d) {
      fprintf(of, "<A HREF=\"%s-d%d.html\">", sf, pr);
    }
    fprintf(of, "<IMG HEIGHT=\"32\" SRC=\"prev.gif\"></IMG>");
    if (r!=first_d) {
      fprintf(of, "</A>");
    }
  }
  else {
    if (r>1) {
      fprintf(of, "<A HREF=\"%s-r%d.html\">", sf, r-1);
    }
    fprintf(of, "<IMG HEIGHT=\"32\" SRC=\"prev.gif\"></IMG>");
    if (r>1) {
      fprintf(of, "</A>");
    }
  }
  fprintf(of, "</TD>");

  fprintf(of, "<TD>");
  if (calendar) {
    if (r!=last_d) {
      fprintf(of, "<A HREF=\"%s-d%d.html\">", sf, nr);
    }
    fprintf(of, "<IMG HEIGHT=\"32\" SRC=\"next.gif\"></IMG>");
    if (r!=first_d) {
      fprintf(of, "</A>");
    }
  }
  else {
    if (r<numr) {
      fprintf(of, "<A HREF=\"%s-r%d.html\">", sf, r+1);
    }
    fprintf(of, "<IMG HEIGHT=\"32\" SRC=\"next.gif\"></IMG>");
    if (r<numr) {
      fprintf(of, "</A>");
    }
  }
  fprintf(of, "</TD>");

  fprintf(of, "</TR>\n");
  fprintf(of, "</TABLE>\n");
}

void RoundsTable(int r) {
  fprintf(of, "<BR>\n<TABLE WIDTH=\"50%%\" FRAME=\"box\">\n");
  int lrn = LastRoundNumber();
  if (lrn < 1) lrn = numr;
  int hr = (lrn+1)/2;
  for (int i=1; i<=lrn; i++) {
    fprintf(of, "<TD");
    if (i==r) fprintf(of, " BGCOLOR=\"DDDDDD\" ");
    fprintf(of, "><A HREF=\"%s-r%d.html\">R%d</A></TD>", sf, i, i);
    if (i%hr==0) {
      fprintf(of, "</TR>\n");
      if (i<lrn) fprintf(of, "<TR>");
    }
  }
  fprintf(of, "</TR>\n");
  fprintf(of, "</TABLE>\n");
//  fprintf(of, "</DIV>\n");
//  fprintf(of, "</SECTION>\n");
//  fprintf(of, "</SECTION>\n");
}

void CalendarTable(int r) {
  int md[13][32];
  for (int i=0; i<=12; i++) for (int j=0; j<=31; j++) md[i][j] = 0;
  for (int i=51; i<=631; i++) if (played[i]) {
    int m=i/50;
    int z=i%50;
    md[0][m]++;
    md[m][z]++;
  }
  int fm = first_d/50;
//  fprintf(of, "<DIV class=\"caltab\" align=\"center\">\n");
  fprintf(of, "<BR>\n<TABLE WIDTH=\"96%%\" BORDER=\"1\" RULES=\"columns\" FRAME=\"box\">\n");
  fprintf(of, "<THEAD BGCOLOR=\"DDDDDD\">\n<TR>\n");
  for (int m=fm-1; m<fm+11; m++) {
    int i = (m%12)+1;
    fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"8%%\">%s</TH>\n", month[i]);
  }
  fprintf(of, "</THEAD>\n<TBODY>\n");
  fprintf(of, "<TR VALIGN=\"top\" ALIGN=\"center\">");
  for (int m=fm-1; m<fm+11; m++) {
    int i = (m%12)+1;
    fprintf(of, "<TD>");
    for (int z=1; z<=31; z++) {
      int d=50*i+z;
      if (played[d]) {
        if (r!=d)
          fprintf(of, "<A HREF=\"%s-d%d.html\"> %d </A>", sf, d, z);
        else
          fprintf(of, "<A HREF=\"%s-d%d.html\"><FONT COLOR=\"FF2200\"><B> %d </B></FONT></A>", sf, d, z);
      }
    }
    fprintf(of, "</TD>");
  }
  fprintf(of, "</TR>\n");
  fprintf(of, "</TBODY>\n</TABLE>\n");
//  fprintf(of, "</DIV>\n");
//  fprintf(of, "</SECTION>\n");
//  fprintf(of, "</SECTION>\n");
}

void DatesTable(int r) {
      int pr = r-1;
      while (pr>0 && played[pr]==0) pr--;
      if (pr==0) {
        pr = 649;
        while (pr>0 && played[pr]==0) pr--;
      }
      int nr = r+1;
      while (nr<649 && played[nr]==0) nr++;
      if (nr==649) {
        nr = 50;
        while (nr<649 && played[nr]==0) nr++;
      }
  int cm = r/50;
  int cd[50];
  int nd = 0;

  if (r!=first_d && pr!=first_d && first_d/50!=cm) { cd[nd++] = first_d; cd[nd++] = 0; }
  if (r!=first_d) {
    if (pr/50!=cm) { cd[nd++] = pr; cd[nd++] = cm;} else { cd[nd++] = cm; }
  }
  else {
    cd[nd++] = cm;
  }

  for (int i=1; i<=31; i++) {
    int z = 50*cm + i;
    if (played[z]) cd[nd++] = z;
  }


  if (r!=last_d && nr/50!=cm) { cd[nd++] = nr; }
  if (r!=last_d && nr!=last_d && last_d/50!=cm) { cd[nd++] = 0; cd[nd++] = last_d; }

  fprintf(of, "\n<TABLE FRAME=\"box\">\n");
  fprintf(of, "<TR>");
  for (int j=0; j<nd; j++) {
    int i = cd[j];
    if (played[i]) {
      fprintf(of, "<TD");
      if (i==r) fprintf(of, " BGCOLOR=\"DDDDDD\" ");
      if (i/50!=cm) fprintf(of, "><TD>|</TD><TD><A HREF=\"%s-d%d.html\">%d %s</A></TD>", sf, i, i%50, romonth[i/50]);
      if (i/50==cm) fprintf(of, "><A HREF=\"%s-d%d.html\">%d</A></TD>", sf, i, i%50);
    }
    else if (i==0) {
      fprintf(of, "<TD>...</TD>");
    }
    else if (i<13) {
      fprintf(of, "<TD> | </TD><TD> %s </TD>", romonth[i]);
    }
  }
  fprintf(of, "</TR>\n");
  fprintf(of, "</TABLE>\n");
//  fprintf(of, "</DIV>\n");
}

void printScoreLink(int h, int i, int j) {
  int rs = res[h][i][j];
  fprintf(of, "<A HREF=\"reports/%d/%d-%d-%d.html\"><FONT COLOR=\"black\">%d-%d</FONT></A>",
    year, id[i], id[j], round[h][i][j]%1000, rs/100, rs%100);
}

void printScore(int h, int i, int j, int rs) {
  int x = rs/100;
  int y = rs%100;
  if (y>=SPECIAL) {
    if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
      int z = y%10;
      fprintf(of, "<TD ALIGN=\"center\"><FONT COLOR=\"red\">p-%d</FONT></TD>", z);
    }
    return;
  }
  fprintf(of, "<TD ALIGN=\"center\"><A HREF=\"reports/%d/%d-%d-%d.html\">%d-%d</A></TD>",
    year, id[i], id[j], round[h][i][j]%1000, rs/100, rs%100);
}

void ResTable(int r) {
//  fprintf(of, "<SECTION class=\"topsec\">\n");
//  fprintf(of, "<SECTION>\n");
//  fprintf(of, "<DIV class=\"restab\">\n");

  int ng, xh[512], xr[512], xi[512], xj[512];
  fprintf(of, "\n<TABLE WIDTH=\"90%%\" BORDER=\"1\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<THEAD>\n<TR BGCOLOR=\"FFFFFF\">\n");
  fprintf(of, "<TH ALIGN=\"left\"   WIDTH=\"22%%\">Data</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"33%%\">Gazde</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"12%%\"></TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"   WIDTH=\"33%%\">Oaspeþi</TH>\n");

  /* collect */
  ng = 0;
  fprintf(of, "</TR>\n</THEAD>\n<TBODY>\n");
  for (int h=0; h<rr; h++) {
  for (int i=0; i<n; i++) {
  for (int j=0; j<n; j++) {
  if (
      (calendar==0 && round[h][i][j]/1000 == r && allres[h][i][j]>=0)
      ||
      (calendar==1 && round[h][i][j]%1000 == r && allres[h][i][j]>=0)
     )
    {
      xh[ng] = h; xi[ng] = i; xj[ng] = j; ng++;
    }
  }
  }
  }

  /* sort */
  int sorted = 0;
  int aux = 0;
  do {
    sorted = 1;
    for (int g=0; g<ng-1; g++) {
      int d1 = round[xh[g]][xi[g]][xj[g]]%1000;
      int d2 = round[xh[g+1]][xi[g+1]][xj[g+1]]%1000;
      if (after(d2, d1)) {
        aux = xh[g]; xh[g]=xh[g+1]; xh[g+1]=aux;
        aux = xi[g]; xi[g]=xi[g+1]; xi[g+1]=aux;
        aux = xj[g]; xj[g]=xj[g+1]; xj[g+1]=aux;
        sorted = 0;
      }
    }
  } while (!sorted);

  /* print */
  for (int g=0; g<ng; g++) {
    int h = xh[g];
    int i = xi[g];
    int j = xj[g];
    fprintf(of, "<TR BGCOLOR=\"DDDDDD\">");
    int rd = round[h][i][j]%1000;
    int rn = round[h][i][j]/1000;
    int rs = allres[h][i][j];
    int d = 1;
    if (winter) d=0;
    if (rd/50 < 8 && rn>numr/2) d = 0;
    int ty = year%100-d;
    if (ty < 0) ty += 100;
    fprintf(of, "<TD>%02d-%02d-%02d</TD>", rd%50, rd/50, ty);
    fprintf(of, "<TD ALIGN=\"right\">%s</TD>", NickOf(L, id[i], year));
    printScore(h, i, j, rs);
    fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NickOf(L, id[j], year));
    fprintf(of, "</TR>");
//    fprintf(of, "<TR><TD COLSPAN=\"4\"></TD><TR>");
    AddResult(h, i, j);
    if (dvz=='A') GetEvents(h, i, j);
    lastr = rd;
  }

  /* all dates unknown */
  if (first_r > 650 && r==numr-1) {
  for (int h=0; h<rr; h++) {
  for (int i=0; i<n; i++) {
  for (int j=0; j<n; j++) {
    AddResult(h, i, j);
    if (dvz=='A') GetEvents(h, i, j);
  }
  }
  }
  }

  fprintf(of, "</TBODY>\n</TABLE>\n");

//  fprintf(of, "</DIV>\n");
//  fprintf(of, "</SECTION>\n");
}

void trim(char *s) {
  if (s[1]==' ' || s[1]=='.') { s[1] = s[2]; s[2] = s[3]; }
  if (s[2]==' ' || s[2]=='.') { s[2] = s[3]; }
  s[3] = 0;
}

void GetInitial(int px, char *pini) {
    pini[1] = '.'; pini[2] = 0;
    if (px<0 || ppren[px]==NULL || ppren[px][0]==' ') pini[0] = 0;
    else if (strlen(ppren[px])>0) pini[0] = ppren[px][0];
    else pini[0] = 0;
}

void HTMLPlayerLink(int px, int full) {
  char pini[12];
  if (px<0) {
    fprintf(of, "?");
  }
  else {
    GetInitial(px, pini);
    makeHexlink(px);
    fprintf(of, "<a href=\"jucatori/%s.html\"><FONT COLOR=\"000000\" SIZE=-1>%s%s</FONT></a>",
      hexlink, (full?ppren[px]:pini), pname[px]);
  }
}

void TopScorersTable(int nts) {
  fprintf(of, "\n<TABLE  WIDTH=\"100%%\" BORDER=\"0\">\n");
  fprintf(of, "<TR><TD><I>Marcatori.</I> \n");
  int g = MAX_GOALS - 1;
  int k = 0;
  int j, over;
  while (k<nts && g>1) {
    while (nrnk[g]==0 && g>1) g--;
    fprintf(of, " [%d: ", g);
    j=0;
    over = k + nrnk[g] - nts;
    while (k<nts && j<nrnk[g]) {
      int p = prnk[g][j];
      if (j>0) fprintf(of, ", ");
      HTMLPlayerLink(p, 0);
//      fprintf(of, "%s", pname[p]);
      if (ppen[p]>0) fprintf(of, "<FONT SIZE=-2> (%dp)</FONT>", ppen[p]);
      j++; k++;
    }
    if (k==nts && j<nrnk[g]-1) fprintf(of, "<FONT SIZE=-1>, ... alþi %d</FONT>", over);
    fprintf(of, "]");
    g--;
  }
  fprintf(of, "</TD></TR></TABLE>\n");
}

void ClasTable(int r, int synoptic) {
  char sn[16];

  fprintf(of, "\n<TABLE  WIDTH=\"100%%\" BORDER=\"1\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<THEAD>\n<TR>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">#</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\"> </TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"20%%\">Echipa</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">J</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"3%%\">V</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">E</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">Î</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"4%%\">gm</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\">%c</TH>\n", (tbr%10<2?'-':':'));
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">gp</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"3%%\">P</TH>\n");
  if (synoptic) {
    fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"3%%\"></TH>\n");
    for (int k=0; k<n; k++) {
      strncpy(sn, NickOf(L, id[rank[k]], year), 5);
      trim(sn);
      fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"3%%\">%s</TH>\n", sn);
    }
  }
  fprintf(of, "</TR>\n</THEAD>\n<TBODY>\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    fprintf(of, "<TR ");
    if (NG>0) {
      if (i%(n/NG)<(pr1/NG)) fprintf(of, "BGCOLOR=\"77FF77\" ");
      else if (i%(n/NG)<(pr1+pr2)/NG) fprintf(of, "BGCOLOR=\"AAFFAA\" ");
      else if (i%(n/NG)>=(n-rel1)/NG) fprintf(of, "BGCOLOR=\"FF8888\" ");
      else if (i%(n/NG)>=(n-(rel1+rel2))/NG) fprintf(of, "BGCOLOR=\"F0E0B0\" ");
      else
      if (i%2==1) fprintf(of, "BGCOLOR=\"DDFFFF\" ");
    }
    else {
      if (i<pr1) fprintf(of, "BGCOLOR=\"77FF77\" ");
      else if (i<pr1+pr2) fprintf(of, "BGCOLOR=\"AAFFAA\" ");
      else if (i>=n-rel1) fprintf(of, "BGCOLOR=\"FF8888\" ");
      else if (i>=n-(rel1+rel2)) fprintf(of, "BGCOLOR=\"F0E0B0\" ");
      else
      if (i%2==1) fprintf(of, "BGCOLOR=\"DDFFFF\" ");
    }
    fprintf(of, ">\n");
    if (NG>0) {
      fprintf(of, "<TD ALIGN=\"right\">%d. </TD>\n", (i%(n/NG))+1);
    }
    else {
      fprintf(of, "<TD ALIGN=\"right\">%d. </TD>\n", i+1);
    }
    fprintf(of, "<TD ALIGN=\"center\" >");
    if (r>1 && prank[x] > i) {
      fprintf(of, "<IMG SRC=\"sus.gif\"></IMG></TD>\n");
    }
    else if (r>1 && prank[x] < i) {
      fprintf(of, "<IMG SRC=\"jos.gif\"></IMG></TD>\n");
    }
    else {
      fprintf(of, "</TD>\n");
    }
    fprintf(of, "<TD ALIGN=\"left\"><A HREF=\"istoric-%d.html\">%s %s</A></TD>\n", id[x], NameOf(L, id[x], year), desc[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", win[x]+drw[x]+los[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", win[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", drw[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", los[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", gsc[x]);
    fprintf(of, "<TD ALIGN=\"center\">%c</TD>\n", (tbr%10<2?'-':':'));
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", gre[x]);
    int points =  ppv*win[x]+drw[x];
    if (pen[x]!=0 && after(pdt[x],lastr)) points -= pen[x];
    if (r > split_after) points = pts[x];
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", points);
    if (synoptic) {
      fprintf(of, "<TD ALIGN=\"center\" </TD>");
      for (int k=0; k<n; k++) {
        fprintf(of, "<TD ALIGN=\"center\" ");
        if (i==k) fprintf(of, "BGCOLOR=\"DDDDDD\" ");
        fprintf(of, ">");
        int y = rank[k];
        for (int h=0; h<rr; h++) {
          int sc = res[h][x][y];
          if (sc>=0) {
            if (h>0) fprintf(of, ",");
            int sy = sc%100;
            if (sy>=SPECIAL) {
              if (sy>=LOSS_BOTH_0 && sy<=LOSS_BOTH_9) {
                int yy = sy%10;
                fprintf(of, "<FONT COLOR=\"red\">p-%d</FONT>", yy);
              }
            }
            else {
//              fprintf(of, "%d-%d", sc/100, sc%100);
                printScoreLink(h, x, y);
            }
          }
        }
        fprintf(of, "</TD>");
      }
    }
    fprintf(of, "</TR>\n");
    if (NG>0 && (i+1)==n/2) {
      fprintf(of, "<TR><TD>---</TD></TR>\n");
    }
  }
  fprintf(of, "</TBODY>\n</TABLE>\n");
}

void HomeAwayTable() {
  char sn[16];
  HRanking();
  GRanking();

  fprintf(of, "<BR>\n<TABLE  WIDTH=\"100%%\" BORDER=\"1\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<THEAD>\n<TR>\n");

  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">#</TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"20%%\">Acasã</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">J</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"3%%\">V</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">E</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">Î</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"4%%\">gm</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\">%c</TH>\n", (tbr%10<2?'-':':'));
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">gp</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"3%%\">P</TH>\n");

  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"5%%\"></TH>\n");

  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">#</TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"20%%\">Deplasare</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">J</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"3%%\">V</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">E</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">Î</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"4%%\">gm</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\">%c</TH>\n", (tbr%10<2?'-':':'));
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"2%%\">gp</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"3%%\">P</TH>\n");

  fprintf(of, "</TR>\n</THEAD>\n<TBODY>\n");
  for (int i=0; i<n; i++) {
    int x = hrank[i];
    fprintf(of, "<TR ");
    if (i%2==1) fprintf(of, "BGCOLOR=\"DDFFFF\" ");
    fprintf(of, ">\n");
    fprintf(of, "<TD ALIGN=\"right\">%d. </TD>\n", i+1);
    fprintf(of, "<TD ALIGN=\"left\"><A HREF=\"istoric-%d.html\">%s</A></TD>\n", id[x], NameOf(L, id[x], year));
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", hwin[x]+hdrw[x]+hlos[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", hwin[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", hdrw[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", hlos[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", hgsc[x]);
    fprintf(of, "<TD ALIGN=\"center\">%c</TD>\n", (tbr%10<2?'-':':'));
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", hgre[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", ppv*hwin[x]+hdrw[x]);

    fprintf(of, "<TD></TD>\n");
    x = grank[i];

    fprintf(of, "<TD ALIGN=\"right\">%d. </TD>\n", i+1);
    fprintf(of, "<TD ALIGN=\"left\"><A HREF=\"istoric-%d.html\">%s</A></TD>\n", id[x], NameOf(L, id[x], year));
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", gwin[x]+gdrw[x]+glos[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", gwin[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", gdrw[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", glos[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", ggsc[x]);
    fprintf(of, "<TD ALIGN=\"center\">%c</TD>\n", (tbr%10<2?'-':':'));
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", ggre[x]);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>\n", ppv*gwin[x]+gdrw[x]);

    fprintf(of, "</TR>\n");
  }
  fprintf(of, "</TBODY>\n</TABLE>\n");

}

void PosTable(int r) {
  fprintf(of, "<BR>\n<TABLE BORDER=\"1\" RULES=\"all\" FRAME=\"box\">\n");
  fprintf(of, "<THEAD>\n<TR>\n");
  fprintf(of, "<TH ALIGN=\"left\">Echipa / Loc</TH>\n");
  for (int i=1; i<=r; i++)
    fprintf(of, "<TH ALIGN=\"right\">%02d</TH>\n", i);
  fprintf(of, "</TR>\n</THEAD>\n<TBODY>\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    fprintf(of, "<TR>");
    fprintf(of, "<TD ");
    if (i%2==1) fprintf(of, "BGCOLOR=\"DDFFFF\"");
    fprintf(of, ">%s</TD>", NickOf(L, id[x], year));
    for (int j=1; j<=r; j++) {
      int loc = pos[x][j]-1;
     fprintf(of, "<TD ALIGN=\"right\" ");
      if (loc<pr1) fprintf(of, "BGCOLOR=\"77FF77\"");
      else if (loc<pr1+pr2) fprintf(of, "BGCOLOR=\"AAFFAA\"");
      else if (loc>=n-rel1) fprintf(of, "BGCOLOR=\"FF8888\"");
      else if (loc>=n-(rel1+rel2)) fprintf(of, "BGCOLOR=\"F0E0B0\"");
      else
      if (i%2==1) fprintf(of, "BGCOLOR=\"DDFFFF\"");
     fprintf(of, ">%d</TD>", pos[x][j]);
    }
    fprintf(of, "</TR>\n");
  }
  fprintf(of, "</TBODY>\n</TABLE>\n");
}

void TableHeader() {
  fprintf(of, "<BR>\n");
  fprintf(of, "\n<TABLE BGCOLOR=\"DDDDDD\" BORDER=\"0\" RULES=\"rows\" FRAME=\"box\">\n");
}

void PrintRound(int r) {
  if (r == split_after + 1) {
    AdjustStats();
  }
  HTMLHeader(r);
  fprintf(of, "\n<TABLE WIDTH=\"99%%\" BORDER=\"0\">\n");
  fprintf(of, "<TD ALIGN=\"left\" VALIGN=\"top\">\n");
  ResTable(r);
  fprintf(of, "</TD>");
  fprintf(of, "<TD WIDTH=\"50%%\" ALIGN=\"left\" VALIGN=\"top\">\n");
  NavTable(r);
  RoundsTable(r);
  fprintf(of, "</TD>\n</TABLE>");
  Ranking(r);
  for (int t=0; t<n; t++) pos[rank[t]][r] = t+1;
  if (dvz=='A') { TopScorers(); TopScorersTable(10); }
  ClasTable(r, 1);
  HomeAwayTable();
  if (r>1) PosTable(r);
  HTMLFooter();
}

void PrintDate(int i) {
  HTMLHeader(i);
  fprintf(of, "\n<TABLE WIDTH=\"99%%\" BORDER=\"0\">\n");
  fprintf(of, "<TD ALIGN=\"left\" VALIGN=\"top\">\n");
  ResTable(i);
  fprintf(of, "</TD>");
  fprintf(of, "<TD WIDTH=\"50%%\" ALIGN=\"left\" VALIGN=\"top\">\n");
  NavTable(i);
  CalendarTable(i);
  fprintf(of, "</TD>\n</TABLE>");
  Ranking(0);
  if (dvz=='A') { TopScorers(); TopScorersTable(10); }
  ClasTable(i, 1);
  HomeAwayTable();
  HTMLFooter();
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  char filename[256];
  if (!Load()) {
    printf("ERROR: called from invalid drectory.\n");
    return -1;
  }
  if (argc < 2) {
    printf("ERROR: No input file specified.\n");
    return -1;
  }

  sprintf(sf, "%s", argv[1]);
  dvz = sf[0] - 32;
  char spl[3];
  if (sf[1]=='.') seria = 0;
  else {
    spl[0] = sf[1]; spl[1] = 0;
    if (sf[2]!='.') {
      spl[1] = sf[2]; spl[2] = 0;
    }
    seria = atoi(spl);
  }
  if (!LoadFile(argv[1])) {
    printf("ERROR: file %s not found.\n", argv[1]);
    return -2;
  }

  split_after = 1000;
  split_top = 0;
  split_points = 0;
  cancel_gdiff = 0;

  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  if (ystr) year = atoi(ystr); else year = 0;
  bool full = true;
  winter = false;
  fd = -1;
  ld = -1;
  NG = 0;
  decorate = 1;
  calendar = 0;
  for (int i=0; i<650; i++) played[i] = 0;
  for (int i=0; i<650; i++) for (int j=0; j<20; j++) sched[i][j] = 0;
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
        else if (strcmp(argv[i],"-c")==0) {
          calendar = 1;
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
        else if (strcmp(argv[i], "-split")==0 && i<argc-2) {
          split_after = atoi(argv[++i]);
          split_top = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-pts")==0 && i<argc-1) {
          split_points = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-gd")==0 && i<argc-1) {
          cancel_gdiff = atoi(argv[++i]);
        }
        else
          printf("Unknown option: %s\n", argv[i]);
      }
      else printf("Illegal command line argument: %s\n", argv[i]);
      i++;
    } while (i<argc);
  }
  if (fd < 0) {
    FindFirstRoundDate();
    fd = first_r;
  }
  if (fd<0) fd = 415;
  if (ld<0) ld = fd+649;

  CollectSched(1);
  if (dvz=='A') {
    LoadPlayers();
    LoadEvents();
  }

  if (calendar) {
    int nd = 0;
    for (int i=fd; i<=ld; i++) {
      int r = i%650;
      if (played[r]) {
        sprintf(filename, "html/%s-d%d.html", sf, r);
        of = fopen(filename, "wt");
        if (of) PrintDate(r);
        fclose(of);
      }
    }
  }
  else {
    for (int i=1; i<=numr; i++) {
      sprintf(filename, "html/%s-r%d.html", sf, i);
      of = fopen(filename, "wt");
      if (of) PrintRound(i);
      fclose(of);
    }
  }

  return 0;
}
