#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int verbosity;
int tm, pl, cr;
int FY, LY;
int ECFY, ECLY;
int KFY, KLY;
int NFY, NLY;

char **club;
char **mnem;
int  NC;
int year;
int *dva, *ecc, *echn;
bool winter;
int num_winter;
int *start_winter, *end_winter;

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

  f = fopen("teams.dat", "rt");
  if (f==NULL) return 0;
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  dva = new int[NC];
  ecc = new int[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[32];
    memmove(mnem[i], s, 15); mnem[i][15] = 0;
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
    dva[i] = ecc[i] = 0;
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
//      fprintf(stderr, "Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
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

/* *************************************** */

#define MAX_NAMES	20000
#define MAX_DB		800
#define MAX_EDB		300
#define MAX_KDB		750
#define MAX_NDB		100

#define TD_MNEM		 0
#define TD_PRENUME	 1
#define TD_NUME		 2
#define TD_DOB		 3
#define TD_NAT		 4
#define TD_CLUBID	 5
#define TD_NR		 6
#define TD_POST		 7
#define TD_MECIURI	 8
#define TD_TITULAR	 9
#define TD_REZERVA	10
#define TD_GOLURI	11
#define TD_SEZOANE	12
#define TD_FY		13
#define TD_LY		14
#define TD_NUM		TD_GOLURI+1
#define TD_XTRA		TD_LY+1

#define CAT_MNEM    0
#define CAT_NAME    1
#define CAT_PREN    2
#define CAT_DOB     3
#define CAT_NAT     4
#define CAT_CLUB    5
#define CAT_NR      6
#define CAT_POST    7
#define CAT_CAPS    8
#define CAT_START   9
#define CAT_SUB    10
#define CAT_GOALS  11
#define CAT_MIN    12
#define CAT_INTEG  13
#define CAT_BENCH  14
#define CAT_GREC   15
#define CAT_OWN    16
#define CAT_PK     17
#define CAT_PKMISS 18
#define CAT_YELLOW 19
#define CAT_RED    20
#define CAT_COLS   CAT_RED+1

#define MAX_PSZ		50
#define MAX_PTDLEN	32

#define PTD_SEZON	0
#define PTD_ECHIPA	1
#define PTD_NR		2
#define PTD_POST	3
#define PTD_DIVM	4
#define PTD_DIVT	5
#define PTD_DIVR	6
#define PTD_DIVG	7
#define PTD_CUPM	8
#define PTD_CUPT	9
#define PTD_CUPR	10
#define PTD_CUPG	11
#define PTD_EURM	12
#define PTD_EURT	13
#define PTD_EURR	14
#define PTD_EURG	15
#define PTD_NATM	2
#define PTD_NATT	3
#define PTD_NATR	4
#define PTD_NATG	5
#define NUM_PTD		PTD_EURG+1

#define CR_NONE 0
#define CR_M	1
#define	CR_G	2
#define CR_T	3
#define CR_R	4
#define CR_S	5

char ****db;
char ****edb;
char ****kdb;
char ****ndb;
char ***pdb;
int echid[MAX_PSZ];
int sezid[MAX_PSZ];

const char *oldECmnem[] = {"SC", "CCE", "CWC", "UEFA"};
const char *intECmnem[] = {"SC", "LC", "CWC", "UEFA"};
const char *newECmnem[] = {"SC", "LC", " ", "EL"};

/* *************************************** */

int NP;
char **pmnem;
char **pname;
char **ppren;
char **pdob;
char **pcty;
char **ppob;
char **pjud;

int *psez;
int *pfy;
int *ply;
int *dmeci, *dtit, *drez, *dgol;
int *kmeci, *ktit, *krez, *kgol;
int *emeci, *etit, *erez, *egol;
int *nmeci, *ntit, *nrez, *ngol;
int *tmeci, *ttit, *trez, *tgol;
int *rank;

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

int borna[256];

char ofilename[128];
FILE *of;

/* *************************************** */

int FindMnem(char *mnem) {
  int c = ((unsigned char)mnem[0]);
  for (int i=borna[c]; i<borna[c+1]; i++)
    if (strcmp(pmnem[i], mnem)==0) return i;
  return -1;
}

const char* ECname(int c, int year) {
  if (c<0 || c>3) return " ";
  if (year<1992) return oldECmnem[c];
  else if (year<2010) return intECmnem[c];
  else return newECmnem[c];
}

void LoadPlayers() {
  int c;

  for (c=0; c<257; c++) borna[c] = -1;
  NP = 0;
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

  psez = new int[MAX_NAMES];
  pfy = new int[MAX_NAMES];
  ply = new int[MAX_NAMES];
  dmeci = new int[MAX_NAMES];
  dtit = new int[MAX_NAMES];
  drez = new int[MAX_NAMES];
  dgol = new int[MAX_NAMES];
  kmeci = new int[MAX_NAMES];
  ktit = new int[MAX_NAMES];
  krez = new int[MAX_NAMES];
  kgol = new int[MAX_NAMES];
  emeci = new int[MAX_NAMES];
  etit = new int[MAX_NAMES];
  erez = new int[MAX_NAMES];
  egol = new int[MAX_NAMES];
  nmeci = new int[MAX_NAMES];
  ntit = new int[MAX_NAMES];
  nrez = new int[MAX_NAMES];
  ngol = new int[MAX_NAMES];
  tmeci = new int[MAX_NAMES];
  ttit = new int[MAX_NAMES];
  trez = new int[MAX_NAMES];
  tgol = new int[MAX_NAMES];

  rank = new int[MAX_NAMES];

  for (int i=0; i<NP; i++) {
    fgets(s, 100, f);
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

     if (i==0) {
       c = ((unsigned char) pmnem[i][0]);
       borna[c] = i;
     }
     else if (i>0 && pmnem[i-1][0]!=pmnem[i][0]) {
       c = ((unsigned char) pmnem[i][0]);
       borna[c] = i;
     }
     psez[i] = pfy[i] = ply[i] = 0;
     rank[i] = i;
  }
  c = ((unsigned char)pmnem[NP-1][0]);
  borna[c+1] = NP;
  for (c=255; c>0; c--) 
    if (borna[c]>=0 && borna[c-1]<0) borna[c-1] = borna[c];
  fclose(f);
  fprintf(stderr, "Loaded %d names.\n", NP);
}

int ResetStats() {
  for (int i=0; i<NP; i++) {
     psez[i] = pfy[i] = ply[i] = 0;
     dmeci[i] = dtit[i] = drez[i] = dgol[i] = 0;
     kmeci[i] = ktit[i] = krez[i] = kgol[i] = 0;
     emeci[i] = etit[i] = erez[i] = egol[i] = 0;
     nmeci[i] = ntit[i] = nrez[i] = ngol[i] = 0;
     tmeci[i] = ttit[i] = trez[i] = tgol[i] = 0;
     rank[i] = i;
  }
}

void websafeMnem(char *om, char *nm) {
  strcpy(nm, om);
  for (int i=0; i<6; i++) {
         if (om[i]=='ã') nm[i]='0';
    else if (om[i]=='º') nm[i]='1';
    else if (om[i]=='þ') nm[i]='2';
    else if (om[i]=='â') nm[i]='3';
    else if (om[i]=='î') nm[i]='4';
    else if (om[i]=='ª') nm[i]='6';
    else if (om[i]=='Þ') nm[i]='7';
    else if (om[i]=='Î') nm[i]='9';
    else if (om[i]=='é') nm[i]='E';
    else if (om[i]=='ö') nm[i]='O';
    else if (om[i]=='ü') nm[i]='U';
  }
}

#define DOB_DD_MM_YYYY	0
#define DOB_YYYYMMDD	1

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

void Collect(int year) {
  char filename[128];
  char s[1000], *tok[100];
  int sm, st, sr, sg;

  sprintf(filename, "ncatalog-%d.dat", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
    return;
  }
  
  int i = 0;
  int n = year - FY;
  do {
    fgets(s, 1000, f);
    if (strlen(s)<10) continue;
    tok[0] = strtok(s, ",\n");
    for (int k=1; k<CAT_COLS; k++) tok[k] = strtok(NULL, ",\n");

    db[n][i] = new char*[CAT_COLS];
    for (int k=0; k<CAT_COLS; k++) {
      int l = 0;
      if (tok[k]!=NULL) {
        l = strlen(tok[k]);
        db[n][i][k] = new char[l+1];
        strcpy(db[n][i][k], tok[k]);
      }
    }    
    i++;
    db[n][i] = NULL;

    s[0] = 0;
  } while (!feof(f));
  fclose(f);
}

void CollectCup(int year) {
  char filename[128];
  char s[1000], *tok[100];
  int sm, st, sr, sg;

  sprintf(filename, "cupa-%d.dat", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
    return;
  }
  
  int i = 0;
  int n = year - KFY;
  do {
    fgets(s, 1000, f);
    if (strlen(s)<10) continue;
    tok[0] = strtok(s, ",\n");
    for (int k=1; k<TD_NUM; k++) tok[k] = strtok(NULL, ",\n");

    kdb[n][i] = new char*[TD_NUM];
    for (int k=0; k<TD_NUM; k++) {
      int l = 0;
      if (tok[k]!=NULL) {
        l = strlen(tok[k]);
        kdb[n][i][k] = new char[l+1];
        strcpy(kdb[n][i][k], tok[k]);
      }
    }    
    i++;
    kdb[n][i] = NULL;

    s[0] = 0;
  } while (!feof(f));
  fclose(f);
}

void CollectEC(int year) {
  char filename[128];
  char s[1000], *tok[100];
  int sm, st, sr, sg;

  sprintf(filename, "euro-%d.dat", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
    return;
  }
  
  int i = 0;
  int n = year - ECFY;
  do {
    fgets(s, 1000, f);
    if (strlen(s)<10) continue;
    tok[0] = strtok(s, ",\n");
    for (int k=1; k<TD_NUM; k++) tok[k] = strtok(NULL, ",\n");

    edb[n][i] = new char*[TD_NUM];
    for (int k=0; k<TD_NUM; k++) {
      int l = 0;
      if (tok[k]!=NULL) {
        l = strlen(tok[k]);
        edb[n][i][k] = new char[l+1];
        strcpy(edb[n][i][k], tok[k]);
      }
    }    
    i++;
    edb[n][i] = NULL;

    s[0] = 0;
  } while (!feof(f));
  fclose(f);
}

void CollectNat(int year) {
  char filename[128];
  char s[1000], *tok[100];
  int sm, st, sr, sg;

  sprintf(filename, "nat-%d.dat", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
    return;
  }
  
  int i = 0;
  int n = year - NFY;
  do {
    fgets(s, 1000, f);
    if (strlen(s)<10) continue;
    tok[0] = strtok(s, ",\n");
    for (int k=1; k<TD_NUM; k++) tok[k] = strtok(NULL, ",\n");

    ndb[n][i] = new char*[TD_NUM];
    for (int k=0; k<TD_NUM; k++) {
      int l = 0;
      if (tok[k]!=NULL) {
        l = strlen(tok[k]);
        ndb[n][i][k] = new char[l+1];
        strcpy(ndb[n][i][k], tok[k]);
      }
    }    
    i++;
    ndb[n][i] = NULL;

    s[0] = 0;
  } while (!feof(f));
  fclose(f);
}

void DivStats(int year) {
  int sm, st, sr, sg, xt;
  int n = year - FY;
  int i = 0;

  while (i<MAX_DB && db[n][i]!=NULL) {
    int p = FindMnem(db[n][i][CAT_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", db[n][i][CAT_MNEM]);
      i++;
      continue;
    }
    sm = st = sr = sg = 0;
    if (db[n][i][CAT_CAPS])  sm = atoi(db[n][i][CAT_CAPS]);
    if (db[n][i][CAT_START]) st = atoi(db[n][i][CAT_START]);
    if (db[n][i][CAT_SUB])   sr = atoi(db[n][i][CAT_SUB]);
    if (db[n][i][CAT_GOALS]) sg = atoi(db[n][i][CAT_GOALS]);
    if (db[n][i][CAT_GREC])  sr = atoi(db[n][i][CAT_GREC]);
  
    if (year != 1957) {
      dmeci[p] += sm;
      dtit[p]  += st;
      drez[p]  += sr;
      dgol[p]  += sg+sr;
    }

    if (sm>0) {
      if (year!=pfy[p] && year!=ply[p]) psez[p]++;
      if (pfy[p]==0) pfy[p] = year;
      if (year<pfy[p]) pfy[p] = year;
      if (year>ply[p]) ply[p] = year;
    }

    i++;
  }
}

void CupStats(int year) {
  int sm, st, sr, sg, xt;
  int n = year - KFY;
  int i = 0;

  while (i<MAX_KDB && kdb[n][i]!=NULL) {
    int p = FindMnem(kdb[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", kdb[n][i][TD_MNEM]);
      i++;
      continue;
    }
    sm = st = sr = sg = 0;
    if (kdb[n][i][TD_MECIURI]) sm = atoi(kdb[n][i][TD_MECIURI]);
    if (kdb[n][i][TD_TITULAR]) st = atoi(kdb[n][i][TD_TITULAR]);
    if (kdb[n][i][TD_REZERVA]) sr = atoi(kdb[n][i][TD_REZERVA]);
    if (kdb[n][i][TD_GOLURI])  sg = atoi(kdb[n][i][TD_GOLURI]);
  
    kmeci[p] += sm;
    ktit[p]  += st;
    krez[p]  += sr;
    kgol[p]  += sg;

    if (sm>0) {
      if (year!=pfy[p] && year!=ply[p]) psez[p]++;
      if (pfy[p]==0) pfy[p] = year;
      if (year<pfy[p]) pfy[p] = year;
      if (year>ply[p]) ply[p] = year;
    }

    i++;
  }
}

void ECStats(int year) {
  int sm, st, sr, sg, xt;
  int n = year - ECFY;
  int i = 0;

  while (i<MAX_EDB && edb[n][i]!=NULL) {
    int p = FindMnem(edb[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", edb[n][i][TD_MNEM]);
      i++;
      continue;
    }
    sm = st = sr = sg = 0;
    if (edb[n][i][TD_MECIURI]) sm = atoi(edb[n][i][TD_MECIURI]);
    if (edb[n][i][TD_TITULAR]) st = atoi(edb[n][i][TD_TITULAR]);
    if (edb[n][i][TD_REZERVA]) sr = atoi(edb[n][i][TD_REZERVA]);
    if (edb[n][i][TD_GOLURI])  sg = atoi(edb[n][i][TD_GOLURI]);
  
    emeci[p] += sm;
    etit[p]  += st;
    erez[p]  += sr;
    egol[p]  += sg;

    if (sm>0) {
      if (year!=pfy[p] && year!=ply[p]) psez[p]++;
      if (pfy[p]==0) pfy[p] = year;
      if (year<pfy[p]) pfy[p] = year;
      if (year>ply[p]) ply[p] = year;
    }

    i++;
  }
}

void NatStats(int year) {
  int sm, st, sr, sg, xt;
  int n = year - NFY;
  int i = 0;

  while (i<MAX_NDB && ndb[n][i]!=NULL) {
    int p = FindMnem(ndb[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ndb[n][i][TD_MNEM]);
      i++;
      continue;
    }
    sm = st = sr = sg = 0;
    if (ndb[n][i][TD_MECIURI]) sm = atoi(ndb[n][i][TD_MECIURI]);
    if (ndb[n][i][TD_TITULAR]) st = atoi(ndb[n][i][TD_TITULAR]);
    if (ndb[n][i][TD_REZERVA]) sr = atoi(ndb[n][i][TD_REZERVA]);
    if (ndb[n][i][TD_GOLURI])  sg = atoi(ndb[n][i][TD_GOLURI]);
  
    nmeci[p] += sm;
    ntit[p]  += st;
    nrez[p]  += sr;
    ngol[p]  += sg;

    if (sm>0) {
      if (year!=pfy[p] && year!=ply[p]) psez[p]++;
      if (pfy[p]==0) pfy[p] = year;
      if (year<pfy[p]) pfy[p] = year;
      if (year>ply[p]) ply[p] = year;
    }

    i++;
  }
}

void Ranking(int cr) {
  int sorted, last;
  for (int i=0; i<NP; i++) {
    rank[i] = i;
    tmeci[i] = dmeci[i] + kmeci[i] + emeci[i] + nmeci[i];
    tgol[i] = dgol[i] + kgol[i] + egol[i] + ngol[i];
  }
  last = NP-1;
  do {
    sorted = 1;
    for (int i=0; i<NP-1; i++) { 
      if (tmeci[rank[i+1]] > tmeci[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
}

void HTMLHeader() {  
  fprintf(of, "<HTML>\n<TITLE>Jucãtori în Prima Divizie, Cupa României, Cupele Europene, Echipa Naþionalã</TITLE>\n");
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");
}

void HTMLTable() {
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  fprintf(of, "<H3>Jucãtori în Prima Divizie, Cupa României, Cupele Europene ºi Echipa Naþionalã</H3>\n");

  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
//  fprintf(of, "<TABLE WIDTH=\"99%%\" cellpadding=\"2\" RULES=\"groups\" frame=\"box\">\n");  
  fprintf(of, "<COLGROUP><COL SPAN=\"5\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"4%%\"> #</TH>");
  fprintf(of, "<TH WIDTH=\"15%%\"> Prenume </TH>");
  fprintf(of, "<TH WIDTH=\"14%%\"> Nume </TH>");
  fprintf(of, "<TH WIDTH=\"7%%\"> D.N. </TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> Naþ </TH>");
  fprintf(of, "<TH WIDTH=\"7%%\"> Primul sezon </TH>");
  fprintf(of, "<TH WIDTH=\"7%%\"> Ultimul sezon </TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> MD</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> GD</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> MC</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> GC</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> ME</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> GE</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> MN</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> GN</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> G</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n"); 

  for (int i=0; i<NP; i++) {
    int x = rank[i];
    if (tmeci[x]==0) continue;
    fprintf(of, "<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD align=\"right\">%d.</TD>", i+1);
    fprintf(of, "<TD align=\"left\">%s</TD>", ppren[x]);
    makeHexlink(x);
    fprintf(of, "<TD align=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"jucatori/%s.html\">%s</A></TD>",
        pname[x], ppren[x], hexlink, pname[x]);
    CanonicDOB(pdob[x], DOB_DD_MM_YYYY);
    fprintf(of, "<TD align=\"right\" sorttable_customkey=\"%d\">%s</TD>", NumericDOB(pdob[x], DOB_YYYYMMDD), pdob[x]);
    fprintf(of, "<TD align=\"center\">%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", pcty[x], pcty[x]);
    fprintf(of, "<TD align=\"center\">%d</TD>", pfy[x]);
    fprintf(of, "<TD align=\"center\">%d</TD>", ply[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", dmeci[x]);
    fprintf(of, "<TD align=\"right\"><FONT COLOR=\"0000FF\">%d</FONT></TD>", dgol[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", kmeci[x]);
    fprintf(of, "<TD align=\"right\"><FONT COLOR=\"0000FF\">%d</FONT></TD>", kgol[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", emeci[x]);
    fprintf(of, "<TD align=\"right\"><FONT COLOR=\"0000FF\">%d</FONT></TD>", egol[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", nmeci[x]);
    fprintf(of, "<TD align=\"right\"><FONT COLOR=\"0000FF\">%d</FONT></TD>", ngol[x]);
    fprintf(of, "<TD align=\"right\"><B>%d</B></TD>", tmeci[x]);
    fprintf(of, "<TD align=\"right\"><B><FONT COLOR=\"0000FF\">%d</FONT></B></TD>", tgol[x]);
    fprintf(of, "</TR>\n");
  }
  fprintf(of, "\n</TABLE>\n");
}

void HTMLFooter() {
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

int main(int argc, char **argv) {

  verbosity = 2;
  FY   = 1933; LY   = 2013;
  KFY  = 1934; KLY  = 2013;
  ECFY = 1957; ECLY = 2014;
  NFY  = 1922; NLY  = 2013;
  tm = -1;
  pl = 0;
  cr = CR_M;

  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-q")==0) verbosity = 0;
    else if (strcmp(argv[k], "-fy")==0 && k<argc-1)  {
      FY = atoi(argv[++k]);
    }
    else if (strcmp(argv[k], "-ly")==0 && k<argc-1)  {
      LY = atoi(argv[++k]);
    }
    else if (strcmp(argv[k], "-kfy")==0 && k<argc-1)  {
      KFY = atoi(argv[++k]);
    }
    else if (strcmp(argv[k], "-lfy")==0 && k<argc-1)  {
      KLY = atoi(argv[++k]);
    }
    else if (strcmp(argv[k], "-t")==0 && k<argc-1)  {
      tm = atoi(argv[++k]);
    }
    else if (strcmp(argv[k], "-rk")==0 && k<argc-1)  {
      cr = atoi(argv[++k]);
    }
    else if (strcmp(argv[k], "-p")==0 && k<argc-1)  {
      pl = atoi(argv[++k]);
    }
  }

  Load();
  LoadPlayers();

  int nums = LY-FY+1;
  db = new char***[nums];
  for (int s=0; s<nums; s++) {
     db[s] = new char**[MAX_DB];
  }

  int numks = KLY-KFY+1;
  kdb = new char***[numks];
  for (int s=0; s<numks; s++) {
     kdb[s] = new char**[MAX_KDB];
  }

  int numecs = ECLY-ECFY+1;
  edb = new char***[numecs];
  for (int s=0; s<numecs; s++) {
     edb[s] = new char**[MAX_EDB];
  }

  int numns = NLY-NFY+1;
  ndb = new char***[numns];
  for (int s=0; s<numns; s++) {
     ndb[s] = new char**[MAX_NDB];
  }

  pdb = new char**[MAX_PSZ];
  for (int s=0; s<MAX_PSZ; s++) {
    pdb[s] = new char*[NUM_PTD];
    for (int j=0; j<NUM_PTD; j++) pdb[s][j] = new char[MAX_PTDLEN];
  }

  ResetStats();

  for (year=FY; year<=LY; year++) {
    Collect(year);
    DivStats(year);
  }

  for (year=KFY; year<=KLY; year++) {
    CollectCup(year);
    CupStats(year);
  }

  for (year=ECFY; year<=ECLY; year++) {
    CollectEC(year);
    ECStats(year);
  }

  for (year=NFY; year<=NLY; year++) {
    CollectNat(year);
    NatStats(year);
  }

    sprintf(ofilename, "html/catalog-general.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Ranking(cr);
    HTMLHeader();
    HTMLTable();
    HTMLFooter();
  
    fclose(of);


  return 0;
}

