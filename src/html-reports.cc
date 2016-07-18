#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"

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

#define MAX_LEVELS  12
#define MAX_RR	     4
#define MAX_N       64
#define MAX_TEAMS 2000

#define MAX_NAMES       20000
#define DB_ROWS         400
#define DB_COLS         60
#define DB_CELL         20
#define EV_COLS		30
#define PL_INITIAL	 0
#define PL_FULL_NAME	 1

#define DB_HOME		 0
#define DB_GUEST	 1
#define DB_SCORE	 2
#define DB_DATE		 3
#define DB_COMP		 4
#define DB_ROUND	 5
#define DB_VENUE	 6
#define DB_ATTEND	 7
#define DB_WEATHER	 8
#define DB_ROSTER1	 9
#define DB_COACH1	31
#define DB_ROSTER2	32
#define DB_COACH2	54
#define DB_REF		55
#define DB_ASIST1	56
#define DB_ASIST2	57
#define DB_OBSERV	58
#define DB_T1		 8
#define DB_T2		31

#define EV_GOAL      0
#define EV_OWNGOAL   1
#define EV_PKGOAL    2
#define EV_PKMISS    3
#define EV_YELLOW    4
#define EV_RED       5
#define EV_YELLOWRED 6

#define SPECIAL		50
#define LOSS_BOTH_0	50
#define LOSS_BOTH_9	59

#define ROSTER_SIZE 22

const char *month[] = {"", "Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
                     "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"};
const char *romonth[] = {"", "ianuarie", "februarie", "martie", "aprilie", "mai", "iunie",
                     "iulie", "august", "septembrie", "octombrie", "noiembrie", "decembrie"};
const char *evicon[] = {"g", "og", "pg", "pm", "cg" , "cr", "cgr"};

char **club;
char **mnem;
int  NC;
int  id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pts[MAX_N], pen[MAX_N], pdt[MAX_N];
int  asez[MAX_TEAMS], awin[MAX_TEAMS], adrw[MAX_TEAMS], alos[MAX_TEAMS], agre[MAX_TEAMS], agsc[MAX_TEAMS], arank[MAX_TEAMS];
int  round[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N], allres[MAX_RR][MAX_N][MAX_N];
int  rank[MAX_N];
int  pos[MAX_N][365];
char desc[MAX_N][32];
char ssn[32];

int  lid[MAX_N], lwin[MAX_N], ldrw[MAX_N], llos[MAX_N], lgsc[MAX_N], lgre[MAX_N], lpts[MAX_N], lpen[MAX_N], lpdt[MAX_N];
int  hwin[MAX_N], hdrw[MAX_N], hlos[MAX_N], hgsc[MAX_N], hgre[MAX_N], hpts[MAX_N];
int  gwin[MAX_N], gdrw[MAX_N], glos[MAX_N], ggsc[MAX_N], ggre[MAX_N], gpts[MAX_N];
int  prank[MAX_N], hrank[MAX_N], grank[MAX_N];
int  tbwin[MAX_N], tbdrw[MAX_N], tblos[MAX_N], tbgsc[MAX_N], tbgre[MAX_N], tbrk[MAX_N];
int  topsc[MAX_NAMES];
int  n, rr, r, lastr, lastrnd, tbr, pr1, pr2, rel1, rel2, ppv, year, numr;
int  NG;
bool winter;
int num_winter;
int *start_winter, *end_winter;
int roster[2*ROSTER_SIZE], annotation[2*ROSTER_SIZE];
int nev, evp[EV_COLS], evm[EV_COLS], evt[EV_COLS];

/* *************************************** */

char  db[DB_ROWS][DB_COLS][DB_CELL];
char edb[DB_ROWS][EV_COLS][DB_CELL];

int NP;
char **pmnem, **pname, **ppren, **pdob, **pcty, **ppob, **pjud;
int *psez, *pmeci, *ptit, *pint, *prez, *pban, *pmin, *pgol, *pgre, *prnk;
int *csez, *cmeci, *ctit, *cint, *crez, *cban, *cmin, *cgol, *cgre, *crnk;

//---------------------------
char *hexlink = new char[32];
void makeHexlink(char *str) {
  sprintf(hexlink, "%x%x%x%x%x%x",
    ((unsigned char)str[0]),
    ((unsigned char)str[1]),
    ((unsigned char)str[2]),
    ((unsigned char)str[3]),
    ((unsigned char)str[4]),
    ((unsigned char)str[5]));
  hexlink[12]=0;
}
//---------------------------

Catalog Ant, Arb;
Locations Loc;

int borna[256];

int  fd, ld, fr, frd, lr, lrd, decorate, calendar;
FILE *of;
char dvz;
int  seria;
char sf[128];
int  played[650];
int  sched[650][24];
int  num_sched, first_d, first_r, last_d, last_r;

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
  for (int i=0; i<NC; i++) {
    asez[i] = awin[i] = alos[i] = agsc[i] = agre[i] = 0;
    arank[i] = i;
  }
  return 1;
}

void ResetStats() {
  for (int i=0; i<NP; i++) {
     psez[i] = pmeci[i] = ptit[i] = pint[i] = prez[i] = pban[i] = pmin[i] = pgol[i] = pgre[i] = 0;
     prnk[i] = i;
  }
}

void InitStats() {
  for (int i=0; i<NP; i++) {
     csez[i] = cmeci[i] = ctit[i] = cint[i] = crez[i] = cban[i] = cmin[i] = cgol[i] = cgre[i] = 0;
     topsc[i] = crnk[i] = i;
  }
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

  psez  = new int[MAX_NAMES];
  pmeci = new int[MAX_NAMES];
  ptit  = new int[MAX_NAMES];
  pint  = new int[MAX_NAMES];
  prez  = new int[MAX_NAMES];
  pban  = new int[MAX_NAMES];
  pmin  = new int[MAX_NAMES];
  pgol  = new int[MAX_NAMES];
  pgre  = new int[MAX_NAMES];
  prnk  = new int[MAX_NAMES];

  csez  = new int[MAX_NAMES];
  cmeci = new int[MAX_NAMES];
  ctit  = new int[MAX_NAMES];
  cint  = new int[MAX_NAMES];
  crez  = new int[MAX_NAMES];
  cban  = new int[MAX_NAMES];
  cmin  = new int[MAX_NAMES];
  cgol  = new int[MAX_NAMES];
  cgre  = new int[MAX_NAMES];
  crnk  = new int[MAX_NAMES];

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

  InitStats();
  ResetStats();
}

int LoadAccumulatedStats() {
  char filename[128];
  sprintf(filename, "data/career-%d.dat", year-1);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: File %s not found.\n", filename);
     return -1;
  }
  int imc, itt, irz, igl, imn, iin, ibn, igr;
  for (int i=0; i<NP; i++) {
    fscanf(f, "%d %d %d %d %d %d %d %d\n", &imc, &itt, &irz, &igl, &imn, &iin, &ibn, &igr);
    cmeci[i] = imc;
    ctit[i]  = itt;
    crez[i]  = irz;
    cgol[i]  = igl;
    cmin[i]  = imn;
    cint[i]  = iin;
    cban[i]  = ibn;
    cgre[i]  = igr;
  }
  fclose(f);
  sprintf(filename, "data/alltime-%d.dat", year-1);
  f = fopen(filename, "rt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: File %s not found.\n", filename);
     return -1;
  }
  int az, aw, ad, al, as, ar;
  for (int i=0; i<NC; i++) {
    fscanf(f, "%d %d %d %d %d %d\n", &az, &aw, &ad, &al, &as, &ar);
    asez[i] = az;
    awin[i] = aw;
    adrw[i] = ad;
    alos[i] = al;
    agsc[i] = as;
    agre[i] = ar;
  }
  fclose(f);
  for (int i=0; i<NC; i++) arank[i] = i;
  return 0;
}

int LoadPlayerStats() {
  char filename[128];
  sprintf(filename, "data/career-%d.dat", year-1);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: File %s not found.\n", filename);
     return -1;
  }
  int imc, itt, irz, igl, imn, iin, ibn, igr;
  for (int i=0; i<NP; i++) {
    fscanf(f, "%d %d %d %d %d %d %d %d\n", &imc, &itt, &irz, &igl, &imn, &iin, &ibn, &igr);
    cmeci[i] = imc;
    ctit[i]  = itt;
    crez[i]  = irz;
    cgol[i]  = igl;
    cmin[i]  = imn;
    cint[i]  = iin;
    cban[i]  = ibn;
    cgre[i]  = igr;
  }
  fclose(f);
  return 0;
}

int LoadAlltimeStats() {
  char filename[128];
  sprintf(filename, "data/alltime-%d.dat", year-1);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: File %s not found.\n", filename);
     return -1;
  }
  int az, aw, ad, al, as, ar;
  for (int i=0; i<NC; i++) {
    fscanf(f, "%d %d %d %d %d %d\n", &az, &aw, &ad, &al, &as, &ar);
    asez[i] = az;
    awin[i] = aw;
    adrw[i] = ad;
    alos[i] = al;
    agsc[i] = as;
    agre[i] = ar;
    arank[i] = i;
  }
  fclose(f);
}

int SaveAccumulatedStats() {
  char filename[128];
  sprintf(filename, "data/career-%d.dat", year);
  FILE *f = fopen(filename, "wt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: Could not open %s.\n", filename);
     return -1;
  }
  for (int i=0; i<NP; i++) {
    fprintf(f, "%3d %3d %3d %3d %6d %3d %3d %3d\n",  cmeci[i], ctit[i], crez[i], cgol[i], cmin[i], cint[i], cban[i], cgre[i]);
  }
  fclose(f);
  sprintf(filename, "data/alltime-%d.dat", year);
  f = fopen(filename, "wt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: Could not open %s.\n", filename);
     return -1;
  }
  for (int i=0; i<NC; i++) {
    fprintf(f, "%3d %5d %5d %5d %7d %7d\n",  asez[i], awin[i], adrw[i], alos[i], agsc[i], agre[i]);
  }
  fclose(f);
  return 0;
}

int FindMnem(char *mnem) {
  int c = ((unsigned char)mnem[0]);
  for (int i=borna[c]; i<borna[c+1]; i++)
    if (strcmp(pmnem[i], mnem)==0) return i;
  return -1;
}

void GetInitial(int px, char *pini) {
    pini[1] = '.'; pini[2] = 0;
    if (px<0 || ppren[px]==NULL || ppren[px][0]==' ') pini[0] = 0;
    else if (strlen(ppren[px])>0) pini[0] = ppren[px][0];
    else pini[0] = 0;
}

void AddStats(int px, int k, int m) {
  if (px<0 || px>=NP) return;
  if (m<0) return;
  pmin[px] += m;
  cmin[px] += m;
  if (m>0) { pmeci[px]++; cmeci[px]++; }
  if (k>=1 && k<=11) {
    ptit[px]++; ctit[px]++;
    if (m==90) { pint[px]++; cint[px]++; }
  }
  else if (k>=12 && k<=14) {
    if (m>0) { prez[px]++; crez[px]++; }
  }
  else if (k<=ROSTER_SIZE) {
    if (m==0) { pban[px]++; cban[px]++; }
  }
}

void LoadDB() {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<rr*n*(n-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(db[i][j], tk[j]);
      else strcpy(db[i][j], " ");
    }
  }
  fclose(f);
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
    if (tbwin[i] > tbwin[j]) return 1;
    if (tbwin[i] < tbwin[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
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
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
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

int asup(int i, int j) {
  int p1 = 2*awin[i]+adrw[i];
  int p2 = 2*awin[j]+adrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (p1==0 && p2==0) {
      if (asez[i]>asez[j]) return 1;
      if (asez[i]<asez[j]) return 0;
    }
    if (agsc[i] - agre[i] > agsc[j] - agre[j]) return 1;
    if (agsc[i] - agre[i] < agsc[j] - agre[j]) return 0;
    if (awin[i] > awin[j]) return 1;
    if (awin[i] < awin[j]) return 0;
    if (agsc[i] > agsc[j]) return 1;
    if (agsc[i] < agsc[j]) return 0;
    if (i > j) return 0;
    return 1;
}

void Tiebreak(int k, int l) {
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
    for (int i=k; i<k; i++)
      if (sup(tbrk[i+1], tbrk[i], (tbr%NUMORD!=RATIO ? gm>=(l-k+1)*(l-k) : RATIO))) {
        sorted = 0;
        int aux = tbrk[i];
        tbrk[i] = tbrk[i+1];
        tbrk[i+1] = aux;
      }
  } while (!sorted);
  for (int i=k; i<=l; i++)
    rank[i] = tbrk[i];
}


void Ranking() {
  // BubbleSort
  int i, j;
  for (i=0; i<n; i++) prank[rank[i]] = i;
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

void ARanking() {
  int i, j;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<NC-1; i++) {
      if (asup(arank[i+1], arank[i])) {
        sorted = 0;
        int aux = arank[i];
        arank[i] = arank[i+1];
        arank[i+1] = aux;
      }
    }
  } while (!sorted);
}

void TopScorers() {
  int i, j;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<NP-1; i++) {
      if (cgol[crnk[i+1]] > cgol[crnk[i]]) {
        sorted = 0;
        int aux = crnk[i];
        arank[i] = crnk[i+1];
        crnk[i+1] = aux;
      }
    }
  } while (!sorted);
}

void FindFirstRound() {
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

int FindPos(int *rk, int n, int a) {
  for (int i=0; i<n; i++)
    if (rk[i]==a) return i+1;
  return 0;
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
      agre[id[a]] += z; agre[id[b]] += z;
      alos[id[a]]++; alos[id[b]]++;
    }
    return 2;
  }
  gsc[a]  += x; gre[a]  += y;
  hgsc[a] += x; hgre[a] += y;
  gsc[b]  += y; gre[b]  += x;
  ggsc[b] += y; ggre[b] += x;
  agsc[id[a]] += x; agre[id[a]] += y;
  agsc[id[b]] += y; agre[id[b]] += x;
  if (x>y) {
    win[a]++; los[b]++; pts[a] += ppv;
    awin[id[a]]++; alos[id[b]]++;
    hwin[a]++; glos[b]++; hpts[a] += ppv;
  }
  else if (x==y) {
    drw[a]++; drw[b]++; pts[a]++; pts[b]++;
    adrw[id[a]]++; adrw[id[b]]++;
    hdrw[a]++; gdrw[b]++; hpts[a]++; gpts[b]++;
  }
  else {
    los[a]++; win[b]++; pts[b] += ppv;
    alos[id[a]]++; awin[id[b]]++;
    hlos[a]++; gwin[b]++; gpts[b] += ppv;
  }
  return 1;
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
    asez[id[i]]++;
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
      allres[h][i][j] = z;
      res[h][i][j] = -1;
      int tr = round[h][i][j]/1000;
      int td = round[h][i][j]%1000;
      if (h>0 && tr <= numr/rr) {
        /* autocorrect multiple round robin */
        round[h][i][j] += 1000*h*(numr/rr);
        tr += 1000*h*(numr/rr);
      }
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

int MatchID(int r, int a, int b) {
  int offset = r*n*(n-1);
  int mid = a*(n-1)+b-(b>a?1:0);
  return offset + mid;
}

int consecutive(int r, int last) {
  if (last%50>=30 && r%50==1) return 0;
  if (last>0 && (r-last>1 || r<last)) return 1;
  return 0;
}

int isWinterSeason(int y) {
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) return 1;
  }
  return 0;
}

int getYear(int y, int r, int z) {
  if (isWinterSeason(y)) return y;
  if (z/50>7) return y-1;
  if (z/50==7) {
    if (r<n-1) return y-1;
  }
  return y;
}

void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);
  else sprintf(ss, "%d/%02d", y-1, y%100);
}


void HTMLHeader(int r, int a, int b) {
  char ss[32], sdate[32];
  SeasonName(year, ss);
  fprintf(of, "<!DOCTYPE html>\n");
  fprintf(of, "<head>\n");
  int z = round[r][a][b]%1000;
  fprintf(of, "  <title>%s vs. %s - %d %s</title>\n", NameOf(L, id[a], year), NameOf(L, id[b], year), z%50, romonth[z/50]);
  fprintf(of, "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\" />\n");
  fprintf(of, "<link rel=\"stylesheet\" href=\"../../css/report.css\" type=\"text/css\" charset=\"iso-8859-2\" />\n");
  fprintf(of, "</head>\n");
  fprintf(of, "<body id=\"pmain\">\n\n");
  fprintf(of, "  <div class=\"block  clearfix block_match_info-wrapper\" id=\"i4w\">\n");
  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "  <div class=\"block_match_info real-content clearfix \" id=\"i4\">\n\n");

}

void HTMLScoreBlock(int r, int a, int b) {
  fprintf(of, "  <div class=\"clearfix\">\n");
  fprintf(of, "  <div class=\"container left\">\n");
  fprintf(of, "    <h3 class=\"thick\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">%s</a>\n", id[a], NameOf(L, id[a], year));
  fprintf(of, "    </h3>\n");
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container middle\">\n");
  fprintf(of, "    <h1 class=\"thick scoretime \">\n");
  fprintf(of, "    <a href=\"../../vs-%04d/vs-%d-%d.html\">", id[a], id[a], id[b]);
  fprintf(of, "      %d - %d\n", allres[r][a][b]/100, allres[r][a][b]%100);
  fprintf(of, "    </a></h1>\n");
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container right\">\n");
  fprintf(of, "    <h3 class=\"thick\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">%s</a>\n", id[b], NameOf(L, id[b], year));
  fprintf(of, "    </h3>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "  </div>\n\n");
}

int isNumeric(char *s) {
	if (s==NULL) return 0;
	int l = strlen(s);
	for (int i=0; i<l; i++) {
		if (s[i]<'0' || s[i]>'9') return 0;
	}
	return 1;
}

void toThousands(char *s, char *t) {
	if (!isNumeric(s)) t[0] = 0;
	int vs = atoi(s);
	int vth = vs/1000;
	int vun = vs%1000;
	if (vth>0) sprintf(t, "%d %03d", vth, vun);
	else sprintf(t, "%s", s);
}

void HTMLInfoBlock(int r, int a, int b) {
  fprintf(of, "  <div class=\"clearfix\">\n\n");

	char sw[128], *sd, *sh, satt[128];
  int mid = MatchID(r, a, b);
  int rg1 = FindPos(rank, n, a);
  int rg2 = FindPos(rank, n, b);
  int rh  = FindPos(hrank, n, a);
  int rg  = FindPos(grank, n, b);
  int ia  = id[a];
  int ib  = id[b];
  int ar1 = FindPos(arank, NC, ia);
  int ar2 = FindPos(arank, NC, ib);

  fprintf(of, "  <div class=\"container left\">\n");
  fprintf(of, "    <div class=\"logo\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">\n", id[a]);
  fprintf(of, "        <img src=\"../../logo/logo-%d.png\" height=\"150\"/>\n", id[a]);
  fprintf(of, "      </a>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "    <table class=\"seasonform\">\n");
  fprintf(of, "      <tr><td>All-time (%d)</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     asez[ia], ar1, awin[ia]+adrw[ia]+alos[ia], awin[ia], adrw[ia], alos[ia], agsc[ia], agre[ia], 2*awin[ia]+adrw[ia]);
  fprintf(of, "      <tr><td>Sezon</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rg1, win[a]+drw[a]+los[a], win[a], drw[a], los[a], gsc[a], gre[a], ppv*win[a]+drw[a]);
  fprintf(of, "      <tr><td>Acasã</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rh, hwin[a]+hdrw[a]+hlos[a], hwin[a], hdrw[a], hlos[a], hgsc[a], hgre[a], ppv*hwin[a]+hdrw[a]);
  fprintf(of, "    </table>\n");
  fprintf(of, "  </div>\n\n");

  int q = round[r][a][b]/1000;
  int z = round[r][a][b]%1000;

  fprintf(of, "  <div class=\"container middle\">\n");
  fprintf(of, "    <div class=\"details clearfix\">\n");
  fprintf(of, "      <dl>\n");
  fprintf(of, "        <dt>Competiþia</dt>\n");
  fprintf(of, "        <dd>Divizia %c  %s</dd>\n", dvz, ssn);
	int etp = round[r][a][b]/1000;
	int cld = round[r][a][b]%1000;
	if (etp>0) {
	  fprintf(of, "        <dd><A HREF=\"../../%c.%d-r%d.html\">Etapa %d</A></dd>\n\n", dvz+32, year, etp, etp);
	}

	strcpy(sw, db[mid][DB_DATE]);
	sd = strtok(sw, "@");
	sh = strtok(NULL, ",");
  fprintf(of, "        <dt>Data</dt>\n");
  fprintf(of, "        <dd><A HREF=\"../../%c.%d-d%d\">%d %s %d</A></dd>\n",
		dvz+32, year, cld,
		z%50, romonth[z/50], getYear(year, q, z));
  fprintf(of, "        <dd>Ora %s</dd>\n", (sh?sh:"00:00"));
  fprintf(of, "      </dl>\n\n");

  fprintf(of, "      <dl>\n");
	  fprintf(of, "        <dt>Stadion</dt>\n");
	int stad = Loc.FindVenue(db[mid][DB_VENUE]);
	if (stad>=0) {
		int ci = Loc.V[stad].city;
  	fprintf(of, "        <dd> %s, %s </dd>\n", Loc.V[stad].getName(year), Loc.C[ci].name);
	}
  fprintf(of, "        <dd> </dd>\n");
  fprintf(of, "        <dt>Spectatori</dt>\n");
	toThousands(db[mid][DB_ATTEND], satt);
  fprintf(of, "        <dd> %s </dd>\n", satt);
  fprintf(of, "      </dl>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container right\">\n");
  fprintf(of, "    <div class=\"logo\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">\n", id[b]);
  fprintf(of, "        <img src=\"../../logo/logo-%d.png\" height=\"150\"/>\n", id[b]);
  fprintf(of, "      </a>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "    <table class=\"seasonform\">\n");
  fprintf(of, "      <tr><td>All-time (%d) </td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     asez[ib], ar2, awin[ib]+adrw[ib]+alos[ib], awin[ib], adrw[ib], alos[ib], agsc[ib], agre[ib], 2*awin[ib]+adrw[ib]);
  fprintf(of, "      <tr><td>Sezon</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rg2, win[b]+drw[b]+los[b], win[b], drw[b], los[b], gsc[b], gre[b], ppv*win[b]+drw[b]);
  fprintf(of, "      <tr><td>Deplasare</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rg, gwin[b]+gdrw[b]+glos[b], gwin[b], gdrw[b], glos[b], ggsc[b], ggre[b], ppv*gwin[b]+gdrw[b]);
  fprintf(of, "    </table>\n");
  fprintf(of, "  </div>\n\n");
  fprintf(of, "</div>\n");

  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void ResetRoster() {
  for (int i=0; i<2*ROSTER_SIZE; i++) {
    roster[i] = -1;
    annotation[i] = 0;
  }
}

void GetRoster(int r, int a, int b) {
  int mid = MatchID(r, a, b);
  char s[DB_CELL], *sp, *sm;
  int rp, rm;
  ResetRoster();
  for (int i=DB_ROSTER1; i<DB_COACH1; i++) {
    strcpy(s, db[mid][i]);
    sp = strtok(s, ":");
    sm = strtok(NULL, ",\n");
    if (sm!=NULL) rm = atoi(sm); else rm=-1;
    if (sp!=NULL) rp = FindMnem(sp); else rp=-1;
    if (rm>=0 && rp>=0) roster[i-DB_ROSTER1] = rp;
  }
  for (int i=DB_ROSTER2; i<DB_COACH2; i++) {
    strcpy(s, db[mid][i]);
    sp = strtok(s, ":");
    sm = strtok(NULL, ",\n");
    if (sm!=NULL) rm = atoi(sm); else rm=-1;
    if (sp!=NULL) rp = FindMnem(sp); else rp=-1;
     if (rm>=0 && rp>=0) roster[ROSTER_SIZE+i-DB_ROSTER2] = rp;
  }
}

int RosterIdx(int px) {
  if (px<0) return -1;
  for (int i=0; i<44; i++)
    if (roster[i] == px) return i;
  return -1;
}

void ResetEvents() {
  nev = 0;
  for (int i=0; i<EV_COLS; i++) { evp[i] = -1; evm[i] = -1; evt[i] = -1; }
}

void GetEvents(int r, int a, int b) {
  int mid = MatchID(r, a, b);
  int ep, em;
  char s[DB_CELL], *sp, *sm;
  ResetEvents();
  for (int i=0; i<EV_COLS; i++) {
    strcpy(s, edb[mid][i]);
    sp = strtok(s, "'`\"/,#!\n");
    sm = strtok(NULL, "'`\"/,#!\n");
    if (sm!=NULL) em = atoi(sm); else em=-1;
    if (sp!=NULL) ep = FindMnem(sp); else ep=-1;
    if (ep>=0 || em>=0) {
      if (em==0) em=999;
      evp[i] = ep; evm[i] = em; evt[i] = EV_GOAL;
           if (edb[mid][i][6]=='`') evt[i] = EV_OWNGOAL;
      else if (edb[mid][i][6]=='"') evt[i] = EV_PKGOAL;
      else if (edb[mid][i][6]=='/') evt[i] = EV_PKMISS;
      else if (edb[mid][i][6]=='!') evt[i] = EV_RED;
      nev++;
    }
  }
}

void SortEvents() {
  int sorted, aux, last;
  do {
    sorted = 1;
    for (int i=0; i<nev-1; i++) {
      if (evm[i+1]<evm[i]) {
        sorted = 0;
        aux = evp[i]; evp[i] = evp[i+1]; evp[i+1] = aux;
        aux = evm[i]; evm[i] = evm[i+1]; evm[i+1] = aux;
        aux = evt[i]; evt[i] = evt[i+1]; evt[i+1] = aux;
      }
    }
  } while (!sorted);
  for (int i=0; i<nev; i++) {
    evp[i] = RosterIdx(evp[i]);
    if (evp[i]>=0 && evp[i]<2*ROSTER_SIZE) {
      if (evt[i] == EV_RED) {
        annotation[evp[i]] = evm[i];
      }
    }
  }
}

void HTMLPlayerLink(int px, int full) {
  char pini[12];
  if (px<0) {
    fprintf(of, "?");
  }
  else {
    GetInitial(px, pini);
    makeHexlink(pmnem[px]);
    fprintf(of, "<a href=\"../../jucatori/%s.html\">%s %s</a>\n",
      hexlink, (full?ppren[px]:pini), pname[px]);
  }
}

void HTMLEventsBlock(int r, int a, int b) {
  int x = allres[r][a][b]/100;
  int y = allres[r][a][b]%100;
  fprintf(of, " <div class=\"block  clearfix block_match_goals-wrapper\" id=\"g10w\">\n");
  fprintf(of, "  <h2>Evenimente</h2>\n\n");

  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_goals real-content clearfix \" id=\"g10\">\n\n");

  fprintf(of, "      <div class=\"fully-padded\">\n");
  fprintf(of, "  <table class=\"matches events\">\n");

  int cx = 0;
  int cy = 0;
  int pid = -1;
  int hsc;
  char sm[12];
  for (int e=0; e<nev; e++) {
    if (evm[e]>0 && evm[e]<=150) sprintf(sm, "%2d'", evm[e]); else sprintf(sm, " ");
    /* only for regular time */
    if (evm[e]>90 && evm[e]<120) sprintf(sm, "90+%d'", evm[e]%90);
    if (evp[e]>=0 && evp[e]<44) pid = roster[evp[e]]; else pid = -1;
    hsc = 2;
    if (evp[e]>= 0 && evp[e]<ROSTER_SIZE) hsc = 1;
    if (evp[e]>=22 && evp[e]<2*ROSTER_SIZE) hsc = 0;
    if (evt[e]==EV_OWNGOAL) {
      hsc = 1-hsc;
    }
    if (hsc==1) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div>");
      HTMLPlayerLink(pid, PL_FULL_NAME);
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/>%s</span>  &nbsp;</div>\n", evicon[evt[e]], sm);
      fprintf(of, "      </td>\n");
      if (evt[e]==EV_GOAL || evt[e]==EV_PKGOAL) {
        cx++;
        pgol[pid]++;
        cgol[pid]++;
      }
      if (evt[e]==EV_OWNGOAL) { cx++; }
      fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", cx, cy);
      fprintf(of, "      <td class=\"player player-b\">\n");
      fprintf(of, "        <div></div>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "    </tr>\n");
    }
    else if (hsc==0) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div></div>\n");
      fprintf(of, "      </td>\n");
      if (evt[e]==EV_GOAL || evt[e]==EV_PKGOAL) {
        pgol[pid]++;
        cgol[pid]++;
        cy++;
      }
      if (evt[e]==EV_OWNGOAL) { cy++; }
      fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", cx, cy);
      fprintf(of, "      <td class=\"player player-b\">\n");
      fprintf(of, "        <div>");
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/>%s</span>  ", evicon[evt[e]], sm);
      HTMLPlayerLink(pid, PL_FULL_NAME);
      fprintf(of, "</div>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "    </tr>\n");
    }
  }
/*
  for (int i=0; i<x; i++) {
    fprintf(of, "    <tr class=\"event    expanded\">\n");
    fprintf(of, "      <td class=\"player player-a\">\n");
    fprintf(of, "        <div>Marcator gazde #%d <span class=\"minute\">m'</span>  &nbsp;</div>\n", i+1);
    fprintf(of, "      </td>\n");
    fprintf(of, "      <td class=\"event-icon\"><div>%d - 0</div></td>\n", i+1);
    fprintf(of, "      <td class=\"player player-b\">\n");
    fprintf(of, "        <div></div>\n");
    fprintf(of, "      </td>\n");
    fprintf(of, "    </tr>\n");
  }

  for (int i=0; i<y; i++) {
    fprintf(of, "    <tr class=\"event    expanded\">\n");
    fprintf(of, "      <td class=\"player player-a\">\n");
    fprintf(of, "        <div></div>\n");
    fprintf(of, "      </td>\n");
    fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", x, i+1);
    fprintf(of, "      <td class=\"player player-b\">\n");
    fprintf(of, "        <div>Marcator oaspeþi #%d <span class=\"minute\">m'</span>  &nbsp;</div>\n", i+1);
    fprintf(of, "      </td>\n");
    fprintf(of, "    </tr>\n");
  }
*/

  fprintf(of, "  </table>\n\n");

  fprintf(of, "      </div>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void HTMLPlayerTH() {
  fprintf(of, "    <thead>\n");
  fprintf(of, "      <tr class=\"sub-head\">\n");
  fprintf(of, "        <th class=\"shirtnumber\">#</th>\n");
  fprintf(of, "        <th class=\"player\">Player</th>\n");
  fprintf(of, "        <th class=\"label\">Label</th>\n");
  fprintf(of, "        <th class=\"season_caps\">SeasonM</th>\n");
  fprintf(of, "        <th class=\"career_caps\">CareerM</th>\n");
  fprintf(of, "        <th class=\"label\">Label</th>\n");
  fprintf(of, "        <th class=\"season_goals\">SeasonG</th>\n");
  fprintf(of, "        <th class=\"career_goals\">CareerG</th>\n");
  fprintf(of, "        <th class=\"bookings\">Bookings</th>\n");
  fprintf(of, "      </tr>\n");
  fprintf(of, "    </thead>\n");
  fprintf(of, "    <tbody>\n");
}

void HTMLPlayerTR(int pn, int px, int m) {
    if (px < 0) {
      fprintf(of, "      <tr class=\"%s\"></tr>\n", (pn%2==1?"odd":"even"));
      return;
    }
    int sn = (pn<=ROSTER_SIZE? pn : pn-ROSTER_SIZE);
    char pini[12];
    GetInitial(px, pini);
    makeHexlink(pmnem[px]);
    fprintf(of, "      <tr class=\"%s\">\n", (sn%2==1?"odd":"even"));
    fprintf(of, "        <td class=\"shirtnumber\">%d</td>\n", sn);
    fprintf(of, "        <td class=\"player large-link\">\n");
    fprintf(of, "          <img src=\"../../../../thumbs/22/3/%s.png\"/>\n", pcty[px]);
    fprintf(of, "          <a href=\"../../jucatori/%s.html\">%s %s</a>\n", hexlink, pini, pname[px]);
    fprintf(of, "        </td>\n");
    fprintf(of, "        <td class=\"label\">M:</td>\n");
    fprintf(of, "        <td class=\"season_caps\">%d</td>\n", pmeci[px]);
    fprintf(of, "        <td class=\"career_caps\"> (%d)</td>\n", cmeci[px]);
    fprintf(of, "        <td class=\"label\">G:</td>\n");
    fprintf(of, "        <td class=\"season_goals\">%d</td>\n", pgol[px]);
    fprintf(of, "        <td class=\"career_goals\"> (%d)</td>\n", cgol[px]);
    fprintf(of, "        <td class=\"bookings\">");
    if (annotation[pn-1] > 0) {
      if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", annotation[pn-1]-m);
      fprintf(of,  "<img src=\"../../cr.png\"/>%d'", annotation[pn-1]);
    } else {
      if (sn <=11 && m<90) fprintf(of, "<img src=\"../../so.png\"/>%d'", m+1);
      else if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", 91-m);
    }
    fprintf(of, "        </td>\n");
    fprintf(of, "      </tr>\n\n");
}

void HTMLCoachTR(char *sc) {
  int cix = Ant.FindMnem(sc);
  fprintf(of, "      <tr class=\"even\">\n");
  fprintf(of, "        <td colspan=\"2\" style=\"padding: 0.5em;\">\n");
  if (cix>=0) {
    makeHexlink(Ant.P[cix].mnem);
    fprintf(of, "          <strong>Antrenor:</strong> <a href=\"../../antrenori/%s.html\">%s %s</a>\n",
        hexlink, Ant.P[cix].pren, Ant.P[cix].name);
  }
  else {
    fprintf(of, "          <strong>Antrenor:</strong> \n");
  }
  fprintf(of, "        </td>\n");
  fprintf(of, "      </tr>\n");
}

void HTMLLineupsBlock(int r, int a, int b) {
  fprintf(of, " <div class=\"block  clearfix block_match_lineups-wrapper\" id=\"l7w\">\n");
  fprintf(of, "  <h2>Formaþii</h2>\n\n");

  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_lineups real-content clearfix \" id=\"l7\">\n\n");

  int mid = MatchID(r, a, b);
  char spn[32], *spm;
  char pini[5];
  int m, px;

  fprintf(of, "<div class=\"container left\">\n");
  fprintf(of, "  <table class=\"playerstats lineups table\">\n");
  HTMLPlayerTH();

  for (int i=1; i<=11; i++) {
    strcpy(spn, db[mid][DB_T1+i]);
    strtok(spn, ":");
    px = FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, pname[px]);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    HTMLPlayerTR(i, px, m);
  }

  HTMLCoachTR(db[mid][DB_COACH1]);

  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "<div class=\"container right\">\n");

  fprintf(of, "  <table class=\"playerstats lineups table\">\n");
  HTMLPlayerTH();

  for (int i=1; i<=11; i++) {
    strcpy(spn, db[mid][DB_T2+i]);
    strtok(spn, ":");
    px = FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, pname[px]);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    HTMLPlayerTR(i+ROSTER_SIZE, px, m);
  }

  HTMLCoachTR(db[mid][DB_COACH2]);

  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void HTMLSubsBlock(int r, int a, int b) {
  fprintf(of, " <div class=\"block  clearfix block_match_substitutes-wrapper\" id=\"s8w\">\n");
  fprintf(of, "  <h2>Rezerve</h2>\n\n");

  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_substitutes real-content clearfix \" id=\"s8\">\n\n");

  int mid = MatchID(r, a, b);
  char spn[32], *spm;
  char pini[5];
  int m, px;

  fprintf(of, "<div class=\"container left\">\n");
  fprintf(of, "  <table class=\"playerstats lineups substitutions table\">\n");
  HTMLPlayerTH();

  for (int i=12; i<=22; i++) {
    strcpy(spn, db[mid][DB_T1+i]);

    strtok(spn, ":");
    px = FindMnem(spn);
    spm = strtok(NULL, ",");
    if (px<0) {
//      fprintf(of, "      <tr class=\"%s\"><td class=\"shirtnumber\">%d</td></tr>\n", (i%2==1?"odd":"even"), i);
      continue;
    }
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, pname[px]);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    HTMLPlayerTR(i, px, m);
  }
  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "<div class=\"container right\">\n");

  fprintf(of, "  <table class=\"playerstats lineups substitutions table\">\n");
  HTMLPlayerTH();

  for (int i=12; i<=22; i++) {
    strcpy(spn, db[mid][DB_T2+i]);
    strtok(spn, ":");
    px = FindMnem(spn);
    if (px<0) {
//      fprintf(of, "      <tr class=\"%s\"><td class=\"shirtnumber\">%d</td></tr>\n", (i%2==1?"odd":"even"), i);
      continue;
    }
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, pname[px]);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    HTMLPlayerTR(i+ROSTER_SIZE, px, m);
  }

  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void HTMLRef(char *sc) {
  int rix = Arb.FindMnem(sc);
  fprintf(of, "          <dt><strong>Arbitru central:  </strong></dt>");
  if (rix>=0) {
        makeHexlink(Arb.P[rix].mnem);
		fprintf(of, "<dd><a href=\"../../arbitri/%s.html\">%s %s (%s)</a></dd>",
          hexlink, Arb.P[rix].pren, Arb.P[rix].name, Arb.P[rix].pob);
  }
  fprintf(of, "</dt>\n");
}

void HTMLRefsBlock(int r, int a, int b) {
  int mid = MatchID(r, a, b);
  fprintf(of, "  <div class=\"block  clearfix block_match_additional_info-wrapper\" id=\"i6w\">\n");
  fprintf(of, "  <h2>Arbitri ºi observatori</h2>\n");
  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_additional_info real-content clearfix \" id=\"i6\">\n\n");

  fprintf(of, "      <div class=\"fully-padded clearfix\">\n");
  fprintf(of, "  <dl class=\"details\">\n");
	HTMLRef(db[mid][DB_REF]);
  fprintf(of, "    <dd></dd>\n\n");

  fprintf(of, "    <dt>Asistenþi:</dt>\n");
  fprintf(of, "    <dd>\n");
  fprintf(of, "    </dd>\n\n");

  fprintf(of, "    <dt>Oficial:</dt>\n");
  fprintf(of, "    <dd></dd>\n");
  fprintf(of, "  </dl>\n");
  fprintf(of, "</div>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void HTMLFooter() {
  fprintf(of, "</div>\n");
  fprintf(of, "</body>\n</html>");
}

void trim(char *s) {
  if (s[1]==' ' || s[1]=='.') { s[1] = s[2]; s[2] = s[3]; }
  if (s[2]==' ' || s[2]=='.') { s[2] = s[3]; }
  s[3] = 0;
}

void PrintReport(int r, int a, int b) {
  char rfilename[128];
  sprintf(rfilename, "html/reports/%d/%d-%d-%d.html", year, id[a], id[b], round[r][a][b]%1000);
  of = fopen(rfilename, "wt");
  if (of==NULL) { fprintf(stderr, "ERROR: Could not open file %s.\n", rfilename); return; }

  GetRoster(r, a, b);
  GetEvents(r, a, b);
  SortEvents();
  HTMLHeader(r, a, b);
  HTMLScoreBlock(r, a, b);
  HTMLInfoBlock(r, a, b);
  if (allres[r][a][b]>0 || nev>0) {
    HTMLEventsBlock(r, a, b);
  }
  HTMLLineupsBlock(r, a, b);
  HTMLSubsBlock(r, a, b);
  HTMLRefsBlock(r, a, b);
  HTMLFooter();
  fclose(of);
}

void ResTable(int r) {
  for (int h=0; h<rr; h++) {
  for (int i=0; i<n; i++) {
  for (int j=0; j<n; j++) {
    if (round[h][i][j]%1000 == r && allres[h][i][j]>=0) {
      AddResult(h, i, j);
    }
  }
  }
  }
  Ranking();
  HRanking();
  GRanking();
  ARanking();
  for (int h=0; h<rr; h++) {
  for (int i=0; i<n; i++) {
  for (int j=0; j<n; j++) {
    if (round[h][i][j]%1000 == r && allres[h][i][j]>=0) {
      PrintReport(h, i, j);
    }
  }
  }
  }
}

void PrintDate(int r) {
  ResTable(r);
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

  char sarg1[128];
  strcpy(sarg1, argv[1]);
  char *ystr = strtok(sarg1, ".");
  ystr = strtok(NULL, " ");
  if (ystr) year = atoi(ystr); else year = 0;
  SeasonName(year, ssn);
  bool full = true;
  winter = false;
  fd = -1;
  ld = -1;
  NG = 0;

  LoadAlltimeStats();
  if (!LoadFile(argv[1])) {
    printf("ERROR: file %s not found.\n", argv[1]);
    return -2;
  }

  decorate = 1;
  calendar = 1;
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

  CollectSched(1);
  LoadPlayers();
  LoadPlayerStats();
  LoadDB();
  LoadEvents();
  Ant.Load("coaches.dat");
  Arb.Load("referees.dat");
	Loc.Load("city.dat", "stadium.dat");

  if (fd < 0) {
    FindFirstRound();
    fd = first_r;
  }
  if (fd<0) fd = 415;
  if (ld<0) ld = fd+649;

  int nd = 0;
  for (int i=fd; i<=ld; i++) {
    int r = i%650;
    if (played[r]) { PrintDate(r); }
  }

  SaveAccumulatedStats();
  return 0;
}
