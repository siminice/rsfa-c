#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
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

#define MAX_LEVELS 12
#define MAX_RR      4
#define MAX_N      64
#define MAX_TEAMS 2000
#define MAX_COLS	 12
#define MAX_SEASONS	300
#define MAX_CUPG	20000

#define SPECIAL         50
#define LOSS_BOTH_0     50
#define LOSS_BOTH_9     59

#define MAX_NAMES   20000
#define DB_ROWS     400
#define DB_COLS     60
#define DB_CELL     20
#define EV_COLS     60

#define DB_HOME      0
#define DB_AWAY      1
#define DB_SCORE     2
#define DB_DATE      3
#define DB_COMP      4
#define DB_ROUND     5
#define DB_VENUE     6
#define DB_ATTEND    7
#define DB_WEATHER   8
#define DB_ROSTER1   9
#define DB_COACH1   31
#define DB_ROSTER2  32
#define DB_COACH2   54
#define DB_REF      55
#define DB_ASIST1   56
#define DB_ASIST2   57
#define DB_OBSERV   58
#define DB_T1        8
#define DB_T2       31

#define DOB_DD_MM_YYYY  0
#define DOB_YYYYMMDD    1

#define TD_CUP	8

const char *month[] = {"???", "jan", "feb", "mar", "apr", "may", "jun", 
                              "jul", "aug", "sep", "oct", "nov", "dec"};
const char* cupround[] = {"*", "F", "S", "Q", "O", "ª", "6", "7", "8", "9"};

//const char* fxcol[] = {"77FF77", "FFFF99", "FF5050", "9999FF", "FFCC99", "FF99FF", "CC33FF", "CCCCCC"};
const char* fxcol[] = {"E4FEE4", "FFFFCC", "FFE4E4", "F0F0FF", "9999FF", "FFCC99", "FF99FF", "CC33FF", "CCCCCC"};
const char *rgbn[]  = {"007700", "BB9922", "FF3333", "222277", "775522", "772277", "550077", "000000"};
const char *rgbd[]  = {"77FF77", "0077FF", "FF7777"};
const char *rgbs[]  = {"006600", "CC9900", "990000"};

int  NC, ND, NKM;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;
int  ****part;
int num_winter;
int *start_winter, *end_winter;
char **cupm;
int **cupr;
int **cupd;
int ncol, nkol;
int vs[MAX_SEASONS][MAX_COLS];
int vd[MAX_SEASONS][MAX_COLS];
int ha[MAX_SEASONS][MAX_COLS];
int pos1[MAX_SEASONS];
int pos2[MAX_SEASONS];

int  id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pts[MAX_N], pen[MAX_N], pdt[MAX_N];
int  round[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N];
int  rank[MAX_N];
char desc[MAX_N][32];
int  tbwin[MAX_N], tbdrw[MAX_N], tblos[MAX_N], tbgsc[MAX_N], tbgre[MAX_N], tbrk[MAX_N];

char inputfile[64], outputfile[64], filename[64];
FILE *of;
int details;

int n, ppv, tbr, rr;
int d1, d2, start, last;
int wh[MAX_LEVELS], dh[MAX_LEVELS], lh[MAX_LEVELS], sh[MAX_LEVELS], rh[MAX_LEVELS];
int wg[MAX_LEVELS], dg[MAX_LEVELS], lg[MAX_LEVELS], sg[MAX_LEVELS], rg[MAX_LEVELS];

//--- player stats
Catalog *Pl;
int NP;
int *psez, *pfy, *ply, *pmeci, *ptit, *pint, *prez, *pban, *pmin, *pgol, *ppen, *pgre, *pown, *pred, *prnk;
char ***cldb;
char ***cedb;
char ***kldb;
char ***kedb;

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

int NumericDOB(char *dob, int fmt) {
  char s[12];
  strcpy(s, dob);
  char *sd = strtok(s, "/.-");
  char *sm = strtok(NULL, "/.-");
  char *sy = strtok(NULL, "/.-");
  int xd = 0;
  if (sd!=NULL) xd = atoi(sd);
  int xm = 0;
  if (sm!=NULL) xm = atoi(sm);
  int xy = 0;
  if (sy!=NULL) xy = atoi(sy);
  if (xm>12 && xd<13) { /* inverseaza luna/ziua */
    int x = xm; xm = xd; xd = x;
  }
  if ((sd!=NULL && sm==NULL && sy==NULL) || (xd>0 && xm==0 && xy==0)) {
    xy = xd; xm = 0; xd = 0;
  }
  if (xy>0 && xy<100) xy = 1900+xy;
  return 10000*xy+100*xm+xd;
}

void CanonicDOB(char *dob, int fmt) {
  char s[12];
  strcpy(s, dob);
  char *sd = strtok(s, "/.-");
  char *sm = strtok(NULL, "/.-");
  char *sy = strtok(NULL, "/.-");
  int xd = 0;
  if (sd!=NULL) xd = atoi(sd);
  int xm = 0;
  if (sm!=NULL) xm = atoi(sm);
  int xy = 0;
  if (sy!=NULL) xy = atoi(sy);
  if (xm>12 && xd<13) { 
    int x = xm; xm = xd; xd = x;
  }
  if ((sd!=NULL && sm==NULL && sy==NULL) || (xd>0 && xm==0 && xy==0)) {
    xy = xd; xm = 0; xd = 0;
  }
  if (xy>0 && xy<100) xy = 1900+xy;
  if (fmt==DOB_DD_MM_YYYY) {
    sprintf(dob, "%02d/%02d/%04d", xd, xm, xy);
  }
  else if (fmt==DOB_YYYYMMDD) {
    sprintf(dob, "%04d%02d%02d", xy, xm, xd);
  }
}

void ResetTeamStats() {
  for (int d=0; d<MAX_LEVELS; d++) {
    wh[d] = dh[d] = lh[d] = sh[d] = rh[d] = 0;
    wg[d] = dg[d] = lg[d] = sg[d] = rg[d] = 0;
  }
}

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  int dummy, n, p, t, d, y;
  char *dv, *pl, *yr, *suf;

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
  if (!f) return 0;
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

  part = new int***[MAX_LEVELS];
  FY = new int[MAX_LEVELS];
  LY = new int[MAX_LEVELS];
  MAX = new int [MAX_LEVELS];

  for (int d=0; d<MAX_LEVELS; d++) {
    FY[d] = 2100;
    LY[d] = 1800;
    MAX[d] = 0;
  }

  ND = 0;
  DIR *dp;
  struct dirent *ep;
  dp = opendir("./"); 
  if (dp != NULL) {  
      while (ep = readdir (dp)) { 
        strcpy(s, ep->d_name);
        dv = strtok(s, ".");
        yr = strtok(NULL, ".");
        suf = strtok(NULL, ".");
        if (dv!=NULL && yr!=NULL && suf==NULL) {
          int l = strlen(dv);
          if (l==0 || l>3) continue;
          d = ((int) s[0]) - 97; 
          if (d<0 || d>=MAX_LEVELS) continue;
          if (d+1>ND) ND = d+1;
          if (l>1) p = atoi(dv+1); else p = 1;
          if (p<0 && p>12) continue;
          y = atoi(yr);
          if (d>=0 && y>1888 && y<2100) {
            if (y<FY[d]) FY[d] = y;
            if (y>LY[d]) LY[d] = y;
            if (p>MAX[d]) MAX[d] = p;
          }
        }
      }
      closedir(dp);  
      for (int d=0; d<ND; d++)
        printf("%c: %d - %d (max: %d)\n", (char) (d+65), FY[d], LY[d], MAX[d]);
  }
  else
   printf("ERROR: Couldn't open the directory.\n");  

  for (int d=0; d<ND; d++) {
    if (MAX[d]>0) {
      part[d] = new int**[LY[d]-FY[d]+1];
      for (int i=FY[d]; i<=LY[d]; i++) {
        part[d][i-FY[d]] = new int*[MAX[d]+1];
        for (int j=0; j<=MAX[d]; j++) {
          part[d][i-FY[d]][j] = new int[64];
        }
      }
    }
  }

// quick data
  for (int d=0; d<ND; d++) {
    sprintf(filename, "part.%c", (char) (d+97));
    f = fopen(filename, "rt");
    for (int y=0; y<=LY[d]-FY[d]; y++) {
      if (f==NULL) part[d][y][0][1] = 0;
      else {
        fscanf(f, "%d %d", &dummy, &n);
        part[d][y][0][0] = dummy;
        part[d][y][0][1] = n;
        for (int i=0; i<n; i++) {
          fscanf(f, "%d", &t); part[d][y][0][i+2] = t;
        }
        fgets(s, 200, f);
      }
    }
    if (f) fclose(f);
  }

  for (int d=0; d<ND; d++) {
    for (int i=1; i<=MAX[d]; i++) {
      sprintf(filename, "part.%c%d", (char) (d+97), i);
      f = fopen(filename, "rt");
      for (int y=0; y<=LY[d]-FY[d]; y++) {
        if (f==NULL) part[d][y][i][1] = 0;
        else {
          fscanf(f, "%d %d", &dummy, &n);
          part[d][y][i][0] = dummy;
          part[d][y][i][1] = n;
          for (int j=0; j<n; j++) {
            fscanf(f, "%d", &t); 
            part[d][y][i][j+2] = t;
          }
          fgets(s, 200, f);
        }
      }
      if (f) fclose(f);
    }
  }

  ResetTeamStats();
  return 1;
}

int Find(char* s) {
  int found = 0;
  int multi = 0;
  int j;
  int l = strlen(s);

  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  for (int i=0; i<l-1; i++)
    if ((s[i]==32 || s[i]=='.') && s[i+1]>96) s[i+1] -= 32;

  int i = 0;
  while (i < NC) {
    if (strcmp(mnem[i], s)==0) return i;
    i++;
  }

  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
    else i++;
  }
  if (found) return i;

  // try uppercase
  for (int j=0; j<l; j++)
    if (s[j]>='a' && s[j]<='z') s[j] -= 32;
  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
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
   do { fgets(name, 30, stdin); } while (!strlen(name));
   name[strlen(name)-1] = 0;
   res = Find(name);
  } while (res < 0);
  return res;
}

int LoadFile(char *filename) {
  FILE *f;
  int h, i, j, x, y, r, z;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[512], *tok[10];
  fgets(s, 100, f);
  sscanf(s, "%d %d %d", &n, &ppv, &tbr);
  rr = tbr/NUMORD + 1;

  /* clear all  data */
  for (h=0; h<rr; h++) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        round[h][i][j] = -1;
        res[h][i][j] = -1;
      }  
    }
  }
  for (i=0; i<n; i++) {
    fgets(s, 512, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
  }
  for (int h=0; h<rr; h++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[h][i][j] = r;
      res[h][i][j]   = z;
    }
    fscanf(f, "\n");
  }
  }
  fclose(f);
}

int FindId(int t) {
  for (int i=0; i<n; i++)
   if (id[i] == t) return i;
  return -1;
}

int in(int y, int d, int p, int t) {
  if (MAX[d]==0) return 0;
  if (y<FY[d] || y>LY[d]) return 0;
  int m = part[d][y-FY[d]][p][1];
  for (int j=2; j<m+2; j++)
    if (part[d][y-FY[d]][p][j] == t) return j-1;
  return -1;
}

void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);
  else sprintf(ss, "%d/%02d", y-1, y%100);
}

void ResetPlayerStats() {
  for (int i=0; i<NP; i++) {
     psez[i] = pfy[i] = ply[i] = pmeci[i] = ptit[i] = pint[i] = prez[i] = pban[i] = pmin[i] = pgol[i] = ppen[i] = pown[i] = pgre[i] = pred[i] = 0;
     prnk[i] = i;
  }
}

void LoadCatalog() {
  Pl = new Catalog();
  Pl->Load("players.dat");
  NP = Pl->Size();
  psez  = new int[MAX_NAMES];
  pfy   = new int[MAX_NAMES];
  ply   = new int[MAX_NAMES];
  pmeci = new int[MAX_NAMES];
  ptit  = new int[MAX_NAMES];
  pint  = new int[MAX_NAMES];
  prez  = new int[MAX_NAMES];
  pban  = new int[MAX_NAMES];
  pmin  = new int[MAX_NAMES];
  pgol  = new int[MAX_NAMES];
  ppen  = new int[MAX_NAMES];
  pown  = new int[MAX_NAMES];
  pgre  = new int[MAX_NAMES];
  pred  = new int[MAX_NAMES];
  prnk  = new int[MAX_NAMES];

  ResetPlayerStats();
}

void AllocateDB() {
  cldb = new char**[DB_ROWS];
  cedb = new char**[DB_ROWS];
  kldb = new char**[DB_ROWS];
  kedb = new char**[DB_ROWS];
  for (int r=0; r<DB_ROWS; r++) {
    cldb[r] = new char*[DB_COLS];
    kldb[r] = new char*[DB_COLS];
    cedb[r] = new char*[EV_COLS];
    kedb[r] = new char*[EV_COLS];
    for (int c=0; c<DB_COLS; c++) {
      cldb[r][c] = new char[DB_CELL];
      kldb[r][c] = new char[DB_CELL];
    }
    for (int c=0; c<EV_COLS; c++) {
      cedb[r][c] = new char[DB_CELL];
      kedb[r][c] = new char[DB_CELL];
    }
  }
}

void LoadLineups(int year) {
  for (int i=0; i<DB_ROWS; i++) for (int j=0; j<DB_COLS; j++) cldb[i][j][0] = 0;
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<n*(n-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(cldb[i][j], tk[j]);
      else strcpy(cldb[i][j], " ");
    }
  }
  fclose(f);
}

void LoadEvents(int year) {
  for (int i=0; i<DB_ROWS; i++) for (int j=0; j<EV_COLS; j++) cedb[i][j][0] = 0;
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "events-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<n*(n-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<EV_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<EV_COLS; j++) {
      if (tk[j]!=NULL) strcpy(cedb[i][j], tk[j]);
      else strcpy(cedb[i][j], " ");
    }
  }
  fclose(f);
}
void LoadCupLineups(int year) {
  for (int i=0; i<DB_ROWS; i++) for (int j=0; j<DB_COLS; j++) kldb[i][j][0] = 0;
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "cup-lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<n*(n-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(kldb[i][j], tk[j]);
      else strcpy(kldb[i][j], " ");
    }
  }
  fclose(f);
}

void LoadCupEvents(int year) {
  for (int i=0; i<DB_ROWS; i++) for (int j=0; j<EV_COLS; j++) kedb[i][j][0] = 0;
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "cup-events-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<n*(n-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<EV_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<EV_COLS; j++) {
      if (tk[j]!=NULL) strcpy(kedb[i][j], tk[j]);
      else strcpy(kedb[i][j], " ");
    }
  }
  fclose(f);
}

void AddPlayerStats(int a, int b, int y, char ***ldb, char ***edb, int r) {
  char spi[64], *sp, *sm;
  int min, pid;
  int h = atoi(ldb[r][DB_HOME]);
  int g = atoi(ldb[r][DB_AWAY]);
  if ((a==h && b==g) || (a==g && b==h)) {
        for (int j=DB_ROSTER1; j<DB_ROSTER2+22; j++) if (j!=DB_COACH1) {
            strcpy(spi, ldb[r][j]);
            sp = strtok(spi, ":");
            sm = strtok(NULL, ":");
            min = sm!=NULL ? atoi(sm) : 0;
            pid = Pl->binFindMnem(sp);
            if (pid>=0) {
              if (min>0) {
                pmin[pid] += min;
                pmeci[pid]++;
                if ((j<DB_ROSTER1+11) || (j>=DB_ROSTER2 && j<DB_ROSTER2+11)) {
                  ptit[pid]++;
                }
                if ((j>=DB_ROSTER1+11 && j<DB_ROSTER2) || (j>=DB_ROSTER2+11)) {
                  prez[pid]++;
                }
                if (pfy[pid] == 0) {
                  psez[pid] = 1;
                  pfy[pid] = ply[pid] = y;
                }
                else if (y > ply[pid]) {
                  psez[pid]++;
                  ply[pid] = y;
                }
              } else {
                pban[pid]++;
              }
            }
        }
        for (int j=0; j<DB_COLS; j++) {
            if (edb[r][j]==NULL || edb[r][j][0]==0x0 || edb[r][j][0]==' ' || edb[r][j][0]=='~') continue;
            strcpy(spi, edb[r][j]);
            char evt = edb[r][j][6];
            spi[6] = 0;
            pid = Pl->binFindMnem(sp);
            if (pid>=0) {
              if (evt==39) {
                pgol[pid]++;
              } else if (evt==34) {
                pgol[pid]++; ppen[pid]++;
              } else if (evt==96) {
                pown[pid]++;
              } else if (evt==33) {
                pred[pid]++;
              }
            }
        }
  }
}

void SortPlayerStats() {
  int sorted, last;
  for (int i=0; i<NP; i++) prnk[i] = i;
  last = NP-1;
  do {
    sorted = 1;
    for (int i=0; i<NP-1; i++) {
      if (pmin[prnk[i+1]] > pmin[prnk[i]]) {
        sorted = 0;
        int aux = prnk[i]; prnk[i] = prnk[i+1]; prnk[i+1] = aux;
      }
    }
  } while (sorted==0);
}

void LoadCup() {
  NKM = 0;
  char s[128];
  int score[8];
  FILE *f = fopen("cuparchive.dat", "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: cannot find file 'cuparchive.dat'\n");
    return;
  }
  cupm = new char*[MAX_CUPG];

  while (!feof(f)) {
    fgets(s, 128, f);
    if (strlen(s) > 10) {
      cupm[NKM] = new char[strlen(s)+1];
      strcpy(cupm[NKM], s);
      NKM++;
    }
    s[0] = 0;
  }
  fclose(f);
}

int CupData(int a, int b) {
  int score[8];
  int row, prevy;

  nkol = 0;
  int na = LY[0]-FY[0]+1;
  cupr = new int*[na];
  cupd = new int*[na];
  for (int y=0; y<na; y++) {
    cupr[y] = new int[4];
    cupd[y] = new int[4];
    for (int t=0; t<4; t++) {
      cupr[y][t] = -1;
      cupd[y][t] = 0;
    }
  }

  prevy = -1;
  row = 0;
  for (int i=0; i<NKM; i++) {
      int year     = 75*(cupm[i][0]-48) + (cupm[i][1]-48) + 1870;
      int mon      = cupm[i][2]-48;
      int day      = cupm[i][3]-48;
      int zi       = 50*((mon-1)%12+1) + day;
      row = year > prevy? 0 : row+1;
      prevy = year;
      int home     = 75*((int)(cupm[i][7]-48)) + cupm[i][8] - 48;
      int guest    = 75*((int)(cupm[i][9]-48)) + cupm[i][10] - 48;
      if (a!=home && a!=guest) continue;
      if (b!=home && b!=guest) continue;

      int ssn      = year;
      int round    = (int) (cupm[i][6] - 48);
      int len  = strlen((char *)cupm[i]);
      int ns   = ssn - FY[0];

      for (int j=0; j<8; j++)
        if (j+11 < len) score[j] = cupm[i][j+11] - 48;
        else score[j] = -1;
      int gdiff = score[0] - score[1];
      int c = 0;
      while (c<4 && cupr[ns][c]>=0) c++;
      if (c>=4) {
        fprintf(stderr, "ERROR: too many cup games between %s and %s in %d.\n", NickOf(L, a, year), NickOf(L, b, year), year);
        continue;
      }

      if (c+1>nkol) nkol = c+1;

      if (a==home) {
        cupr[ns][c] = 1000*round+100*score[0]+score[1];
        cupd[ns][c] = zi;
        if (gdiff>0) wh[10]++;
        else if (gdiff==0) dh[10]++;
        else lh[10]++;
        sh[10] += score[0];
        rh[10] += score[1];
        ha[ns][ncol+c] = 1;
      }
      else {
        cupr[ns][c] = 1000*round+100*score[1]+score[0];
        cupd[ns][c] = zi;
        if (gdiff>0) lg[10]++;
        else if (gdiff==0) dg[10]++;
        else wg[10]++;
        rg[10] += score[0];
        sg[10] += score[1];
        ha[ns][ncol+c] = 2;
      }
      if (details>0) {
        LoadCupLineups(year);
        LoadCupEvents(year);
        AddPlayerStats(a, b, year, kldb, kedb, row);
      }
  }
  return 1;
}

void getResults(int a, int b, int y, int d1, int d2) {
  int t;
  char filename[15], strdiv[15], strsez[64], ssn[64];

  int d = d1/100;
  int p = d1%100; 
  int ns = y-FY[0];

  if (d1==d2 && d1<400) {
    if (p==0) sprintf(filename, "%c.%d", d+97, y);
    else sprintf(filename, "%c%d.%d", d+97, p, y);
    LoadFile(filename);
    int t1 = FindId(a);
    int t2 = FindId(b);

    if (details>0 && d1==0) {

      int r1 = (n-1)*t1 + t2 - (t1<t2);
      int r2 = (n-1)*t2 + t1 - (t2<t1);
      LoadLineups(y);
      LoadEvents(y);
      AddPlayerStats(a, b, y, cldb, cedb, r1);
      AddPlayerStats(a, b, y, cldb, cedb, r2);
    }

    for (int h=0; h<rr; h++) {

      int x1 = res[h][t1][t2]/100;
      int y1 = res[h][t1][t2]%100;
      int x2 = res[h][t2][t1]/100;
      int y2 = res[h][t2][t1]%100;

      if (res[h][t1][t2]<0) {
      }
      else if (y1>=SPECIAL) {
        if (y1>=LOSS_BOTH_0 && y1<=LOSS_BOTH_9) {
          //
        }
      }
      else {
        sh[d] += x1; rh[d] += y1;
        if (x1==y1) { dh[d]++; }
        else if (x1<y1) { lh[d]++; }
        else { wh[d]++; }
        vs[ns][2*h] = 100*x1+y1;
        vd[ns][2*h] = round[h][t1][t2]%1000;
        ha[ns][2*h] = 1;
      }

      if (res[h][t2][t1]<0) {
      }
      else if (y2>=SPECIAL) {
        if (y2>=LOSS_BOTH_0 && y2<=LOSS_BOTH_9) {
        //
        }
      }
      else {
        sg[d] += y2; rg[d] += x2;
        if (x2==y2) { dg[d]++; }
        else if (x2>y2) { lg[d]++; }
        else { wg[d]++; }
        vs[ns][2*h+1] = 100*y2+x2;
        vd[ns][2*h+1] = round[h][t2][t1]%1000;
        ha[ns][2*h+1] = 2;
      }
    } // for h

    if (2*rr>ncol) ncol=2*rr;
  }
}

void StatLine(int a, int b, int y, int d1, int d2, int rka, int rkb) {
  int t, ga, gb;
  char strdiv[15], strsez[64], strlink[256], ssn[64];

  const char *cn1 = rgbn[d1/100]; // rgbn[d1/100];
  const char *cn2 = rgbn[d2/100]; // rgbn[d2/100];
  const char *cd1 = fxcol[d1/100]; // rgbd[d1/100];
  const char *cd2 = fxcol[d2/100]; // rgbd[d2/100];
  SeasonName(y, ssn);
  sprintf(strlink, "%s", ssn);
  if (d1==d2 && d1<400) {
    char cd = ((char)d1/100)+97;
    if (d1%100>0) {
      sprintf(strlink, "<A HREF=\"../%c%d.%d-r1.html\"><FONT COLOR=\"#000000\">%s</FONT></A>", cd, d1%100, y, ssn);
    } else {
      sprintf(strlink, "<A HREF=\"../%c.%d-r1.html\"><FONT COLOR=\"#000000\">%s</FONT></A>", cd, y, ssn);
    }
  }
  fprintf(of, "<TR");
  if (y%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
  fprintf(of,"><TD>%s</TD>", strlink);
  if (d1>=400 || rka<0) {
    fprintf(of, "<TD COLSPAN=\"4\"></TD>\n");
  }
  else {
    fprintf(of, "<TD BGCOLOR=\"%s\">%c", cd1, (char)(d1/100+65));
    if (d1%100>0) fprintf(of, "%d", d1%100);
    fprintf(of, "</TD>");
    fprintf(of, "<TD BGCOLOR=\"%s\">#%d</TD>", cd1, rka);
    if (d1==d2 && d1<400) {
     fprintf(of, "<TD BGCOLOR=\"%s\">%s</TD>", cd1, NameOf(L, a, y));
     fprintf(of, "<TD>-</TD>");
    }
    else
     fprintf(of, "<TD BGCOLOR=\"%s\"></TD><TD/>", cd1);
  }

  if (d2>=400 || rkb<0) {
    fprintf(of, "<TD COLSPAN=\"4\"></TD>\n");
  }
  else {
    fprintf(of, "<TD BGCOLOR=\"%s\">%c", cd2, (char)(d2/100+65));
    if (d2%100>0) fprintf(of, "%d", d2%100);
    fprintf(of, "</TD>");
    fprintf(of, "<TD BGCOLOR=\"%s\">#%d</TD>", cd2, rkb);
    if (d1==d2 && d1<400) {
      fprintf(of, "<TD BGCOLOR=\"%s\">%s</TD>", cd2, NameOf(L, b, y));
      fprintf(of, "<TD>-</TD>");
    }
    else
     fprintf(of, "<TD BGCOLOR=\"%s\"></TD><TD/>", cd2);
  }

  int d = d1/100;
  int p = d1%100;
  int ns = y-FY[0];
  int col1 = 0;
  int col2 = 0;

  if (d1==d2 && d1<400) {
    for (int c=0; c<ncol; c++) {
      ga = vs[ns][c]/100;
      gb = vs[ns][c]%100;
      col1 = 0;
      col2 = 0;
      if (ga==gb) { col1 = 1; }
      else if (ga<gb) { col1 = 2; }

      if (vs[ns][c]<0) {
        fprintf(of, "<TD/>");
      }
      else if (gb>=SPECIAL) {
        fprintf(of, "<TD/>");
        if (gb>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
          //
        }
        fprintf(of, "<TD/>");
      }
      else {
        fprintf(of, "<TD><A HREF=\"../reports/%d/%d-%d-%d.html\"><FONT COLOR=\"%s\"><B>%d-%d</B></FONT></A></TD>", 
           y, (ha[ns][c]==1?a:b), (ha[ns][c]==1?b:a), vd[ns][c], rgbs[col1], ga, gb);
      }
    }
  }
  else {
    if (ncol>0) fprintf(of, "<TD COLSPAN=\"%d\"></TD>", ncol);
  }

    for (int c=0; c<nkol; c++) {
      int kr = cupr[ns][c]/1000;
      int ks = cupr[ns][c]%1000;
      ga = ks/100;
      gb = ks%100;

      col2 = 0;
      if (cupr[ns][c]<0) {
        fprintf(of, "<TD/>");
      }
      else {
        if (ga==gb) { col2 = 1; }
        else if (ga<gb) { col2 = 2; }
        fprintf(of, "<TD ALIGN=\"right\">");
        if (c==0) {
          fprintf(of, "%s: ", cupround[kr]);
				}
        fprintf(of, "<A HREF=\"../reports/%d/c%d-%d-%d.html\"><FONT COLOR=\"%s\">%d-%d</FONT></A></TD>",
					y, (ha[ns][ncol+c]==1?a:b), (ha[ns][ncol+c]==1?b:a), cupd[ns][c], rgbs[col2], ga, gb);
      }
    }


  fprintf(of, "</TR>\n");
}

void DatatablesHeader() {
    fprintf(of, "<link rel=\"stylesheet\" type=\"text/css\" href=\"//cdn.datatables.net/1.10.10/css/jquery.dataTables.css\">");
    fprintf(of, "<script type=\"text/javascript\" language=\"javascript\" src=\"//code.jquery.com/jquery-1.11.3.min.js\"></script>");
    fprintf(of, "<script type=\"text/javascript\" charset=\"utf8\" src=\"//cdn.datatables.net/1.10.10/js/jquery.dataTables.js\"></script>");
    fprintf(of, "<script type=\"text/javascript\" class=\"init\">");
    fprintf(of, "$(document).ready( function () {");
    fprintf(of, "    $('#all').DataTable( {");
    fprintf(of, "      pageLength : 100,");
    fprintf(of, "    } );");
    fprintf(of, "} );");
    fprintf(of, "</script>");
}

void HTMLLineupsHeader() {
  fprintf(of, "<script src=\"../sorttable.js\"></script>\n");
//  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<TABLE id=\"all\" class=\"display\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>Data naºterii</TH>");
  fprintf(of, "<TH>Naþ.</TH>");
  fprintf(of, "<TH>Sez.</TH>");
  fprintf(of, "<TH>Primul</TH>");
  fprintf(of, "<TH>Ultimul</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Minute</TH>");
  fprintf(of, "<TH>Titular</TH>");
  fprintf(of, "<TH>Rezervã</TH>");
  fprintf(of, "<TH>Goluri</TH>");
  fprintf(of, "<TH>Pen</TH>");
  fprintf(of, "<TH>Auto</TH>");
  fprintf(of, "<TH>Gol/-</TH>");
  fprintf(of, "<TH>Elim</TH>");
  fprintf(of, "</TR></THEAD>\n");
}

void HTMLPlayerStatsTable() {
  fprintf(of, "<H3>Statistici jucãtori:</H3>\n");
  HTMLLineupsHeader();
  SortPlayerStats();
  for (int i=0; i<NP; i++) {
    int x = prnk[i];
    if (pmeci[x]==0) continue;
    fprintf(of, "<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD align=\"right\">%d.</TD>", i+1);
    fprintf(of, "<TD align=\"left\">%s</TD>", Pl->P[x].pren);
    makeHexlink(Pl->P[x].mnem);
    fprintf(of, "<TD align=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"../jucatori/%s.html\">%s</A></TD>",
        Pl->P[x].name, Pl->P[x].pren, hexlink, Pl->P[x].name);
    CanonicDOB(Pl->P[x].dob, DOB_DD_MM_YYYY);
    fprintf(of, "<TD align=\"right\" sorttable_customkey=\"%d\">%s</TD>", NumericDOB(Pl->P[x].dob, DOB_YYYYMMDD), Pl->P[x].dob);
    fprintf(of, "<TD align=\"center\">%s<IMG SRC=\"../../../thumbs/22/3/%s.png\"></IMG></TD>", Pl->P[x].cty, Pl->P[x].cty);
    fprintf(of, "<TD align=\"right\">%d</TD>", psez[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pfy[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ply[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pmeci[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pmin[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ptit[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", prez[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pgol[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ppen[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pown[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pgre[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pred[x]);
    fprintf(of, "</TR>\n");
  }
  fprintf(of, "</TABLE>");
}

void VsTable(int a, int b) {
  int d1, d2, rk1, rk2, rka, rkb, sn;
  char s[200], ssn[64];

//  if (a>b) { int aux = a; a=b; b=aux; }
  sprintf(outputfile, "html/vs-%04d/vs-%d-%d.html", a, a, b);
  of = fopen(outputfile, "wt");
  if (!of) return;

  fprintf(of, "<HTML>\n<TITLE>Rezultate %s vs %s</TITLE>\n", NameOf(L, a, 3000), NameOf(L, b, 3000));
  fprintf(of, "<HEAD>\n<link href=\"../css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  DatatablesHeader();
  fprintf(of, "</HEAD>\n<BODY>\n");
  fprintf(of, "<H2>Rezultate %s vs %s</H2>\n", NameOf(L, a, 3000), NameOf(L, b, 3000));

  int ng[MAX_LEVELS];
  for (int d=0; d<MAX_LEVELS; d++) ng[d] = 0;
  for (int j=0; j<MAX_LEVELS; j++) ng[j] += wh[j]+dh[j]+lh[j]+wg[j]+dg[j]+lg[j];

  int ngk = 0; for (int d=0; d<ND; d++) ngk = ngk + ng[d];
  if (ngk>0) {
    fprintf(of, "<TABLE WIDTH=\"50%%\" BORDER=\"1\" CELLPADDING=\"2\" RULES=\"rows\">\n");
    fprintf(of, "<COLGROUP><COL SPAN=\"1\"></COLGROUP>");
    fprintf(of, "<COLGROUP><COL SPAN=\"9\"></COLGROUP>");
    fprintf(of, "<TR BGCOLOR=\"DDDDDD\">");
    fprintf(of, "<TH ALIGN=\"left\"></TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"7%%\">J</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"9%%\">V</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"7%%\">E</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"7%%\">Î</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"12%%\">gm</TH>");
    fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\">-</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"5%%\">gp</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"10%%\">P</TH>");
    fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"12%%\">%%</TH>");
    fprintf(of, "</TR>\n");
  
      int wink, drwk, losk, gsck, grek, pctk;

      wink = 0; for (int d=0; d<ND; d++) wink = wink + wh[d]+wg[d];
      drwk = 0; for (int d=0; d<ND; d++) drwk = drwk + dh[d]+dg[d];
      losk = 0; for (int d=0; d<ND; d++) losk = losk + lh[d]+lg[d];
      gsck = 0; for (int d=0; d<ND; d++) gsck = gsck + sh[d]+sg[d];
      grek = 0; for (int d=0; d<ND; d++) grek = grek + rh[d]+rg[d];

      fprintf(of, "<TBODY><TR BGCOLOR=\"EEEEEE\"><TD COLSPAN=\"10\"><B><FONT COLOR=\"000077\">Campionat<FONT></B></TD></TR></TBODY>\n");

      fprintf(of, "<TBODY><TR>");
      fprintf(of, "<TD ALIGN=\"left\">Total</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR>\n");
     
      wink = 0; for (int d=0; d<ND; d++) wink = wink + wh[d];
      drwk = 0; for (int d=0; d<ND; d++) drwk = drwk + dh[d];
      losk = 0; for (int d=0; d<ND; d++) losk = losk + lh[d];
      ngk  = wink+drwk+losk;
      gsck = 0; for (int d=0; d<ND; d++) gsck = gsck + sh[d];
      grek = 0; for (int d=0; d<ND; d++) grek = grek + rh[d];
      pctk = (ngk>0? (int) ((100.0*wink+50*drwk) / (ngk)) : 0);

      fprintf(of, "<TR>");
      fprintf(of, "<TD ALIGN=\"left\">Acasã</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR>\n");

      wink = 0; for (int d=0; d<ND; d++) wink = wink + wg[d];
      drwk = 0; for (int d=0; d<ND; d++) drwk = drwk + dg[d];
      losk = 0; for (int d=0; d<ND; d++) losk = losk + lg[d];
      ngk  = wink+drwk+losk;
      gsck = 0; for (int d=0; d<ND; d++) gsck = gsck + sg[d];
      grek = 0; for (int d=0; d<ND; d++) grek = grek + rg[d];
      pctk = (ngk>0? (int) ((100.0*wink+50*drwk) / (ngk)) : 0);

      fprintf(of, "<TR>");
      fprintf(of, "<TD ALIGN=\"left\">Deplasare</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR></TBODY>\n");

  for (int j=0; j<MAX_LEVELS; j++) {
    if (ng[j]>0) {

      ngk  = ng[j];
      wink = wh[j]+wg[j];
      drwk = dh[j]+dg[j];
      losk = lh[j]+lg[j];
      gsck = sh[j]+sg[j];
      grek = rh[j]+rg[j];
      pctk = (ngk>0? (int) ((100.0*wink+50*drwk) / (ngk)) : 0);

      if (j<ND)
        fprintf(of, "<TBODY><TR BGCOLOR=\"EEEEEE\"><TD COLSPAN=\"10\"><B><FONT COLOR=\"000077\">Divizia %c</FONT></B></TD></TR></TBODY>\n", (char)(j+65));
      else
        fprintf(of, "<TBODY><TR BGCOLOR=\"EEEEEE\"><TD COLSPAN=\"10\"><B><FONT COLOR=\"000077\">Cupã</FONT></B></TD></TR></TBODY>\n");

      fprintf(of, "<TBODY><TR>");
      fprintf(of, "<TD ALIGN=\"left\">Total</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR>\n");
     
      wink = wh[j];
      drwk = dh[j];
      losk = lh[j];
      ngk  = wink+drwk+losk;
      gsck = sh[j];
      grek = rh[j];
      pctk = (ngk>0? (int) ((100.0*wink+50*drwk) / (ngk)) : 0);
  
      if (j<ND) {
      fprintf(of, "<TR>");
      fprintf(of, "<TD ALIGN=\"left\">Acasã</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR>\n");

      wink = wg[j];
      drwk = dg[j];
      losk = lg[j];
      ngk  = wink+drwk+losk;
      gsck = sg[j];
      grek = rg[j];
      pctk = (ngk>0? (int) ((100.0*wink+50*drwk) / (ngk)) : 0);
  
      fprintf(of, "<TR>");
      fprintf(of, "<TD ALIGN=\"left\">Deplasare</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR></TBODY>\n");
      } // j <ND
      }
    }
  }
  fprintf(of, "</TABLE>\n");

  fprintf(of, "<BR><BR>\n");
  fprintf(of, "<TABLE WIDTH=\"75%%\" BORDER=\"1\" RULES=\"rows\">\n");
  fprintf(of, "<TR>");
  fprintf(of, "<COL WIDTH=\"4%%\" ALIGN=\"right\">");
  fprintf(of, "<COL WIDTH=\"1%%\" ALIGN=\"center\">");
  fprintf(of, "<COL WIDTH=\"1%%\" ALIGN=\"right\">");
  fprintf(of, "<COL ALIGN=\"left\">");
  fprintf(of, "<COL WIDTH=\"1%%\">");
  fprintf(of, "<COL WIDTH=\"1%%\" ALIGN=\"center\">");
  fprintf(of, "<COL WIDTH=\"1%%\" ALIGN=\"right\">");
  fprintf(of, "<COL ALIGN=\"left\">");
  fprintf(of, "<COL WIDTH=\"1%%\">");
  for (int c=0; c<ncol; c++)
    fprintf(of, "<COL WIDTH=\"5%%\" ALIGN=\"center\">");
  for (int c=0; c<nkol; c++)
    fprintf(of, "<COL WIDTH=\"7%%\" ALIGN=\"center\">");
  fprintf(of, "</TR>\n");

  fprintf(of, "<TR>");
  fprintf(of, "<TD ALIGN=\"center\">Sezon</TD>");
  fprintf(of, "<TD ALIGN=\"center\" COLSPAN=\"%d\" ALIGN=\"center\">Campionat</TD>", 8+ncol);
  if (nkol>0) fprintf(of, "<TD ALIGN=\"center\" COLSPAN=\"%d\" ALIGN=\"center\">Cupã</TD>", nkol);
  fprintf(of, "</TR>\n");

  start = 0; last = 3000;

  for (int y = fd; y<=ld; y++) {
    sn = y-FY[0];
    if (pos1[sn]<40000 || pos2[sn]<40000) {
       if (y>last+1) {
         for (int j=last+1; j<y; j++) {
            SeasonName(j, ssn);
            fprintf(of, "<TR><TD>%s</TD><TD COLSPAN=\"%d\"></TD></TR>", ssn, 8+ncol+nkol);
         }
       }
       start = 1;
       d1  = pos1[sn]/100;
       d2  = pos2[sn]/100;
       rka = pos1[sn]%100;
       rkb = pos2[sn]%100;
       StatLine(a, b, y, d1, d2, rka, rkb);
       last = y;
    }
    else if (cupr[sn][0]>=0) {
       StatLine(a, b, y, 1000, 0, 0, 0);
    }
  }
  fprintf(of, "</TBODY>\n</TABLE>\n");

  if (details>0) {
    HTMLPlayerStatsTable();
  }

  fprintf(of, "</BODY>\n</HTML>");

  fclose(of);
}



void Vs(int a, int b) {
  int d1, d2, rk1, rk2, rka, rkb;

  /* clear history */
  ncol = 0;
  for (int y=0; y<MAX_SEASONS; y++) {
    pos1[y] = 40000; pos2[y] = 40000;
    for (int c=0; c<MAX_COLS; c++) vs[y][c] = vd[y][c] = ha[y][c] = -1;
  }

  start = 0; last = 3000;
  for (int y = fd; y<=ld; y++) {
    int sn = y-FY[0];
    if (part[0][sn][0][1] == 0 && part[0][sn][1][1] == 0) continue;

    rka = -1;
    rkb = -1;
    d1 = 400;
    d2 = 400;
    for (int d=ND-1; d>=0; d--) {
      for (int p = MAX[d]+1; p>=0; p--) {
        rk1 = in(y, d, p%(MAX[d]+1), a);
        rk2 = in(y, d, p%(MAX[d]+1), b);
        if (rk1>0) {
          rka = rk1;
          d1 = 100*d+p%(MAX[d]+1);
          pos1[sn]=100*d1+rka;
        }
        if (rk2>0) {
          rkb = rk2;
          d2 = 100*d+p%(MAX[d]+1);
          pos2[sn]=100*d2+rkb;
        }
      }
    }
    if (rka>0 || rkb>0) {
       getResults(a, b, y, d1, d2);
    }
  }
  CupData(a,b);

  VsTable(a,b);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  int a, b;
  details = 0;
  int m = -1;

  a = -1;
  b = -1;
  for (int k=1; k<argc; k++) {
    if (strstr(argv[k], "-d")==argv[k]) details = 1;
    if (strcmp(argv[k], "-fd")==0) {
      if (k+1<argc) fd = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-ld")==0) {
      if (k+1<argc) ld = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-t1")==0) {
      if (k+1<argc) a = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-t2")==0) {
      if (k+1<argc) b = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-max")==0) {
      if (k+1<argc) m = atoi(argv[k+1]);
    }
    if (strstr(argv[k], "+")!=NULL) {
      details = 1;
    }
  }

  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }
  LoadCup();
  if (details>0) {
     LoadCatalog();
     AllocateDB();
  }

  if (m<0) m=NC;
  if (fd < FY[0]) fd = FY[0];
  if (ld < fd || ld > LY[0]) ld = LY[0];

  while (a<0) {
    a = GetUnique("Team #1: ");
  }
  printf("%d.%s\n", a, NameOf(L, a, 3000));
  if (b<0) {
    for (int i=0; i<m; i++)  {
//      printf("%3d.%s\n", i+1, NameOf(L, i, 3000));
      printf(".");
      if (a!=i) Vs(a,i);
      ResetTeamStats();
      ResetPlayerStats();
    }
  }
  else {
    Vs(a,b);
  }
  printf("\n");
  return 0;
}
