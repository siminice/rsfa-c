#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"

#define TM_LIGA	-1
#define TM_EURO	-2
#define TM_NATL	-3
#define TM_CUPA	-4
#define EURO 1000
#define NATIONALA 39000
#define PSO_TIME     200
#define UNKNOWN_TIME 999

int verbosity;
int tm, pl, cr;
int FY, LY;
int CFY, CLY;
int EFY, ELY;
int KFY, KLY;
int NFY, NLY;

char **club;
char **mnem;
char **ctty;
char **dir;
int  NC, NN;
int year;
int *dva, *ecc, *echn;
int **ina;
bool winter;
int num_winter;
int *start_winter, *end_winter;

Ranking *R, *Opp;

const char* fxcol[] = {"F0F0B0", "AAFFAA", "FF8888"};
const char* cupround = "CFSQOª";
const char* ecln = "SCKU";

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
Aliases **CL;

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
  if (f==NULL) {
		fprintf(stderr, "Error: file \"teams.dat\" not found.\n");
		return 0;
	}
  fscanf(f, "%d\n", &NC);
  R = new Ranking(NC);
  Opp = new Ranking(NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  dva = new int[NC];
	ina = new int*[NC];
  ecc = new int[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  for (int i=0; i<NC; i++) {
		ina[i]= new int[300];
		for (int y=0; y<300; ++y) ina[i][y] = 0;
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
  if (f==NULL) {
    fprintf(stderr, "Error: file \"alias.dat\" not found.\n");
    return 0;
  }
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

  f = fopen("country.dat", "rt");
  if (!f) return 0;
  fscanf(f, "%d\n", &NN);
	CL = new Aliases*[NN];
  for (int i=0; i<NN; i++) CL[i] = new Aliases;
  ctty = new char*[NN];
  dir = new char*[NN];
  for (int i=0; i<NN; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, ":");
    dir[i] = strdup(tok[0]);
    tok[1] = strtok(NULL, "*");
		for  (int j=2; j<20; ++j) tok[j] = strtok(NULL, "*");
		int k = 2;
    while(tok[k]) {
      ystr = strtok(tok[k], " ");
      name = strtok(NULL, "~");
      nick = strtok(NULL, "@");
      int y = atoi(ystr);
      alias *a = new alias(y, name, name);
      CL[i]->Append(a);
//      fprintf(stderr, "Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
      k++;
    }
    ctty[i] = strdup(name);
    s[0] = 0;
  }
  fclose(f);

	f = fopen("part.a", "rt");
  if (f==NULL) {
		fprintf(stderr, "Error: file \"part.a\" not found.\n");
		return 0;
	}

	int y, dummy, n, t;
  while (!feof(f)) {
      fscanf(f, "%d %d", &y, &n);
      if (feof(f)) continue;
      for (int i=0; i<n; i++) {
        fscanf(f, "%d", &t);
				if (y>=CFY && t>=0 && t<NC) ina[t][y-CFY] = 1;
      }
      fgets(s, 200, f);
  }
	fclose(f);
  return 1;
}

char *EuroName(int t, int year, int nick) {
  int ct = t/1000;
  int cl = t%1000;
	if (cl==0) return CL[ct]->GetName(year);
  char filename[128];
  char s[128];
  char *name = new char[64];
  char *temp = new char[64];
  sprintf(name, "%d/%d", ct, cl);
  sprintf(filename, "../%s/euroteams.dat", dir[ct]);
  FILE *f = fopen(filename, "rt");
  if (!f) return name;
  for (int i=0; i<=cl; ++i) fgets(s, 128, f);
  strcpy(temp, s);
  name = strtok(temp, ",\n");
  if (!nick) {
    name = strtok(NULL, ",\n");
  }
  fclose(f);
  return name;
}

char *NameOf(Aliases **L, int t, int y) {
	if (t>=EURO) return EuroName(t, y, 0);
  char *s = L[t]->GetName(y);
  if (!s) return club[t];
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
	if (t>=EURO) return EuroName(t, y, 1);
  char *s = L[t]->GetNick(y);
  if (!s) return mnem[t];
  return s;
}

int GetYear(char *sd) {
	if (!sd) return -1;
	if (strlen(sd)<10) return -1;
	char sy[16];
	strncpy(sy, sd+6, 4);
	sy[4] = 0;
	int y = strtol(sy, NULL, 10);
	return y;
}

/* *************************************** */

#define MAX_NAMES	20000
#define MAX_SEASONS	200
#define MAX_CDB		800
#define MAX_LDB		500
#define MAX_CKDB	750
#define MAX_LKDB	 50
#define MAX_CEDB	200
#define MAX_LEDB	100
#define MAX_CNDB	100
#define MAX_LNDB	 30
#define LDB_COLS	 60
#define VDB_COLS	 30
#define MAX_LOG	   1000

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
#define OLDCAT_COLS CAT_GOALS+1

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

#define MAX_PSZ		50
#define MAX_PTDLEN	32

#define PTD_SEZON	0
#define PTD_ECHIPA	1
#define PTD_POST	2
#define PTD_DIVM	3
#define PTD_DIVN	4
#define PTD_DIVG	5
#define PTD_CUPM	6
#define PTD_CUPN	7
#define PTD_CUPG	8
#define PTD_EURM	9
#define PTD_EURN	10
#define PTD_EURG	11
#define PTD_NATM	2
#define PTD_NATN	3
#define PTD_NATT	3
#define PTD_NATR	4
#define PTD_NATG	5
#define NUM_PTD		PTD_EURG+1
#define PTD_DIVT	4
#define PTD_DIVR	5
#define PTD_CUPT	7
#define PTD_CUPR	8
#define PTD_EURT	10
#define PTD_EURR	11

#define DB_HOME          0
#define DB_AWAY          1
#define DB_SCORE         2
#define DB_DATE          3
#define DB_COMP          4
#define DB_ROUND         5
#define DB_VENUE         6
#define DB_ATTEND        7
#define DB_WEATHER       8
#define DB_ROSTER1       9
#define DB_COACH1       31
#define DB_ROSTER2      32
#define DB_COACH2       54
#define DB_REF          55
#define DB_ASIST1       56
#define DB_ASIST2       57
#define DB_OBSERV       58
#define DB_T1            8
#define DB_T2           31

#define COMP_LIGA	0
#define COMP_CUPA	1
#define COMP_EURO	2
#define COMP_NATL	3
#define NUM_COMPS COMP_NATL+1

#define CR_NONE 0
#define CR_M	1
#define	CR_G	2
#define CR_T	3
#define CR_R	4
#define CR_S	5

char ****ccdb;
char ****ckdb;
char ****cedb;
char ****cndb;
char ***pdb;
char ****ladb;
char ****lcdb;
char ****lkdb;
char ****ledb;
char ****lndb;
char ****vadb;
char ****vcdb;
char ****vkdb;
char ****vedb;
char ****vndb;
int  echid[MAX_PSZ];
int  sezid[MAX_PSZ];
int  clist[MAX_NAMES][MAX_LOG];		// caps list for each player, clist[0] = #caps
int  mlist[MAX_NAMES][MAX_LOG];		// minutes in game for each player
int  elist[MAX_NAMES][MAX_LOG];		// event list for each player
int  ord[MAX_SEASONS][MAX_LDB];
int  qord[MAX_NAMES];
int  qd[MAX_NAMES];
int  ngm[MAX_SEASONS][NUM_COMPS+1];
int  debord[MAX_NAMES];
int  qdeb[MAX_NAMES];

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
int *pmeci;
int *ptit;
int *prez;
int *pgol;
int *ppen;
int *pmin;
int *pown;
int *prec;
int *pred;
int *rank;

int borna[256];

char ofilename[128];
FILE *of;

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

/* *************************************** */

int binFindMnem(char *s) {
  int c = ((unsigned char)s[0]);
  int lo = borna[c];
  int hi = borna[c+1]-1;
  int mid = (hi+lo)/2;
  while (lo<hi) {
    int comp = strcmp(s, pmnem[mid]);
    if (comp==0) return mid;
    if (comp<0) { hi = mid; } else { lo = mid+1; }
    mid = (lo+hi)/2;
  }
  if (strcmp(pmnem[mid], s)==0) return mid;
  return -1;
}

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
  pmeci = new int[MAX_NAMES];
  ptit = new int[MAX_NAMES];
  prez = new int[MAX_NAMES];
  pgol = new int[MAX_NAMES];
  ppen = new int[MAX_NAMES];
  pmin = new int[MAX_NAMES];
  pown = new int[MAX_NAMES];
  prec = new int[MAX_NAMES];
  pred = new int[MAX_NAMES];
  rank = new int[MAX_NAMES];
  echn = new int[MAX_NAMES];

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

     if (i==0 || (i>0 && pmnem[i-1][0]!=pmnem[i][0])) {
       c = ((unsigned char) pmnem[i][0]);
       borna[c] = i;
     }
     psez[i] = pfy[i] = ply[i] = pmeci[i] = ptit[i] = prez[i] = pgol[i] = 0;
		 pmin[i] = ppen[i] = pown[i] = prec[i] = pred[i] = 0;
     echn[i] = 0;
     rank[i] = i;
	 clist[i][0] = 0;
	 qdeb[i] = 0x7FFFFFFF;
	for (int j=0; j<MAX_LOG; j++) elist[i][j] = 0;
  }
  c = ((unsigned char)pmnem[NP-1][0]);
  borna[c+1] = NP;
  for (c=255; c>0; c--)
    if (borna[c]>=0 && borna[c-1]<0) borna[c-1] = borna[c];
  fclose(f);
  fprintf(stderr, "Loaded %d names.\n", NP);
}

void ResetStats() {
  for (int i=0; i<NP; i++) {
     psez[i] = pfy[i] = ply[i] = pmeci[i] = ptit[i] = prez[i] = pgol[i] = 0;
		 pmin[i] = ppen[i] = pown[i] = prec[i] = pred[i] = 0;
     rank[i] = i;
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

int Season(char *ss) {
  char s[32];
  strcpy(s, ss);
  int winter = 0;
  if (strchr(s, '/')!=NULL) winter = 1;
  strtok(s, "-/");
  int y = atoi(s);
  return y + winter;
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

int NumericDate(char *s) {
	int l = strlen(s);
	if (l<10) return -1;
    while (s[0]==' ') s++;
	if (s[0]==0) return -1;
	int ssn = 1000*(s[6]-48) + 100*(s[7]-48) + 10*(s[8]-48) + (s[9]-48) - (FY-1);
    int day = 500*(s[3]-48) + 50*(s[4]-48) + 10*(s[0]-48) + (s[1]-48);
	int hrs = 0;
	int min = 0;
	if (l>15) {
		if (s[11]>='0' && s[11]<='9') {
			hrs = 10*(s[11]-48) + (s[12]-48) - 8;
			min = 10*(s[14]-48) + (s[15]-48);
		}
	}
	int x = min + 60*hrs + 1000*day + 1000000*ssn;
	/*
  int x = 10000*ssn;
  int expo = 1;
  for (int i=0; i<4; ++i) {
    x += expo*((int)s[dig[i]]-48);
    expo = expo*10;
  }
	*/
  return x;
}

int compareDate(const void* x1, const void* x2) {
  int i1 = *(int*)x1;
  int i2 = *(int*)x2;
  return (qd[i1]-qd[i2]);
}

void qSortDB(char ****adb, int year) {
  int y = year-FY;
  for (int i=0; i<ngm[y][NUM_COMPS]; ++i) {
    qd[i] = NumericDate(adb[y][i][DB_DATE]);
    qord[i]=i;
  }
  qsort(qord, ngm[y][NUM_COMPS], sizeof(int), compareDate);
  for (int i=0; i<ngm[y][NUM_COMPS]; i++) ord[y][i] = qord[i];
}

void qSortDebut() {
	for (int i=0; i<NP; ++i) { qord[i] = i; qd[i] = qdeb[i]; }
	qsort(qord, NP, sizeof(int), compareDate);
	for (int i=0; i<NP; i++) debord[qord[i]] = i;
}

void CollectLineups(int year, char ****ldb, int comp) {
  char filename[128];
  char s[4096], *tok[100];

	     if (comp==COMP_LIGA) sprintf(filename, "lineups-%d.db", year);
	else if (comp==COMP_CUPA) sprintf(filename, "cup-lineups-%d.db", year);
	else if (comp==COMP_EURO) sprintf(filename, "euro-lineups-%d.db", year);
	else if (comp==COMP_NATL) sprintf(filename, "nat-lineups-%d.db", year);

  FILE *f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: Cannot open file %s.\n", filename); return; }

  int i = 0;
  int y = year - FY;
	     if (comp==COMP_LIGA) y = year - CFY;
	else if (comp==COMP_CUPA) y = year - KFY;
	else if (comp==COMP_EURO) y = year - EFY;
	else if (comp==COMP_NATL) y = year - NFY;

//	ldb[y] = new char**[MAX_LDB];
  do {
    fgets(s, 4096, f);
    if (strlen(s)<10) continue;
	ldb[y][i] = new char*[LDB_COLS]; ldb[y][i][0] = 0;
    tok[0] = strtok(s, ",\n");
    for (int j=1; j<LDB_COLS; j++) tok[j] = strtok(NULL, ",\n");
    for (int j=0; j<LDB_COLS; j++) {
      if (tok[j]!=NULL) {
        int l = strlen(tok[j]);
        ldb[y][i][j] = new char[l+1];
        strcpy(ldb[y][i][j], tok[j]);
      }
			else { ldb[y][i][j] = new char[1]; ldb[y][i][j][0] = 0; }
    }
    ldb[y][++i] = NULL;
    s[0] = 0;
  } while (!feof(f));
	ngm[year-FY][comp] = i;
  fclose(f);
}

void CollectEvents(int year, char ****vdb, int comp) {
  char filename[128];
  char s[4096], *tok[100];

	     if (comp==COMP_LIGA) sprintf(filename, "events-%d.db", year);
	else if (comp==COMP_CUPA) sprintf(filename, "cup-events-%d.db", year);
	else if (comp==COMP_EURO) sprintf(filename, "euro-events-%d.db", year);
	else if (comp==COMP_NATL) sprintf(filename, "nat-events-%d.db", year);

  FILE *f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: Cannot open file %s.\n", filename); return; }

  int i = 0;
  int y = year - FY;
	     if (comp==COMP_LIGA) y = year - CFY;
	else if (comp==COMP_CUPA) y = year - KFY;
	else if (comp==COMP_EURO) y = year - EFY;
	else if (comp==COMP_NATL) y = year - NFY;
//	vdb[y] = new char**[MAX_LDB];
  do {
    fgets(s, 4096, f);
    if (strlen(s)<10) continue;
	vdb[y][i] = new char*[VDB_COLS]; vdb[y][i][0] = 0;
    tok[0] = strtok(s, ",\n");
    for (int j=1; j<VDB_COLS; j++) tok[j] = strtok(NULL, ",\n");
    for (int j=0; j<VDB_COLS; j++) {
      if (tok[j]!=NULL) {
        int l = strlen(tok[j]);
        vdb[y][i][j] = new char[l+1];
        strcpy(vdb[y][i][j], tok[j]);
      }
			else { vdb[y][i][j] = new char[1]; vdb[y][i][j][0] = 0; }
    }
    vdb[y][++i] = NULL;
    s[0] = 0;
  } while (!feof(f));
  fclose(f);
}

void CollectCatalog(int year, char ****cdb, int comp) {
  char filename[128];
  char s[1000], *tok[100];
  int sm, st, sr, sg;

       if (comp==COMP_LIGA) sprintf(filename, "ncatalog-%d.dat", year);
	else if (comp==COMP_CUPA) sprintf(filename, "ncupa-%d.dat", year);
	else if (comp==COMP_EURO) sprintf(filename, "neuro-%d.dat", year);
	else if (comp==COMP_NATL) sprintf(filename, "nnat-%d.dat", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Cannot open file %s.\n", filename);
    return;
  }

  int i = 0;
	int n = 0;
	     if (comp==COMP_LIGA) n = year - CFY;
	else if (comp==COMP_CUPA) n = year - KFY;
	else if (comp==COMP_EURO) n = year - EFY;
	else if (comp==COMP_NATL) n = year - NFY;
	
  do {
    fgets(s, 1000, f);
    if (strlen(s)<10) continue;
    tok[0] = strtok(s, ",\n");
    for (int k=1; k<CAT_COLS; k++) tok[k] = strtok(NULL, ",\n");

    cdb[n][i] = new char*[CAT_COLS]; cdb[n][i][0] = 0;
    for (int k=0; k<CAT_COLS; k++) {
      int l = 0;
      if (tok[k]!=NULL) {
        l = strlen(tok[k]);
        cdb[n][i][k] = new char[l+1];
        strcpy(cdb[n][i][k], tok[k]);
      }
    }
    i++;
    cdb[n][i] = NULL;

    s[0] = 0;
  } while (!feof(f));
  fclose(f);
}

int wdl(char *s) {
	if (!s) return -1;
	int v = atoi(s);
	int x = v/100;
	int y = v%100;
	if (x>y) return 1;
	if (x<y) return 2;
	return 0;
}

int revwdl(int x) {
	return (7-3*x)*x/2;
}

void ExtractLists(int year, char ****ldb, char ****edb) {
	char spi[64], *sp, *sm;
	int min, n, pid, r;
  int y = year - FY;
	int i, j;

  for (i=0; i<ngm[y][NUM_COMPS]; ++i) {
		r = ord[y][i];
		int ndt = NumericDate(ldb[y][r][DB_DATE]);
		int wx = wdl(ldb[y][r][DB_SCORE]);
		int wy = revwdl(wx);
        int th = atoi(ldb[y][r][DB_HOME]);
        int ta = atoi(ldb[y][r][DB_AWAY]);
		for (j=DB_ROSTER1; j<DB_ROSTER2+22; j++) if (j!=DB_COACH1) {
			strcpy(spi, ldb[y][r][j]);
			sp = strtok(spi, ":");
			sm = strtok(NULL, ":");
			if (sm!=NULL) min=atoi(sm); else min=0;
			if (min>0 &&
                ((j<DB_COACH1 && (th<=EURO || th==NATIONALA)) ||
                 (j>DB_COACH1 && (ta<=EURO || ta==NATIONALA)))) {
				pid = binFindMnem(sp);
				n = clist[pid][0];
				if (pid>=0) {
					if (r<ngm[y][COMP_LIGA]) {
						if (qdeb[pid]<0 || qdeb[pid]>1000000000) {
                           qdeb[pid] = ndt;
  						   if ((j>=DB_ROSTER1+11 && j<DB_ROSTER1+22) || (j>=DB_ROSTER2+11)) {
						 	 qdeb[pid] += (90-min);
						   }
                        }
					}
					clist[pid][n+1] = 1000*y+r;
					mlist[pid][n+1] = 1000*(j<DB_ROSTER2?wx:wy)+min;
					if (j>=DB_ROSTER2) mlist[pid][n+1] += 10000;
					clist[pid][0]++;
				}
			}
		}
		for (j=0; j<VDB_COLS; j++) {
			if (edb[y][r][j]==NULL || edb[y][r][j][0]==0x0 || edb[y][r][j][0]==' ' || edb[y][r][j][0]=='~') continue;
			strcpy(spi, edb[y][r][j]);
			char evt = edb[y][r][j][6];
			if (evt==39 || evt==34) {
				spi[6] = 0;
                int evm = atoi(spi+7);
				pid = binFindMnem(sp);
				if (pid>=0 && (evm < PSO_TIME || evm == UNKNOWN_TIME)) {
					elist[pid][clist[pid][0]]++;
				}
			}
		}
  }
}

void MergeDB(int year) {
	int yc = year - CFY;
	int yk = year - KFY;
	int ye = year - EFY;
	int yn = year - NFY;
	int y  = year - FY;
	int offset = 0;
	for (int i=0; i<ngm[y][COMP_LIGA]; ++i) {
		ladb[y][i] = lcdb[yc][i];
		vadb[y][i] = vcdb[yc][i];
	}
	offset += ngm[y][COMP_LIGA];
	for (int i=0; i<ngm[y][COMP_CUPA]; ++i) {
		ladb[y][i+offset] = lkdb[yk][i];
		vadb[y][i+offset] = vkdb[yk][i];
	}
	offset += ngm[y][COMP_CUPA];
	for (int i=0; i<ngm[y][COMP_EURO]; ++i) {
		ladb[y][i+offset] = ledb[ye][i];
		vadb[y][i+offset] = vedb[ye][i];
	}
	offset += ngm[y][COMP_EURO];
	for (int i=0; i<ngm[y][COMP_NATL]; ++i) {
		ladb[y][i+offset] = lndb[yn][i];
		vadb[y][i+offset] = vndb[yn][i];
	}
	ngm[y][NUM_COMPS] = 0;
	for (int c=0; c<NUM_COMPS; ++c) ngm[y][NUM_COMPS] += ngm[y][c];
}

void TeamStats(int year, int t, int pl) {
  int sm, st, sr, sg, sn, sp, sa, sh, scr, xt;
  int n = year - CFY;
  int i = 0;

  while (i<MAX_CDB && ccdb[n][i]!=NULL) {
    int p = FindMnem(ccdb[n][i][CAT_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ccdb[n][i][CAT_MNEM]);
      i++;
      continue;
    }
    xt = -1999;
    if (ccdb[n][i][CAT_CLUB]) xt = atoi(ccdb[n][i][CAT_CLUB]);
    if (xt>=0 && xt<NC) {
			dva[xt] = 1;
		}
    if (t>=0 && xt!=t) {
      i++;
      continue;
    }
    sm = st = sr = sg = 0;
    sn = sp = sa = sh = 0;
    scr = 0;
    if (ccdb[n][i][CAT_CAPS])   sm  = atoi(ccdb[n][i][CAT_CAPS]);
    if (ccdb[n][i][CAT_START])  st  = atoi(ccdb[n][i][CAT_START]);
    if (ccdb[n][i][CAT_SUB])    sr  = atoi(ccdb[n][i][CAT_SUB]);
    if (ccdb[n][i][CAT_GOALS])  sg  = atoi(ccdb[n][i][CAT_GOALS]);
    if (ccdb[n][i][CAT_MIN])    sn  = atoi(ccdb[n][i][CAT_MIN]);
    if (ccdb[n][i][CAT_PK])     sp  = atoi(ccdb[n][i][CAT_PK]);
    if (ccdb[n][i][CAT_OWN])    sa  = atoi(ccdb[n][i][CAT_OWN]);
    if (ccdb[n][i][CAT_GREC])   sh  = atoi(ccdb[n][i][CAT_GREC]);
    if (ccdb[n][i][CAT_RED])    scr = atoi(ccdb[n][i][CAT_RED]);

    if (year !=1957) {
      pmeci[p] += sm;
      ptit[p]  += st;
      prez[p]  += sr;
      pgol[p]  += sg;
	  pmin[p]  += sn;
	  ppen[p]  += sp;
	  pown[p]  += sa;
	  prec[p]  += sh;
      pred[p]  += scr;
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

void HTMLLineupsBox(int year) {
	int nty = 0;
	int part[64];
	for (int t=0; t<NC; ++t) {
		if (ina[t][year-CFY]) {
			part[nty++] = t;
		}
	}
	int ncols = nty/2;
//	if (ncols%3!=0) ncols = nty/4;
	char ssn[32];
	SeasonName(year, ssn);
	fprintf(of, "<CENTER>\n");
	fprintf(of, "<H3>Loturi jucãtori %s</H3>\n", ssn);
	fprintf(of, "<TABLE BORDER=\"1\" CELLPADDING=\"2\" CELLSPACING=\"3\" FRAME=\"box\">\n");
	for (int i=0; i<nty; ++i) {
		if (i%ncols==0) {
			if (i>0) fprintf(of, "</TR>");
			fprintf(of, "<TR>");
		}
		fprintf(of, "<TD ALIGN=\"center\"> <A HREF=\"lot-%d-%d.html\">%s</A></TD>", part[i], year, NickOf(L, part[i], year));
	}
	fprintf(of, "</TR></TABLE></CENTER><HR>\n");
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
//  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<TABLE id=\"all\" class=\"display\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>Data naºterii</TH>");
  fprintf(of, "<TH>Naþ.</TH>");
  fprintf(of, "<TH>Club</TH>");
  fprintf(of, "<TH>Nr</TH>");
  fprintf(of, "<TH>Post</TH>");
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

void HTMLInA(int t, int year) {
	fprintf(of, "<CENTER>\n");
	fprintf(of, "<TABLE BORDER=\"1\" CELLPADDING=\"2\" CELLSPACING=\"3\" FRAME=\"box\">\n");
	char ssn[64];
	for (int y=(CFY/10)*10; y<=CLY; y++) {
		if (y%10==0) {
			if (y>1930) fprintf(of, "</TR>\n");
			fprintf(of, "<TR>");
		}
		fprintf(of, "<TD ALIGN=\"center\">");
		if (y>=CFY && y<=CLY) {
			SeasonName(y, ssn);
			if (ina[t][y-CFY]) {
				fprintf(of, "<A HREF=\"lot-%d-%d.html\">%s%s%s</A>", t, y,
				(year==y?"<FONT COLOR=\"990000\">":""), ssn, (year==y?"</FONT>":""));
			}
			else {
				fprintf(of, "%s", ssn);
			}
		}
		fprintf(of, "</TD>");
	}
	fprintf(of, "</TR></TABLE>\n");
	fprintf(of, "</CENTER><HR>\n");
}

void YearlyTeamStats(int t, int year) {
	if (t<0 || t>=NC) return;
	if (year<CFY || year>CLY) return;
  int n = year - CFY;
  int i = 0;
	if (!ina[t][n]) return;

	char filename[128];
	sprintf(filename, "html/lot-%d-%d.html", t, year);
	of = fopen(filename, "wt");
	if (!of) {
		fprintf(stderr, "ERROR: cannot open file \"html/lot-%d-%d.html\" for output.\n", t, year);
		return;
	}

	char ssn[32];
	SeasonName(year, ssn);
	fprintf(of, "<HTML>\n<TITLE>Lot %s %s</TITLE>\n", NameOf(L, t, year), ssn);
 	fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
 	fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
    DatatablesHeader();
    fprintf(of, "</HEAD>\n<BODY>\n");
    fprintf(of, "<script src=\"sorttable.js\"></script>\n");
	HTMLInA(t, year);
	int prevy = year-1;	while (prevy>=CFY && ina[t][prevy-CFY]==0) prevy--;
	int nexty = year+1;	while (prevy<=CLY && ina[t][nexty-CFY]==0) nexty++;
	char plink[128];
	char nlink[128];
	plink[0] = nlink[0] = 0;
	if (prevy>=CFY)
		sprintf(plink, "<A HREF=lot-%d-%d.html><IMG HEIGHT=\"20\" SRC=\"prev.gif\"></A>", t, prevy);
	if (nexty<=CLY)
		sprintf(nlink, "<A HREF=lot-%d-%d.html><IMG HEIGHT=\"20\" SRC=\"next.gif\"></A>", t, nexty);
	fprintf(of, "<H3>%s Lot %s %s %s</H3>\n", plink, NameOf(L, t, year), ssn, nlink);
	HTMLLineupsHeader();
	int j=0;
	int xt;
  while (i<MAX_CDB && ccdb[n][i]!=NULL) {
    if (ccdb[n][i][CAT_CLUB]) xt = atoi(ccdb[n][i][CAT_CLUB]);
    if (xt==t) {
    	int p = FindMnem(ccdb[n][i][CAT_MNEM]);
    	if (p<0) {
    	  fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ccdb[n][i][CAT_MNEM]);
    	  i++;
    	  continue;
    	}
	    fprintf(of, "\n<TR");
 	   	if (j%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
 	   	fprintf(of, ">");
 	   	fprintf(of, "<TD>%d</TD>", j+1);
 	   	fprintf(of, "<TD ALIGN=\"left\">%s</TD>", ccdb[n][i][CAT_PREN]);
        makeHexlink(p);
 	   	fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"jucatori/%s.html\">%s</A></TD>",
 	       pname[p], ppren[p], hexlink, ccdb[n][i][CAT_NAME]);
 	   	fprintf(of, "<TD sorttable_customkey=\"%d\">%s</TD>", NumericDOB(ccdb[n][i][CAT_DOB], DOB_YYYYMMDD), ccdb[n][i][CAT_DOB]);
 	   	fprintf(of, "<TD>%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", ccdb[n][i][CAT_NAT], ccdb[n][i][CAT_NAT]);
 	   	int xt = atoi(ccdb[n][i][CAT_CLUB]);
 	   	fprintf(of, "<TD>%s</TD>", NickOf(L, xt, year));
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_NR]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_POST]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_CAPS]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_MIN]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_START]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_SUB]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_GOALS]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_PK]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_OWN]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_GREC]);
 	   	fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_RED]);
	    fprintf(of, "</TR>\n");
			j++;
		}
  	i++;
  }
  fprintf(of, "</BODY>\n</HTML>");
	fclose(of);
}

void CupStats(int year, int t) {
  int sm, st, sr, sg, sn, sp, sa, sh, scr, xt;
  int n = year - KFY;
  int i = 0;

  while (i<MAX_CKDB && ckdb[n][i]!=NULL) {
    int p = FindMnem(ckdb[n][i][CAT_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ckdb[n][i][CAT_MNEM]);
      i++;
      continue;
    }
    xt = -2999;
    if (ckdb[n][i][CAT_CLUB]) xt = atoi(ckdb[n][i][CAT_CLUB]);

    sm = st = sr = sg = 0;
    sn = sp = sa = sh = 0;
    scr = 0;
    if (ckdb[n][i][CAT_CAPS])   sm = atoi(ckdb[n][i][CAT_CAPS]);
    if (ckdb[n][i][CAT_START])  st = atoi(ckdb[n][i][CAT_START]);
    if (ckdb[n][i][CAT_SUB])    sr = atoi(ckdb[n][i][CAT_SUB]);
    if (ckdb[n][i][CAT_GOALS])  sg = atoi(ckdb[n][i][CAT_GOALS]);
    if (ckdb[n][i][CAT_MIN])    sn = atoi(ckdb[n][i][CAT_MIN]);
    if (ckdb[n][i][CAT_PK])     sp = atoi(ckdb[n][i][CAT_PK]);
    if (ckdb[n][i][CAT_OWN])    sa = atoi(ckdb[n][i][CAT_OWN]);
    if (ckdb[n][i][CAT_GREC])   sh = atoi(ckdb[n][i][CAT_GREC]);
    if (ckdb[n][i][CAT_RED])   scr = atoi(ckdb[n][i][CAT_RED]);

      pmeci[p] += sm;
      ptit[p]  += st;
      prez[p]  += sr;
      pgol[p]  += sg;
      pmin[p]  += sn;
      ppen[p]  += sp;
      pown[p]  += sa;
      prec[p]  += sh;
      pred[p]  += scr;

    if (sm>0) {
      if (year!=pfy[p] && year!=ply[p]) psez[p]++;
      if (pfy[p]==0) pfy[p] = year;
      if (year<pfy[p]) pfy[p] = year;
      if (year>ply[p]) ply[p] = year;
    }
    i++;
  }
}

void EuroStats(int year, int t) {
  int sm, st, sr, sg, xt;
  int sn, sp, sa, sh;
  int scr;
  int n = year - EFY;
  int i = 0;

  while (i<MAX_CEDB && cedb[n][i]!=NULL) {
    int p = FindMnem(cedb[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", cedb[n][i][TD_MNEM]);
      i++;
      continue;
    }
    xt = -3999;
    if (cedb[n][i][TD_CLUBID]) xt = atoi(cedb[n][i][TD_CLUBID]);
    if (xt>=0 && xt<NC) ecc[xt] = 1;
//    if (t>=0 && xt!=t) {
//      i++;
//      continue;
//    }

    sm = st = sr = sg = 0;
    sn = sp = sa = sh = 0;
    scr = 0;
    if (cedb[n][i][CAT_CAPS])   sm = atoi(cedb[n][i][CAT_CAPS]);
    if (cedb[n][i][CAT_START])  st = atoi(cedb[n][i][CAT_START]);
    if (cedb[n][i][CAT_SUB])    sr = atoi(cedb[n][i][CAT_SUB]);
    if (cedb[n][i][CAT_GOALS])  sg = atoi(cedb[n][i][CAT_GOALS]);
    if (cedb[n][i][CAT_MIN])    sn = atoi(cedb[n][i][CAT_MIN]);
    if (cedb[n][i][CAT_PK])     sp = atoi(cedb[n][i][CAT_PK]);
    if (cedb[n][i][CAT_OWN])    sa = atoi(cedb[n][i][CAT_OWN]);
    if (cedb[n][i][CAT_GREC])   sh = atoi(cedb[n][i][CAT_GREC]);
    if (cedb[n][i][CAT_RED])   scr = atoi(cedb[n][i][CAT_RED]);

      pmeci[p] += sm;
      ptit[p]  += st;
      prez[p]  += sr;
      pgol[p]  += sg;
      pmin[p]  += sn;
      ppen[p]  += sp;
      pown[p]  += sa;
      prec[p]  += sh;
      pred[p]  += scr;

    if (sm>0) {
      if (year!=pfy[p] && year!=ply[p]) psez[p]++;
      if (pfy[p]==0) pfy[p] = year;
      if (year<pfy[p]) pfy[p] = year;
      if (year>ply[p]) ply[p] = year;
    }

    i++;
  }
}

void NatStats(int year, int t) {
  int sm, st, sr, sg, xt;
  int sn, sp, sa, sh;
  int scr;
  int n = year - NFY;
  int i = 0;

  while (i<MAX_CNDB && cndb[n][i]!=NULL) {
    int p = FindMnem(cndb[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", cndb[n][i][TD_MNEM]);
      i++;
      continue;
    }

    sm = st = sr = sg = 0;
    sn = sp = sa = sh = 0;
    if (cndb[n][i][CAT_CAPS])   sm = atoi(cndb[n][i][CAT_CAPS]);
    if (cndb[n][i][CAT_START])  st = atoi(cndb[n][i][CAT_START]);
    if (cndb[n][i][CAT_SUB])    sr = atoi(cndb[n][i][CAT_SUB]);
    if (cndb[n][i][CAT_GOALS])  sg = atoi(cndb[n][i][CAT_GOALS]);
    if (cndb[n][i][CAT_MIN])    sn = atoi(cndb[n][i][CAT_MIN]);
    if (cndb[n][i][CAT_PK])     sp = atoi(cndb[n][i][CAT_PK]);
    if (cndb[n][i][CAT_OWN])    sa = atoi(cndb[n][i][CAT_OWN]);
    if (cndb[n][i][CAT_GREC])   sh = atoi(cndb[n][i][CAT_GREC]);
    if (cndb[n][i][CAT_RED])   scr = atoi(cndb[n][i][CAT_RED]);

      pmeci[p] += sm;
      ptit[p]  += st;
      prez[p]  += sr;
      pgol[p]  += sg;
      pmin[p]  += sn;
      ppen[p]  += sp;
      pown[p]  += sa;
      prec[p]  += sh;
      pred[p]  += scr;

    if (year!=pfy[p] && year!=ply[p]) psez[p]++;
    if (pfy[p]==0) pfy[p] = year;
    if (year<pfy[p]) pfy[p] = year;
    if (year>ply[p]) ply[p] = year;
    if (sm>0) echn[p] = 1;

    i++;

  }
}

void Lineups(int year) {
	char ssn[64];
	SeasonName(year, ssn);
  int n = year - CFY;
  int i = 0;

  fprintf(of, "<HTML>\n<TITLE>Loturi Divizia A %s</TITLE>\n", ssn);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  DatatablesHeader();
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  HTMLLineupsBox(year);
  HTMLLineupsHeader();

  while (i<MAX_CDB && ccdb[n][i]!=NULL) {
    int p = FindMnem(ccdb[n][i][CAT_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ccdb[n][i][CAT_MNEM]);
      i++;
      continue;
    }

    fprintf(of, "\n<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD>%d</TD>", i+1);
    fprintf(of, "<TD ALIGN=\"left\">%s</TD>", ccdb[n][i][CAT_PREN]);
    makeHexlink(p);
    fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"jucatori/%s.html\">%s</A></TD>",
        pname[p], ppren[p], hexlink, ccdb[n][i][CAT_NAME]);
    fprintf(of, "<TD sorttable_customkey=\"%d\">%s</TD>", NumericDOB(ccdb[n][i][CAT_DOB], DOB_YYYYMMDD), ccdb[n][i][CAT_DOB]);
    fprintf(of, "<TD>%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", ccdb[n][i][CAT_NAT], ccdb[n][i][CAT_NAT]);
    int xt = atoi(ccdb[n][i][CAT_CLUB]);
    fprintf(of, "<TD>%s</TD>", NickOf(L, xt, year));
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_NR]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_POST]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_CAPS]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_MIN]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_START]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_SUB]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_GOALS]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_PK]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_OWN]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_GREC]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ccdb[n][i][CAT_RED]);
    fprintf(of, "</TR>\n");

    i++;
  }

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
}

void InsertBlankRow(int row, int nrows) {
  for (int r=nrows; r>row; r--) {
    for (int j=0; j<NUM_PTD; j++) {
      strcpy(pdb[r][j], pdb[r-1][j]);
    }
  }
    for (int j=0; j<NUM_PTD; j++) {
      strcpy(pdb[row][j], " ");
    }
}

void SwapRows(int r1, int r2) {
  char aux[MAX_PSZ];
  for (int j=0; j<NUM_PTD; j++) {
    strcpy(aux, pdb[r1][j]);
    strcpy(pdb[r1][j], pdb[r2][j]);
    strcpy(pdb[r2][j], aux);
  }
  int ea = echid[r1]; 
  echid[r1] = echid[r2];
  echid[r2] = ea;
  int sa = sezid[r1];
  sezid[r1] = sezid[r2];
  sezid[r2] = sa;
}

void PlaceRow(int i, int n) {
  int loc, si, s1, s2;
  for (int i=n-1; i>1; i--) {
    loc = i;
    if (loc>0) s1 = sezid[loc-1]; else s1 = -1;
    if (loc<n-1) s2 = sezid[loc+1]; else s2 = 10000;
    si = sezid[i];
    if (si>s2) {
      do {
        SwapRows(loc, loc+1);
        s1 = s2;
        loc++;
        if (loc<n-1) s2 = sezid[loc+1]; else s2 = 10000;
      } while (si>s2);
    }
    else if (si<s1) {
      do {
        SwapRows(loc, loc-1);
        s2 = s1;
        loc--;
        if (loc>0) s1 = sezid[loc-1]; else s1 = -1;
      } while (si<s1);
    }
  }
}

void InsertSort(int n) {

}

void printBGColor(int y, int k, int altc) {
  fprintf(of, "<TR");
	if (k<ngm[y][COMP_LIGA]) {
		if (altc%2==1) fprintf(of, " BGCOLOR=\"FFFFFF\"");
		else fprintf(of, " BGCOLOR=\"DDFFFF\"");
	}
	else if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]) {
		fprintf(of, " BGCOLOR=\"FFE4C4\"");
	}
	else if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]+ngm[y][COMP_EURO]) {
		fprintf(of, " BGCOLOR=\"97C8ED\"");
	}
	else {
		fprintf(of, " BGCOLOR=\"FFDDDD\"");
	}
  fprintf(of, ">");
}

void HTMLTeamRankingTable(Ranking *tr) {
  tr->bubbleSort(RULE_PTS);
  fprintf(of, "<script src=\"../../sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" width=\"75%%\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"5%%\">#</TH>");
  fprintf(of, "<TH WIDTH=\"35%%\">Club</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Meciuri</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Goluri</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Victorii</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Egaluri</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Înfr.</TH>");
  fprintf(of, "<TH WIDTH=\"15%%\" COLSPAN=\"3\">Golaveraj</TH>");
//  fprintf(of, "<TH WIDTH=\"1%%\"></TH>");
//  fprintf(of, "<TH WIDTH=\"4%%\">Gol-</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\">Puncte</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\">Procentaj%%</TH>");
  fprintf(of, "</TR></THEAD>\n");
  for (int i=0; i<NC; ++i) {
    int t = tr->rank[i];
    Stat s = tr->S[t];
    int ng = s.numg();
    if (ng > 0) {
      fprintf(of, "\n<TR");
      if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
      fprintf(of, ">");
      fprintf(of, "<TD>%d</TD>", i+1);
      fprintf(of, "<TD>%s</TD>", NameOf(L, t, 3000));
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.promo);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.win);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.drw);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.los);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gsc);
      fprintf(of, "<TD>-</TD>");
      fprintf(of, "<TD ALIGN=\"left\">%d</TD>", s.gre);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*s.win+s.drw);
      fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int)(100*s.pct()));
      fprintf(of, "</TR>\n");
    }
  }
  fprintf(of, "</TABLE>\n");
}

void PlayerStats(int pl) {
  int sm, sn, st, sr, sg, sh, xt;
  int ec, en, em, et, er, eg, eh;
  int tkm, tkn, tkt, tkr, tkg;
  int tem, ten, tet, ter, teg;
  int tnm, tnn, tnt, tnr, tng;
  int n, nrow, lasty;
  char ssn[32];
  char wsm[12];

  R->reset();
  Opp->reset();
  nrow = 0;
  makeHexlink(pl);
  fprintf(stderr, "%06d [%s] [%s]: %s %s.\n", pl, pmnem[pl], hexlink, ppren[pl], pname[pl]);
  sprintf(ofilename, "html/jucatori/%s.html", hexlink);
  of = fopen(ofilename, "wt");
  if (of==NULL) {
    fprintf(stderr, "ERROR: cannot write to file %s.\n", ofilename);
    return;
  }

  fprintf(of, "<HTML>\n<TITLE>%s %s</TITLE>\n", ppren[pl], pname[pl]);
  fprintf(of, "<HEAD>\n<link href=\"../css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<script src=\"../sorttable.js\"></script>\n");
  fprintf(of, "<TABLE CELLSPACING=\"10\" CELLPADDING=\"5\">\n<TR>\n<TD>");
  fprintf(of, "<H3><IMG SRC=\"../../../thumbs/22/3/%s.png\"></IMG> %s %s</H3>\n", pcty[pl], ppren[pl], pname[pl]);
  fprintf(of, "<UL><LI>Data naºterii: %s </LI>\n", pdob[pl]);
  fprintf(of, "<LI>Locul naºterii: %s\n", ppob[pl]);
  if (pjud[pl]!=NULL && pjud[pl][0]!=0) {
      fprintf(of, " (%s)", pjud[pl]);
  }
  fprintf(of, "</LI>\n");
	int ncaps = clist[pl][0];
	int debyear = CFY;
	int retyear = CLY;
	int debmid, debssn, debc, debnum;
	if (ncaps>0) {
		int k = 0;
		do {
			k++;
			debmid = clist[pl][k];
			debssn = debmid/1000;
			debnum = debmid%1000;
		} while (k<=ncaps && debnum>=ngm[debssn][COMP_LIGA]);
		if (k<=ncaps) {
			debyear = debssn + FY;
			debc = debyear - CFY;
			retyear = clist[pl][ncaps]/1000 + FY;
			char sdate[32];
			strcpy(sdate, lcdb[debc][debnum][DB_DATE]);
			strtok(sdate, "@");
			int hid = atoi(lcdb[debc][debnum][DB_HOME]);
			int aid = atoi(lcdb[debc][debnum][DB_AWAY]);
			int scr = atoi(lcdb[debc][debnum][DB_SCORE]);
		  fprintf(of, "<LI>Debut în prima divizie  #%d<BR>data: %s<BR>meciul: <A HREF=\"../reports/%d/%d-%d.html\">%s-%s %d-%d</A></LI>\n",
				debord[pl]+1, sdate, debyear, hid, aid, NickOf(L, hid, debyear), NickOf(L, aid, debyear), scr/100, scr%100);
		}
	}
  fprintf(of, "</UL>\n");
  websafeMnem(pmnem[pl], wsm);
  fprintf(of, "<TD><IMG src=\"foto/%s.jpg\" HEIGHT=\"150\" ALT=\"Foto\" VALIGN=\"top\"></IMG></TD>\n", wsm);
  fprintf(of, "</TR></TABLE>\n");
  fprintf(of, "Prezenþe la cluburi româneºti: <BR><BR>\n");

  fprintf(of, "<TABLE WIDTH=\"99%%\" cellpadding=\"2\" RULES=\"groups\" frame=\"box\">\n");  
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"FFFFFF\">\n");
  fprintf(of, "<TH COLSPAN=\"3\"></TH>\n");
  fprintf(of, "<TH COLSPAN=\"3\">Prima Divizie</TH>\n");
  fprintf(of, "<TH COLSPAN=\"3\">Cupa României</TH>\n");
  fprintf(of, "<TH COLSPAN=\"3\">Cupele Europene</TH>\n");
  fprintf(of, "</TR>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"9%%\"> Sezon </TH>");
  fprintf(of, "<TH WIDTH=\"20%%\"> Echipã </TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> P</TH>");

  fprintf(of, "<TH WIDTH=\"6%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> G</TH>");
  fprintf(of, "<TH WIDTH=\"8%%\"> min</TH>");

  fprintf(of, "<TH WIDTH=\"6%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> G</TH>");
  fprintf(of, "<TH WIDTH=\"8%%\"> min</TH>");

  fprintf(of, "<TH WIDTH=\"12%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> G</TH>");
  fprintf(of, "<TH WIDTH=\"8%%\"> min</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n"); 

  lasty = -1;

  for (int year=debyear; year<=retyear; year++) {

    SeasonName(year, ssn);
    n = year - CFY;
    int i = 0;

    while (i<MAX_CDB && ccdb[n][i]!=NULL) {
      int p = FindMnem(ccdb[n][i][CAT_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ccdb[n][i][CAT_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }

      /* umple golul */
      if (lasty>0 && year-lasty>1) {
        for (int yy=lasty+1; yy<year; yy++) {
          char ssnyy[12];
          SeasonName(yy, ssnyy);
          sezid[nrow] = year;
          strcpy(pdb[nrow][PTD_SEZON], ssnyy);
          strcpy(pdb[nrow][PTD_ECHIPA], " ");
          strcpy(pdb[nrow][PTD_POST], " ");
          strcpy(pdb[nrow][PTD_DIVM], " ");
          strcpy(pdb[nrow][PTD_DIVG], " ");
          strcpy(pdb[nrow][PTD_DIVN], " ");
          for (int j=PTD_CUPM; j<NUM_PTD; j++) strcpy(pdb[nrow][j], " ");
          nrow++;
        }
      }

      xt = -10999;
      if (ccdb[n][i][CAT_CLUB]) xt = atoi(ccdb[n][i][CAT_CLUB]);

      sm = sn = st = sr = sg = 0;
      if (ccdb[n][i][CAT_CAPS])  sm = atoi(ccdb[n][i][CAT_CAPS]);
      if (ccdb[n][i][CAT_GOALS]) sg = atoi(ccdb[n][i][CAT_GOALS]);
      if (ccdb[n][i][CAT_GREC])  sh = atoi(ccdb[n][i][CAT_GREC]);
      if (ccdb[n][i][CAT_MIN])   sn = atoi(ccdb[n][i][CAT_MIN]);

      if (year !=1957) {
        pmeci[p] += sm;
        pmin[p]  += sn;
        pgol[p]  += (sh<0 ? sh : sg);
      }

      if (sg>0) {
        if (year!=pfy[p] && year!=ply[p]) psez[p]++;
        if (pfy[p]==0) pfy[p] = year;
        if (year<pfy[p]) pfy[p] = year;
        if (year>ply[p]) ply[p] = year;
      }

      echid[nrow] = xt;
      sezid[nrow] = year;
      strcpy(pdb[nrow][PTD_SEZON], ssn);
      strcpy(pdb[nrow][PTD_ECHIPA], NickOf(L, xt, year));
      strcpy(pdb[nrow][PTD_POST], ccdb[n][i][CAT_POST]);
      sprintf(pdb[nrow][PTD_DIVM], "%d", sm);
      sprintf(pdb[nrow][PTD_DIVN], "%d", sn);
      sprintf(pdb[nrow][PTD_DIVG], "%d", (sh<0? sh : sg));
      for (int j=PTD_CUPM; j<NUM_PTD; j++) sprintf(pdb[nrow][j], " ");
      nrow++;
      i++;
      lasty = year;
    }
  }

  tkm = tkn = tkt = tkr = tkg = 0;
  tem = ten = tet = ter = teg = 0;
  tnm = tnn = tnt = tnr = tng = 0;

  // potriveste meciurile in cupa romaniei

  for (int year=KFY; year<=KLY; year++) {
    n = year - KFY;
    int i = 0;
    SeasonName(year, ssn);

    while (i<MAX_CKDB && ckdb[n][i]!=NULL) {
      int p = FindMnem(ckdb[n][i][CAT_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ckdb[n][i][CAT_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }

      xt = -10998;
      if (ckdb[n][i][CAT_CLUB]) xt = atoi(ckdb[n][i][CAT_CLUB]);

      em = en = et = er = eg = eh = 0;
      if (ckdb[n][i][CAT_CAPS])   em = atoi(ckdb[n][i][CAT_CAPS]);
      if (ckdb[n][i][CAT_GOALS])  eg = atoi(ckdb[n][i][CAT_GOALS]);
      if (ckdb[n][i][CAT_GREC])   eh = atoi(ckdb[n][i][CAT_GREC]);
      if (ckdb[n][i][CAT_MIN])    en = atoi(ckdb[n][i][CAT_MIN]);

      int krow = nrow;
      int py;
      for (int j=0; j<nrow; j++) {
        if (strcmp(ssn, pdb[j][PTD_SEZON])==0) {
          if ((strcmp(pdb[j][PTD_ECHIPA], NickOf(L, xt, year))==0) ||
              (strcmp(pdb[j][PTD_ECHIPA], " ")==0))
          {
            krow = j;
          }
        }
      }

      if (krow>=0 && krow<=nrow && em>0) {
        if (krow==nrow) {
          strcpy(pdb[krow][PTD_SEZON], ssn);
          strcpy(pdb[krow][PTD_ECHIPA], NickOf(L, xt, year));
          for (int j=PTD_POST; j<NUM_PTD; j++) strcpy(pdb[krow][j], " ");
          echid[krow] = xt;
          sezid[krow] = year;
          nrow++;
        }
        if (pdb[krow][PTD_ECHIPA][0]==' ') 
          strcpy(pdb[krow][PTD_ECHIPA], NickOf(L, xt, year));
        sprintf(pdb[krow][PTD_CUPM], "%d", em);
        sprintf(pdb[krow][PTD_CUPN], "%d", en);
        sprintf(pdb[krow][PTD_CUPG], "%d", (eh<0? eh : eg));
      }

      if (krow==nrow-1) PlaceRow(krow, nrow);

      tkm += em; tkn += en; tkt += et; tkr += er;
      tkg += (eh<0? eh : eg);
      i++;
    }
  }

  // potriveste meciurile in cupele europene

  for (int year=EFY; year<=ELY; year++) {
    n = year - EFY;
    int i = 0;
    SeasonName(year, ssn);

    em = et = er = eg = eh = en = 0;
    while (i<MAX_CEDB && cedb[n][i]!=NULL) {
      int p = FindMnem(cedb[n][i][TD_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", cedb[n][i][TD_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }

      xt = -14999;
      if (cedb[n][i][TD_CLUBID]) xt = atoi(cedb[n][i][TD_CLUBID]);
      ec = -1;
      if (cedb[n][i][TD_NR])      ec = atoi(cedb[n][i][TD_NR]);
      if (cedb[n][i][CAT_CAPS])   em += atoi(cedb[n][i][CAT_CAPS]);
      if (cedb[n][i][CAT_GOALS])  eg += atoi(cedb[n][i][CAT_GOALS]);
      if (cedb[n][i][CAT_GREC])   eh += atoi(cedb[n][i][CAT_GREC]);
      if (cedb[n][i][CAT_MIN])    en += atoi(cedb[n][i][CAT_MIN]);

      i++;
    }

    if (em<=0) continue;

    int erow = nrow;
    int py;
    for (int j=0; j<nrow; j++) {
      if (strcmp(ssn, pdb[j][PTD_SEZON])==0) {
        if ((strcmp(pdb[j][PTD_ECHIPA], NickOf(L, xt, year))==0) ||
            (strcmp(pdb[j][PTD_ECHIPA], " ")==0))
        {
          erow = j;
        }
      }
    }

    if (erow>=0 && erow<=nrow && em>0) {
      if (erow==nrow) {
        sezid[erow] = year;
        echid[erow] = xt;
        strcpy(pdb[erow][PTD_SEZON], ssn);
        strcpy(pdb[erow][PTD_ECHIPA], NickOf(L, xt, year));
        for (int j=PTD_POST; j<NUM_PTD; j++) strcpy(pdb[erow][j], " ");
        nrow++;
      }
      if (pdb[erow][PTD_ECHIPA][0]==' ')
        strcpy(pdb[erow][PTD_ECHIPA], NickOf(L, xt, year));
      sprintf(pdb[erow][PTD_EURM], "%s %2d", ECname(ec, year), em);
      sprintf(pdb[erow][PTD_EURN], "%d", en);
      sprintf(pdb[erow][PTD_EURG], "%d", (eh<0? eh : eg));
    }

    if (erow==nrow-1) PlaceRow(erow, nrow);

    tem += em; tet += et; ter += er; ten += en;
    teg += (eh<0? eh : eg);
  }

  // ghiceste ordinea cronologica
  char a1, a2, a3;
  char aux[MAX_PTDLEN+1];
  for (int i=0; i<nrow-2; i++) {
    if (echid[i]!=echid[i+1] && echid[i]==echid[i+2]) {
/*
      a1 = pdb[i][PTD_SEZON][strlen(pdb[i][PTD_SEZON])-1];
      a2 = pdb[i+1][PTD_SEZON][strlen(pdb[i+1][PTD_SEZON])-1];
      a3 = pdb[i+2][PTD_SEZON][strlen(pdb[i+2][PTD_SEZON])-1];
*/
      a1 = sezid[i];
      a2 = sezid[i+1];
      a3 = sezid[i+2];
      if (a1==a2 && a2==a3-1) {
        SwapRows(i, i+1);
      }
      if (a1==a2-1 && a2==a3) {
        SwapRows(i+1, i+2);
      }
    }
  }


	/* Tabela pentru jucator la club */
  char bgc[12], rbgc[12];
  for (int i=0; i<nrow; i++) {
      fprintf(of, "<TR");
      strcpy(bgc, "FFFFFF"); strcpy(rbgc, "DDFFFF");
      if (i%2==1) fprintf(of, " BGCOLOR=\"%s\"", rbgc);
      fprintf(of, ">");
      fprintf(of, "<TD ALIGN=\"center\"><a href=\"#%d\"><font color=\"000000\">%s</font></a></TD>", Season(pdb[i][PTD_SEZON]), pdb[i][PTD_SEZON]);
      fprintf(of, "<TD ALIGN=\"left\">  %s</TD>", pdb[i][PTD_ECHIPA]);
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", pdb[i][PTD_POST]);

      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVM]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVG]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVN]);

      if (i%2==0) { strcpy(bgc, "DDFFFF"), strcpy(rbgc, "FFFFFF"); }
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPM]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPG]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPN]);

      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURM]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURG]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURN]);

      fprintf(of, "</TR>\n");
  }

  fprintf(of, "<TFOOT>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TD>Total</TD>");
  fprintf(of, "<TD></TD>");
  fprintf(of, "<TD></TD>");
  if (pmeci[pl]>0) {
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", pmeci[pl]);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", pgol[pl]);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", pmin[pl]);
  }
  else fprintf(of, "<TD/><TD/><TD/>");

  if (tkm>0) {
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkm);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkg);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkn);
  }
  else fprintf(of, "<TD/><TD/><TD/>");

  if (tem>0) {
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", tem);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", teg);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", ten);
  }
  else fprintf(of, "<TD/><TD/><TD/>");

  fprintf(of, "</TR>\n");
  fprintf(of, "\n</TABLE>\n");
  fprintf(of, "<BR>Total la echipele de club: <B>%d</B> meciuri, <B>%d</B> goluri\n",
		pmeci[pl]+tkm+tem, pgol[pl]+tkg+teg);

  if (echn[pl]>0) {
//  fprintf(of, "<BR><BR>La echipa naþionalã:<BR><BR>\n");
  fprintf(of, "<BR><BR>\n");

  fprintf(of, "<TABLE WIDTH=\"60%%\" cellpadding=\"2\" RULES=\"groups\" frame=\"box\">\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"FFFFFF\">\n");
  fprintf(of, "<TH COLSPAN=\"7\">Echipa Naþionalã</TH>\n");
  fprintf(of, "</TR>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"15%%\"> Sezon </TH>");
  fprintf(of, "<TH WIDTH=\"35%%\"> Club </TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> G</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> min</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> T</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> R</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n");

  int ntb = 0;
  for (int year=NFY; year<=NLY; year++) {
    n = year - NFY;
    int i = 0;

    em = et = er = eg = eh = 0;
    while (i<MAX_CNDB && cndb[n][i]!=NULL) {
      int p = FindMnem(cndb[n][i][TD_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", cndb[n][i][TD_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }

      SeasonName(year, ssn);
      fprintf(of, "<TR");
      if (ntb%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
      fprintf(of, ">");
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ssn);
      fprintf(of, "<TD ALIGN=\"left\">%s</TD>",   cndb[n][i][CAT_CLUB]);
//      fprintf(of, "<TD></TD>");

      if (cndb[n][i][CAT_CAPS])  em = atoi(cndb[n][i][CAT_CAPS]);
      if (cndb[n][i][CAT_MIN])   en = atoi(cndb[n][i][CAT_MIN]);
      if (cndb[n][i][CAT_START]) et = atoi(cndb[n][i][CAT_START]);
      if (cndb[n][i][CAT_SUB])   er = atoi(cndb[n][i][CAT_SUB]);
      if (cndb[n][i][CAT_GOALS]) eg = atoi(cndb[n][i][CAT_GOALS]);
      if (cndb[n][i][CAT_GREC])  eh = atoi(cndb[n][i][CAT_GREC]);

      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  cndb[n][i][CAT_CAPS]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>",  eh<0? eh : eg);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  cndb[n][i][CAT_MIN]);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  cndb[n][i][CAT_START]);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  cndb[n][i][CAT_SUB]);
      fprintf(of, "</TR>\n");
      ntb++;

      tnm += em; tnn += en; tnt += et; tnr += er;
      tng += (eh<0? eh : eg);
      i++;
    }
  }

  fprintf(of, "<TFOOT>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TD ALIGN=\"center\" COLSPAN=\"2\">Total</TD>");

  if (tnm>0) {
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnm);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tng);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnn);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnt);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnr);
  }
  else fprintf(of, "<TD/><TD/><TD/><TD/>");
  fprintf(of, "</TR>\n");
  fprintf(of, "\n</TABLE>\n");

  }

  fprintf(of, "<HR>\n");
  fprintf(of, "<H3>Lista meciurilor</H3>\n");
  fprintf(of, "<TABLE WIDTH=\"85%%\" cellpadding=\"1\" RULES=\"groups\" frame=\"box\">\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"5\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"FFFFFF\">\n");
  fprintf(of, "<TH COLSPAN=\"5\">Info</TH>\n");
  fprintf(of, "<TH COLSPAN=\"2\">Meci</TH>\n");
  fprintf(of, "<TH COLSPAN=\"3\">Sezon</TH>\n");
  fprintf(of, "<TH COLSPAN=\"3\">Carierã</TH>\n");
  fprintf(of, "</TR>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"18%%\"> Data@Ora </TH>");
  fprintf(of, "<TH WIDTH=\"17%%\"> Gazde </TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> Scor </TH>");
  fprintf(of, "<TH WIDTH=\"17%%\"> Oaspeþi </TH>");
  fprintf(of, "<TH WIDTH=\"3%%\"> Et </TH>");

  fprintf(of, "<TH WIDTH=\"4%%\"> g</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> min</TH>");

  fprintf(of, "<TH WIDTH=\"4%%\"> j</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> g</TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> min</TH>");

  fprintf(of, "<TH WIDTH=\"4%%\"> j</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> g</TH>");
  fprintf(of, "<TH WIDTH=\"8%%\"> min</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n");

	int scaps = 0;
	int smins = 0;
	int sgols = 0;
	int ccaps = 0;
	int cmins = 0;
	int cgols = 0;
	int lsez = -1;
	int altc = 0;
    int firstrow = 1;
  for (int i=0; i<ncaps; i++) {
			int mid = clist[pl][i+1];
			int y = mid/1000;
			int year = FY+y;
			int k = mid%1000;
			if (y!=lsez) {
				if (lsez>=0) fprintf(of, "</TBODY>");
				fprintf(of, "<TBODY>");
				altc++;
				scaps = 0; smins = 0; sgols = 0;
				lsez = y;
                firstrow = 1;
			}
            else {
                firstrow = 0;
            }
			printBGColor(y, k, altc);
			int ny  = GetYear(ladb[y][k][DB_DATE]);
			int hid = atoi(ladb[y][k][DB_HOME]);
			int aid = atoi(ladb[y][k][DB_AWAY]);
			int scr = atoi(ladb[y][k][DB_SCORE]);
			int ecl = ladb[y][k][DB_COMP][0] - 48;
			int rnd = atoi(ladb[y][k][DB_ROUND]);
			int haw = mlist[pl][i+1]/10000;
			int wxl = (mlist[pl][i+1]/1000)%10;
			int min = mlist[pl][i+1]%1000;
			int gls = elist[pl][i+1];
            int shx = scr/100;
            int sgx = scr%100;
      if (firstrow) {
          fprintf(of, "<TD ID=\"%d\" ALIGN=\"left\">%s</TD>", year, ladb[y][k][DB_DATE]);
      } else {
          fprintf(of, "<TD ALIGN=\"left\">%s</TD>", ladb[y][k][DB_DATE]);
      }

      if (!haw) {
        if (hid<1000) { R->S[hid].addRes(shx, sgx); R->S[hid].promo += gls; }
        if (aid<1000) { Opp->S[aid].addRes(shx, sgx); Opp->S[aid].promo += gls; }
      } else {
        if (aid<1000) { R->S[aid].addRes(sgx, shx); R->S[aid].promo += gls; }
        if (hid<1000) { Opp->S[hid].addRes(sgx, shx); Opp->S[hid].promo += gls; }
      }

			if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]+ngm[y][COMP_EURO])
	      fprintf(of, "<TD ALIGN=\"left\">%s%s%s</TD>", (haw?"":"<I>"), NickOf(L, hid, year), (haw?"":"</I>"));
			else
  	    fprintf(of, "<TD ALIGN=\"left\">%s%s%s</TD>", (haw?"":"<I>"), NickOf(CL, hid/1000, year), (haw?"":"</I>"));

			if (k<ngm[y][COMP_LIGA]) {
	      fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../reports/%d/%d-%d.html\">%d-%d</A></TD>", 
					fxcol[wxl], year, hid, aid, shx, sgx);
			}
			else if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]) {
	      fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../reports/%d/c%d-%d.html\">%d-%d</A></TD>", 
					fxcol[wxl], year, hid, aid, shx, sgx);
			}
			else if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]+ngm[y][COMP_EURO]) {
	      fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../reports/%d/e%d-%d.html\">%d-%d</A></TD>", 
					fxcol[wxl], year, hid, aid, shx, sgx);
			}
			else {
	      fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../reports/%d/n%d-%d.html\">%d-%d</A></TD>", 
					fxcol[wxl], ny, hid, aid, shx, sgx);
			}

			if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]+ngm[y][COMP_EURO])
	      fprintf(of, "<TD ALIGN=\"left\">%s%s%s</TD>", (haw?"<I>":""), NickOf(L, aid, year), (haw?"</I>":""));
			else
	      fprintf(of, "<TD ALIGN=\"left\">%s%s%s</TD>", (haw?"<I>":""), NickOf(CL, aid/1000, year), (haw?"</I>":""));

			if (k<ngm[y][COMP_LIGA])
	      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", rnd);
			else if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]) {
	      fprintf(of, "<TD ALIGN=\"right\">%c</TD>", cupround[rnd]);
			}
			else if (k<ngm[y][COMP_LIGA]+ngm[y][COMP_CUPA]+ngm[y][COMP_EURO]) {
	      fprintf(of, "<TD ALIGN=\"right\">%c</TD>", ecln[ecl]);
			}
			else {
	      fprintf(of, "<TD></TD>");
			}

			scaps++;
			smins += min;
			sgols += gls;
			ccaps++;
			cmins += min;
			cgols += gls;

			if (gls>0) { fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gls);	} else { fprintf(of, "<TD></TD>"); }
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", min);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", scaps);
			if (sgols>0) { fprintf(of, "<TD ALIGN=\"right\">%d</TD>", sgols);	} else { fprintf(of, "<TD></TD>"); }
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", smins);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ccaps);
			if (cgols>0) { fprintf(of, "<TD ALIGN=\"right\">%d</TD>", cgols);	} else { fprintf(of, "<TD></TD>"); }
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", cmins);
      fprintf(of, "</TR>\n");
  }
  fprintf(of, "</TBODY></TABLE>\n");

  fprintf(of, "<BR><H3>Total pentru echipã</H3>\n");
  HTMLTeamRankingTable(R);

  fprintf(of, "<BR><H3>Total împotriva echipei</H3>\n");
  HTMLTeamRankingTable(Opp);

  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

void Sort(int cr) {
  int sorted, last;
  for (int i=0; i<NP; i++) rank[i] = i;
  last = NP-1;
  do {
    sorted = 1;
    for (int i=0; i<NP-1; i++) { 
      if (pmeci[rank[i+1]] > pmeci[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
}

void HTMLHeader(int t) {
  if (t==TM_LIGA || t>=NC)
    fprintf(of, "<HTML>\n<TITLE>Loturi Divizia A %d-%d</TITLE>\n", CFY, CLY);
  else if (t==TM_CUPA)
    fprintf(of, "<HTML>\n<TITLE>Prezenþe în Cupa României</TITLE>\n");
  else if (t==TM_EURO)
    fprintf(of, "<HTML>\n<TITLE>Prezenþe în cupele europene</TITLE>\n");
  else if (t==TM_NATL)
    fprintf(of, "<HTML>\n<TITLE>Prezenþe în echipa naþionalã</TITLE>\n");
  else
    fprintf(of, "<HTML>\n<TITLE>Loturi %s în Divizia A %d-%d</TITLE>\n", NameOf(L, t, 3000), CFY, CLY);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  DatatablesHeader();
  fprintf(of, "</HEAD>\n<BODY>\n");
}

void HTMLTable(int t) {
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  if (t==TM_LIGA || t>=NC)
    fprintf(of, "<H3>Jucãtori în campionatul României %d-%d</H3>\n", CFY, CLY);
  else if (t==TM_CUPA)
    fprintf(of, "<H3>Jucãtori în Cupa României %d-%d</H3>\n", KFY, KLY);
  else if (t==TM_EURO)
    fprintf(of, "<H3>Jucãtori ai echipelor româneºti în cupele europene %d-%d</H3>\n", EFY, ELY);
  else if (t==TM_NATL)
    fprintf(of, "<H3>Jucãtori în echipa naþionalã %d-%d</H3>\n", NFY, NLY);
  else
    fprintf(of, "<H3>Jucãtori %s %d-%d</H3>\n", NameOf(L, t, 3000), CFY, CLY);
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

  for (int i=0; i<NP; i++) {
    int x = rank[i];
    if (pmeci[x]==0) continue;
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
 	fprintf(of, "<TD align=\"right\">%d</TD>", prec[x]);
 	fprintf(of, "<TD align=\"right\">%d</TD>", pred[x]);
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
  CFY  = 1933; CLY = 2016;
  KFY  = 1934; KLY = 2016;
  EFY  = 1957; ELY = 2016;
  NFY  = 1922; NLY = 2016;
  FY   = 1922; LY  = 2016;
  tm = -1;
  pl = -1;
  cr = CR_M;

  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-q")==0) verbosity = 0;
    else if (strcmp(argv[k], "-fy")==0 && k<argc-1)  {
      FY = atoi(argv[++k]);
      CFY = KFY = EFY = NFY = FY;
    }
    else if (strcmp(argv[k], "-ly")==0 && k<argc-1)  {
      LY = atoi(argv[++k]);
      CLY = KLY = ELY = NLY = LY;
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
  ccdb = new char***[nums];
  lcdb = new char***[nums];
  vcdb = new char***[nums];
  ladb = new char***[nums];
  vadb = new char***[nums];
  for (int s=0; s<nums; s++) {
     ccdb[s] = new char**[MAX_CDB];
     lcdb[s] = new char**[MAX_LDB];
     vcdb[s] = new char**[MAX_LDB];
     ladb[s] = new char**[MAX_LDB];
     vadb[s] = new char**[MAX_LDB];
  }

  int numks = KLY-KFY+1;
  ckdb = new char***[numks];
  lkdb = new char***[numks];
  vkdb = new char***[numks];
  for (int s=0; s<numks; s++) {
     ckdb[s] = new char**[MAX_CKDB];
     lkdb[s] = new char**[MAX_LKDB];
     vkdb[s] = new char**[MAX_LKDB];
  }

  int numecs = ELY-EFY+1;
  cedb = new char***[numecs];
  ledb = new char***[numecs];
  vedb = new char***[numecs];
  for (int s=0; s<numecs; s++) {
     cedb[s] = new char**[MAX_CEDB];
     ledb[s] = new char**[MAX_LEDB];
     vedb[s] = new char**[MAX_LEDB];
  }

  int numns = NLY-NFY+1;
  cndb = new char***[numns];
  lndb = new char***[numns];
  vndb = new char***[numns];
  for (int s=0; s<numns; s++) {
     cndb[s] = new char**[MAX_CNDB];
     lndb[s] = new char**[MAX_LNDB];
     vndb[s] = new char**[MAX_LNDB];
  }

  pdb = new char**[MAX_PSZ];
  for (int s=0; s<MAX_PSZ; s++) {
    pdb[s] = new char*[NUM_PTD];
    for (int j=0; j<NUM_PTD; j++) pdb[s][j] = new char[MAX_PTDLEN];
  }

  for (year=FY; year<=LY; year++) {
		for (int c=0; c<=NUM_COMPS; ++c) ngm[year-FY][c] = 0;
	}

  for (year=CFY; year<=CLY; year++) {
    CollectCatalog(year, ccdb, COMP_LIGA);
		CollectLineups(year, lcdb, COMP_LIGA);
		CollectEvents (year, vcdb, COMP_LIGA);
  }

  for (year=KFY; year<=KLY; year++) {
		CollectLineups(year, lkdb, COMP_CUPA);
		CollectEvents (year, vkdb, COMP_CUPA);
  }

  for (year=EFY; year<=ELY; year++) {
		CollectLineups(year, ledb, COMP_EURO);
		CollectEvents (year, vedb, COMP_EURO);
  }

  for (year=NFY; year<=NLY; year++) {
		CollectLineups(year, lndb, COMP_NATL);
		CollectEvents (year, vndb, COMP_NATL);
  }

  for (year=FY; year<=LY; year++) {
		MergeDB(year);
		qSortDB(ladb, year);
		ExtractLists(year, ladb, vadb);
	}
	qSortDebut();

  for (year=CFY; year<=CLY; year++) {
     //printf("Team stats %d %d.\n", year, tm);
     TeamStats(year, tm, pl);
  }

  if (FY==LY) {
    sprintf(ofilename, "html/catalog-%d.html", FY);
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Lineups(FY);
    fclose(of);
    return 0;
  }
  else if (tm<NC) {
    sprintf(ofilename, "html/catalog.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Sort(cr);
    HTMLHeader(tm);
    HTMLTable(tm);
    HTMLFooter();

    fclose(of);
  }

//  if (tm>=-1 && tm<NC) fprintf(stderr, "<UL>\n");

  if (tm==-1) for (int t=0; t<NC; t++) if (dva[t]>0) {
    fprintf(stderr, "<LI><A HREF=\"lot-%d.html\">Jucãtori %s</A></LI>\n", t, NameOf(L, t, 3000));
		for (int y=CFY; y<=CLY; y++)  {
            //printf("Yearly team stats %d %d.\n", t, y);
			YearlyTeamStats(t, y);
		}
    ResetStats();
    for (year=CFY; year<=CLY; year++) {
      //printf("Team stats %d %d.\n", year, t);
      TeamStats(year, t, pl);
    } 

    Sort(cr); 

    sprintf(ofilename, "html/lot-%d.html", t);
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      continue;
    }
    HTMLHeader(t);
    HTMLInA(t, -1);
    HTMLTable(t);
    HTMLFooter();

    fclose(of);
  }
//  if (tm>=-1 && tm<NC) fprintf(stderr, "</UL>\n<HR>\n");


  ResetStats();
  for (year=KFY; year<=KLY; year++) {
    CollectCatalog(year, ckdb, COMP_CUPA);
    CupStats(year, TM_CUPA);
  }

  
    sprintf(ofilename, "html/catalog-cupa.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Sort(cr);
    HTMLHeader(TM_CUPA);
    HTMLTable(TM_CUPA);
    HTMLFooter();
  
    fclose(of);


  ResetStats();
  for (year=EFY; year<=ELY; year++) {
    CollectCatalog(year, cedb, COMP_EURO);
    EuroStats(year, TM_EURO);
  }

  
    sprintf(ofilename, "html/catalog-euro.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Sort(cr);
    HTMLHeader(TM_EURO);
    HTMLTable(TM_EURO);
    HTMLFooter();
  
    fclose(of);

  ResetStats();
  for (year=NFY; year<=NLY; year++) {
    CollectCatalog(year, cndb, COMP_NATL);
    NatStats(year, TM_NATL);
  }

  
    sprintf(ofilename, "html/catalog-nat.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Sort(cr);
    HTMLHeader(TM_NATL);
    HTMLTable(TM_NATL);
    HTMLFooter();
  
    fclose(of);


  ResetStats();
  if (pl<0) for (int p=0; p<NP; p++) {
    PlayerStats(p);
  }
  else if (pl<NP) PlayerStats(pl);
 
  return 0;
}

