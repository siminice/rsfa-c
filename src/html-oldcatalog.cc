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

const char* fxcol[] = {"F0F0B0", "AAFFAA", "FF8888"};

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
#define MAX_SEASONS	200
#define MAX_DB		800
#define MAX_EDB		300
#define MAX_KDB		750
#define MAX_NDB		100
#define MAX_LDB		400
#define LDB_COLS	 60
#define MAX_LOG	1000

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
char ****ldb;
char ****vdb;
int  echid[MAX_PSZ];
int  sezid[MAX_PSZ];
int  clist[MAX_NAMES][MAX_LOG];		// caps list for each player, clist[0] = #caps
int  mlist[MAX_NAMES][MAX_LOG];		// minutes in game for each player
int  elist[MAX_NAMES][MAX_LOG];		// event list for each player
int  ord[MAX_SEASONS][MAX_LDB];
int  qord[MAX_NAMES];
int  qd[MAX_NAMES];
int  ngm[MAX_SEASONS];
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
int *rank;

int borna[256];

char ofilename[128];
FILE *of;

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

int ResetStats() {
  for (int i=0; i<NP; i++) {
     psez[i] = pfy[i] = ply[i] = pmeci[i] = ptit[i] = prez[i] = pgol[i] = 0;
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
	if (s[0]==' ') return -1;
	int ssn = 1000*(s[6]-48) + 100*(s[7]-48) + 10*(s[8]-48) + (s[9]-48) - (FY-1);
  int day = 500*(s[3]-48) + 50*(s[4]-48) + 10*(s[0]-48) + (s[1]-48);
	int hrs = 0;
	int min = 0;
	if (l>15) {
		hrs = 10*(s[11]-48) + (s[12]-48) - 8;
		min = 10*(s[14]-48) + (s[15]-48);
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

void qSortDB(int year) {
  int y = year-FY;
  for (int i=0; i<ngm[y]; ++i) {
    qd[i] = NumericDate(ldb[y][i][DB_DATE]);
    qord[i]=i;
  }
  qsort(qord, ngm[y], sizeof(int), compareDate);
  for (int i=0; i<ngm[y]; i++) ord[y][i] = qord[i];
}

void qSortDebut() {
	for (int i=0; i<NP; ++i) { qord[i] = i; qd[i] = qdeb[i]; }
	qsort(qord, NP, sizeof(int), compareDate);
	for (int i=0; i<NP; i++) debord[qord[i]] = i;
}

void CollectLineups(int year) {
  char filename[128];
  char s[4096], *tok[100];

  sprintf(filename, "lineups-%d.db", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: Cannot open file %s.\n", filename); return; }

  int i = 0;
  int y = year - FY;
	ldb[y] = new char**[MAX_LDB];
  do {
    fgets(s, 4096, f);
    if (strlen(s)<10) continue;
		ldb[y][i] = new char*[LDB_COLS];
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
		ngm[y] = i;
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

void ExtractLists(int year) {
	char spi[20], *sp, *sm;
	int min, n, pid;
  int y = year - FY;

  for (int i=0; i<ngm[y]; ++i) {
		int r = ord[y][i];
		int ndt = NumericDate(ldb[y][r][DB_DATE]);
		int wx = wdl(ldb[y][r][DB_SCORE]);
		int wy = revwdl(wx);
		for (int j=DB_ROSTER1; j<DB_ROSTER2+22; j++) if (j!=DB_COACH1) {
			strcpy(spi, ldb[y][r][j]);
			sp = strtok(spi, ":");
			sm = strtok(NULL, ":");
			if (sm!=NULL) min=atoi(sm); else min=0;
			if (min>0) {
				pid = binFindMnem(sp);
				n = clist[pid][0];
				if (pid>=0) {
					if (clist[pid][0]==0) qdeb[pid] = ndt;
					if ((j>=DB_ROSTER1+11 && j<DB_ROSTER1+22) || (j>=DB_ROSTER2+11)) {
						qdeb[pid] += (90-min);
					}
					clist[pid][n+1] = 1000*y+r;
					mlist[pid][n+1] = 1000*(j<DB_ROSTER2?wx:wy)+min;
					clist[pid][0]++;
				}
			}
		}
		for (int j=0; j<LDB_COLS; j++) {
			if (vdb[y][r][j]==NULL || vdb[y][r][j][0]==' ') continue;
			strcpy(spi, vdb[y][r][j]);
			char evt = vdb[y][r][j][6];
			if (evt==39 || evt==34) {
				spi[6] = 0;
				pid = binFindMnem(sp);
				if (pid>=0) {
					elist[pid][clist[pid][0]]++;
				}
			}
		}
  }
}

void CollectEvents(int year) {
  char filename[128];
  char s[4096], *tok[100];

  sprintf(filename, "events-%d.db", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: Cannot open file %s.\n", filename); return; }

  int i = 0;
  int y = year - FY;
	vdb[y] = new char**[MAX_LDB];
  do {
    fgets(s, 4096, f);
    if (strlen(s)<10) continue;
		vdb[y][i] = new char*[LDB_COLS];
    tok[0] = strtok(s, ",\n");
    for (int j=1; j<LDB_COLS; j++) tok[j] = strtok(NULL, ",\n");
    for (int j=0; j<LDB_COLS; j++) {
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

void CollectDiv(int year) {
  char filename[128];
  char s[1000], *tok[100];
  int sm, st, sr, sg;

  sprintf(filename, "catalog-%d.dat", year);
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
    for (int k=1; k<TD_NUM; k++) tok[k] = strtok(NULL, ",\n");

    db[n][i] = new char*[TD_NUM];
    for (int k=0; k<TD_NUM; k++) {
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

void TeamStats(int year, int t, int pl) {
  int sm, st, sr, sg, xt;
  int n = year - FY;
  int i = 0;

  while (i<MAX_DB && db[n][i]!=NULL) {
    int p = FindMnem(db[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", db[n][i][TD_MNEM]);
      i++;
      continue;
    }
    xt = -1999;
    if (db[n][i][TD_CLUBID]) xt = atoi(db[n][i][TD_CLUBID]);
    if (xt>=0 && xt<NC) dva[xt] = 1;
    if (t>=0 && xt!=t) {
      i++;
      continue;
    }
    sm = st = sr = sg = 0;
    if (db[n][i][TD_MECIURI]) sm = atoi(db[n][i][TD_MECIURI]);
    if (db[n][i][TD_TITULAR]) st = atoi(db[n][i][TD_TITULAR]);
    if (db[n][i][TD_REZERVA]) sr = atoi(db[n][i][TD_REZERVA]);
    if (db[n][i][TD_GOLURI])  sg = atoi(db[n][i][TD_GOLURI]);
  
    if (year !=1957) {
      pmeci[p] += sm;
      ptit[p]  += st;
      prez[p]  += sr;
      pgol[p]  += sg;
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

void CupStats(int year, int t) {
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
    xt = -2999;
    if (kdb[n][i][TD_CLUBID]) xt = atoi(kdb[n][i][TD_CLUBID]);
    sm = st = sr = sg = 0;
    if (kdb[n][i][TD_MECIURI]) sm = atoi(kdb[n][i][TD_MECIURI]);
    if (kdb[n][i][TD_TITULAR]) st = atoi(kdb[n][i][TD_TITULAR]);
    if (kdb[n][i][TD_REZERVA]) sr = atoi(kdb[n][i][TD_REZERVA]);
    if (kdb[n][i][TD_GOLURI])  sg = atoi(kdb[n][i][TD_GOLURI]);
  
    pmeci[p] += sm;
    ptit[p]  += st;
    prez[p]  += sr;
    pgol[p]  += sg;

    if (year!=pfy[p] && year!=ply[p]) psez[p]++;
    if (pfy[p]==0) pfy[p] = year;
    if (year<pfy[p]) pfy[p] = year;
    if (year>ply[p]) ply[p] = year;

    i++;
  }
}

void ECStats(int year, int t) {
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
    xt = -3999;
    if (edb[n][i][TD_CLUBID]) xt = atoi(edb[n][i][TD_CLUBID]);
    if (xt>=0 && xt<NC) ecc[xt] = 1;
//    if (t>=0 && xt!=t) {
//      i++;
//      continue;
//    }
    sm = st = sr = sg = 0;
    if (edb[n][i][TD_MECIURI]) sm = atoi(edb[n][i][TD_MECIURI]);
    if (edb[n][i][TD_TITULAR]) st = atoi(edb[n][i][TD_TITULAR]);
    if (edb[n][i][TD_REZERVA]) sr = atoi(edb[n][i][TD_REZERVA]);
    if (edb[n][i][TD_GOLURI])  sg = atoi(edb[n][i][TD_GOLURI]);
  
    pmeci[p] += sm;
    ptit[p]  += st;
    prez[p]  += sr;
    pgol[p]  += sg;

    if (year!=pfy[p] && year!=ply[p]) psez[p]++;
    if (pfy[p]==0) pfy[p] = year;
    if (year<pfy[p]) pfy[p] = year;
    if (year>ply[p]) ply[p] = year;

    i++;
  }
}

void NatStats(int year, int t) {
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
  
    pmeci[p] += sm;
    ptit[p]  += st;
    prez[p]  += sr;
    pgol[p]  += sg;

    if (year!=pfy[p] && year!=ply[p]) psez[p]++;
    if (pfy[p]==0) pfy[p] = year;
    if (year<pfy[p]) pfy[p] = year;
    if (year>ply[p]) ply[p] = year;
    if (sm>0) echn[p] = 1;

    i++;
  }
}

void Lineups(int year) {
  int n = year - FY;
  int i = 0;

  fprintf(of, "<HTML>\n<TITLE>Loturi Divizia A %d-%d</TITLE>\n", year-1, year);
  fprintf(of, "<HEAD>\n<link href=\"/css/report.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>Data naºterii</TH>");
  fprintf(of, "<TH>Naþionalitate</TH>");
  fprintf(of, "<TH>Club</TH>");
  fprintf(of, "<TH>Nr</TH>");
  fprintf(of, "<TH>Post</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Titular</TH>");
  fprintf(of, "<TH>Rezervã</TH>");
  fprintf(of, "<TH>Goluri</TH>");
  fprintf(of, "</TR></THEAD>\n");

  while (i<MAX_DB && db[n][i]!=NULL) {
    int p = FindMnem(db[n][i][TD_MNEM]);
    if (p<0) {
      fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", db[n][i][TD_MNEM]);
      i++;
      continue;
    }

    fprintf(of, "\n<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD>%d</TD>", p);
    fprintf(of, "<TD ALIGN=\"left\">%s</TD>", db[n][i][TD_PRENUME]);
    fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"jucatori/%03d/%03d.html\">%s</A></TD>",
        pname[p], ppren[p], p/1000, p%1000, db[n][i][TD_NUME]);
    fprintf(of, "<TD sorttable_customkey=\"%d\">%s</TD>", NumericDOB(db[n][i][TD_DOB], DOB_YYYYMMDD), db[n][i][TD_DOB]);
    fprintf(of, "<TD>%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", db[n][i][TD_NAT], db[n][i][TD_NAT]);
    int xt = atoi(db[n][i][TD_CLUBID]);
    fprintf(of, "<TD>%s</TD>", NickOf(L, xt, year));
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", db[n][i][TD_NR]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", db[n][i][TD_POST]);
    for (int j=TD_MECIURI; j<=TD_GOLURI; j++) {
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>", db[n][i][j]);
    }
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

void PlayerStats(int pl) {
  int sm, st, sr, sg, xt;
  int ec, em, et, er, eg;
  int tkm, tkt, tkr, tkg;
  int tem, tet, ter, teg;
  int tnm, tnt, tnr, tng;
  int n, nrow, lasty;
  char ssn[32];
  char wsm[12];

  nrow = 0;
  fprintf(stderr, "Jucãtor %03d/%03d: %s %s.\n", pl/1000, pl%1000, ppren[pl], pname[pl]);
  sprintf(ofilename, "html/jucatori/%03d/%03d.html", pl/1000, pl%1000);
  of = fopen(ofilename, "wt");
  if (of==NULL) {
    fprintf(stderr, "ERROR: cannot write to file %s.\n", ofilename);
    return;
  }

  fprintf(of, "<HTML>\n<TITLE>%s %s</TITLE>\n", ppren[pl], pname[pl]);
  fprintf(of, "<HEAD>\n<link href=\"../../css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<TABLE CELLSPACING=\"10\" CELLPADDING=\"5\">\n<TR>\n<TD>");
  fprintf(of, "<H3><IMG SRC=\"../../../../thumbs/22/3/%s.png\"></IMG> %s %s</H3>\n", pcty[pl], ppren[pl], pname[pl]);
  fprintf(of, "<UL><LI>Data naºterii: %s </LI>\n", pdob[pl]);
  fprintf(of, "<LI>Locul naºterii: %s\n", ppob[pl]);
  if (pjud[pl]!=NULL && pjud[pl][0]!=0) {
      fprintf(of, " (%s)", pjud[pl]);
  }
  fprintf(of, "</LI>\n");
	int ncaps = clist[pl][0];
	if (ncaps>0) {
		int debmid = clist[pl][1];
		int debssn = debmid/1000;
		int debnum = debmid%1000;
		int debyear = debssn + FY;
		char sdate[32];
		strcpy(sdate, ldb[debssn][debnum][DB_DATE]);
		strtok(sdate, "@");
		int hid = atoi(ldb[debssn][debnum][DB_HOME]);
		int aid = atoi(ldb[debssn][debnum][DB_AWAY]);
		int scr = atoi(ldb[debssn][debnum][DB_SCORE]);
	  fprintf(of, "<LI>Debut #%d: <BR>în %s<BR> la meciul <A HREF=\"../../reports/%d/%d-%d.html\">%s-%s %d-%d</A></LI>\n",
			debord[pl]+1, sdate, debyear, hid, aid, NickOf(L, hid, debyear), NickOf(L, aid, debyear), scr/100, scr%100);
	}
  fprintf(of, "</UL>\n");
  websafeMnem(pmnem[pl], wsm);
  fprintf(of, "<TD><IMG src=\"../foto/%s.jpg\" HEIGHT=\"150\" ALT=\"Foto\" VALIGN=\"top\"></IMG></TD>\n", wsm);
  fprintf(of, "</TR></TABLE>\n");
  fprintf(of, "Prezenþe la cluburi româneºti: <BR><BR>\n");

  fprintf(of, "<TABLE WIDTH=\"99%%\" cellpadding=\"2\" RULES=\"groups\" frame=\"box\">\n");  
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"FFFFFF\">\n");
  fprintf(of, "<TH COLSPAN=\"4\"></TH>\n");
  fprintf(of, "<TH COLSPAN=\"4\">Prima Divizie</TH>\n");
  fprintf(of, "<TH COLSPAN=\"4\">Cupa României</TH>\n");
  fprintf(of, "<TH COLSPAN=\"4\">Cupele Europene</TH>\n");
  fprintf(of, "</TR>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"8%%\"> Sezon </TH>");
  fprintf(of, "<TH WIDTH=\"20%%\"> Echipã </TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> nr</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> P</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> T</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> R</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> G</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> T</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> R</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> G</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> T</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> R</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> G</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n"); 

  lasty = -1;

  for (int year=FY; year<=LY; year++) {

    SeasonName(year, ssn);
    n = year - FY;
    int i = 0;

    while (i<MAX_DB && db[n][i]!=NULL) {
      int p = FindMnem(db[n][i][TD_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", db[n][i][TD_MNEM]);
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
          strcpy(pdb[nrow][PTD_NR], " ");
          strcpy(pdb[nrow][PTD_POST], " ");
          strcpy(pdb[nrow][PTD_DIVM], " ");
          strcpy(pdb[nrow][PTD_DIVT], " ");
          strcpy(pdb[nrow][PTD_DIVR], " ");
          strcpy(pdb[nrow][PTD_DIVG], " ");
          for (int j=PTD_CUPM; j<NUM_PTD; j++) strcpy(pdb[nrow][j], " ");
          nrow++;
        }
      }      

      xt = -10999;
      if (db[n][i][TD_CLUBID]) xt = atoi(db[n][i][TD_CLUBID]);  

      sm = st = sr = sg = 0;
      if (db[n][i][TD_MECIURI]) sm = atoi(db[n][i][TD_MECIURI]);
      if (db[n][i][TD_TITULAR]) st = atoi(db[n][i][TD_TITULAR]);
      if (db[n][i][TD_REZERVA]) sr = atoi(db[n][i][TD_REZERVA]);
      if (db[n][i][TD_GOLURI])  sg = atoi(db[n][i][TD_GOLURI]);
  
      if (year !=1957) {
        pmeci[p] += sm;
        ptit[p]  += st;
        prez[p]  += sr;
        pgol[p]  += sg; 
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
      strcpy(pdb[nrow][PTD_NR], db[n][i][TD_NR]);
      strcpy(pdb[nrow][PTD_POST], db[n][i][TD_POST]);
      sprintf(pdb[nrow][PTD_DIVM], "%d", sm);
      sprintf(pdb[nrow][PTD_DIVT], "%d", st);
      sprintf(pdb[nrow][PTD_DIVR], "%d", sr);
      sprintf(pdb[nrow][PTD_DIVG], "%d", sg);
      for (int j=PTD_CUPM; j<NUM_PTD; j++) sprintf(pdb[nrow][j], " ");
      nrow++;
      i++;
      lasty = year;
    }
  }

  tkm = tkt = tkr = tkg = 0;
  tem = tet = ter = teg = 0;
  tnm = tnt = tnr = tng = 0;

  // potriveste meciurile in cupa romaniei

  for (int year=KFY; year<=KLY; year++) {
    n = year - KFY;
    int i = 0;
    SeasonName(year, ssn);

    while (i<MAX_KDB && kdb[n][i]!=NULL) {
      int p = FindMnem(kdb[n][i][TD_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", kdb[n][i][TD_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }

      xt = -10998;
      if (kdb[n][i][TD_CLUBID]) xt = atoi(kdb[n][i][TD_CLUBID]);  

      em = et = er = eg = 0;
      if (kdb[n][i][TD_MECIURI]) em = atoi(kdb[n][i][TD_MECIURI]);
      if (kdb[n][i][TD_TITULAR]) et = atoi(kdb[n][i][TD_TITULAR]);
      if (kdb[n][i][TD_REZERVA]) er = atoi(kdb[n][i][TD_REZERVA]);
      if (kdb[n][i][TD_GOLURI])  eg = atoi(kdb[n][i][TD_GOLURI]);

      int krow = nrow;
      int py;
      for (int j=0; j<nrow; j++) {
        if (strcmp(ssn, pdb[j][PTD_SEZON])==0) {
          if ((strcmp(pdb[j][PTD_ECHIPA], NickOf(L, xt, year))==0) ||
              (strcmp(pdb[j][PTD_ECHIPA], " ")==0)
             ) 
          {
            krow = j;
          }
        }
      }

      if (krow>=0 && krow<=nrow && em>0) {
        if (krow==nrow) {
          strcpy(pdb[krow][PTD_SEZON], ssn);
          strcpy(pdb[krow][PTD_ECHIPA], NickOf(L, xt, year));
          for (int j=PTD_NR; j<NUM_PTD; j++) strcpy(pdb[krow][j], " ");
          echid[krow] = xt;
          sezid[krow] = year;
          nrow++;
        }
        if (pdb[krow][PTD_ECHIPA][0]==' ') 
          strcpy(pdb[krow][PTD_ECHIPA], NickOf(L, xt, year));
        sprintf(pdb[krow][PTD_CUPM], "%d", em);
        sprintf(pdb[krow][PTD_CUPT], "%d", et);
        sprintf(pdb[krow][PTD_CUPR], "%d", er);
        sprintf(pdb[krow][PTD_CUPG], "%d", eg);
      } 

      if (krow==nrow-1) PlaceRow(krow, nrow);

      tkm += em; tkt += et; tkr += er; tkg += eg;
      i++;
    }
  }

  // potriveste meciurile in cupele europene

  for (int year=ECFY; year<=ECLY; year++) {
    n = year - ECFY;
    int i = 0;
    SeasonName(year, ssn);

    em = et = er = eg = 0;
    while (i<MAX_EDB && edb[n][i]!=NULL) {
      int p = FindMnem(edb[n][i][TD_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", edb[n][i][TD_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }

      xt = -14999;
      if (edb[n][i][TD_CLUBID]) xt = atoi(edb[n][i][TD_CLUBID]);  
      ec = -1;
      if (edb[n][i][TD_NR]) ec = atoi(edb[n][i][TD_NR]);  
     

      if (edb[n][i][TD_MECIURI]) em += atoi(edb[n][i][TD_MECIURI]);
      if (edb[n][i][TD_TITULAR]) et += atoi(edb[n][i][TD_TITULAR]);
      if (edb[n][i][TD_REZERVA]) er += atoi(edb[n][i][TD_REZERVA]);
      if (edb[n][i][TD_GOLURI])  eg += atoi(edb[n][i][TD_GOLURI]);

      i++;
    }

    if (em<=0) continue;

    int erow = nrow;
    int py;
    for (int j=0; j<nrow; j++) {
      if (strcmp(ssn, pdb[j][PTD_SEZON])==0) {
        if ((strcmp(pdb[j][PTD_ECHIPA], NickOf(L, xt, year))==0) ||
            (strcmp(pdb[j][PTD_ECHIPA], " ")==0)
           ) 
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
        for (int j=PTD_NR; j<NUM_PTD; j++) strcpy(pdb[erow][j], " ");
        nrow++;
      }  
      if (pdb[erow][PTD_ECHIPA][0]==' ')
        strcpy(pdb[erow][PTD_ECHIPA], NickOf(L, xt, year));
      sprintf(pdb[erow][PTD_EURM], "%s %2d", ECname(ec, year), em);
      sprintf(pdb[erow][PTD_EURT], "%d", et);
      sprintf(pdb[erow][PTD_EURR], "%d", er);
      sprintf(pdb[erow][PTD_EURG], "%d", eg);
    }

    if (erow==nrow-1) PlaceRow(erow, nrow);

    tem += em; tet += et; ter += er; teg += eg;
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

  char bgc[12], rbgc[12];
  for (int i=0; i<nrow; i++) {
      fprintf(of, "<TR");
      strcpy(bgc, "FFFFFF"); strcpy(rbgc, "DDFFFF");
      if (i%2==1) fprintf(of, " BGCOLOR=\"%s\"", rbgc);
      fprintf(of, ">");
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", pdb[i][PTD_SEZON]);
      fprintf(of, "<TD ALIGN=\"left\">  %s</TD>", pdb[i][PTD_ECHIPA]);
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", pdb[i][PTD_NR]);
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", pdb[i][PTD_POST]);

      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVM]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVT]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVR]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\">%s</FONT></TD>", pdb[i][PTD_DIVG]);

      if (i%2==0) { strcpy(bgc, "DDFFFF"), strcpy(rbgc, "FFFFFF"); }
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPM]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPT]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPR]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\">%s</FONT></TD>", pdb[i][PTD_CUPG]);

      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURM]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURT]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURR]);
      fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\">%s</FONT></TD>", pdb[i][PTD_EURG]);

      fprintf(of, "</TR>\n");
  }

  fprintf(of, "<TFOOT>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TD>Total</TD>");
  fprintf(of, "<TD></TD>");
  fprintf(of, "<TD></TD>");
  fprintf(of, "<TD></TD>");
  if (pmeci[pl]>0) {
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", pmeci[pl]);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", ptit[pl]);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", prez[pl]);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"990000\"><B>%d</FONT></B></TD>", pgol[pl]);
  }
  else fprintf(of, "<TD/><TD/><TD/><TD/>");

  if (tkm>0) {
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkm);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkt);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkr);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"BB9922\"><B>%d</FONT></B></TD>", tkg);
  }
  else fprintf(of, "<TD/><TD/><TD/><TD/>");

  if (tem>0) {
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", tem);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", tet);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", ter);
    fprintf(of, "<TD ALIGN=\"right\"><FONT COLOR=\"000099\"><B>%d</FONT></B></TD>", teg);
  }
  else fprintf(of, "<TD/><TD/><TD/><TD/>");

  fprintf(of, "</TR>\n");

  fprintf(of, "\n</TABLE>\n");

  fprintf(of, "<BR>Total la echipele de club: <B>%d</B> meciuri, <B>%d</B> goluri\n", pmeci[pl]+tkm+tem, pgol[pl]+tkg+teg);

  if (echn[pl]>0) {
//  fprintf(of, "<BR><BR>La echipa naþionalã:<BR><BR>\n");
  fprintf(of, "<BR><BR>\n");

  fprintf(of, "<TABLE WIDTH=\"60%%\" cellpadding=\"2\" RULES=\"groups\" frame=\"box\">\n");  
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"FFFFFF\">\n");
  fprintf(of, "<TH COLSPAN=\"6\">Echipa Naþionalã</TH>\n");
  fprintf(of, "</TR>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"15%%\"> An </TH>");
  fprintf(of, "<TH WIDTH=\"45%%\"> Club </TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> M</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> T</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> R</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> G</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n"); 

  int ntb = 0;
  for (int year=NFY; year<=NLY; year++) {
    n = year - NFY;
    int i = 0;

    em = et = er = eg = 0;
    while (i<MAX_NDB && ndb[n][i]!=NULL) {
      int p = FindMnem(ndb[n][i][TD_MNEM]);
      if (p<0) {
        fprintf(stderr, "ERROR: mnemonic ID %s not found.\n", ndb[n][i][TD_MNEM]);
        i++;
        continue;
      }
      if (p!=pl) {
        i++;
        continue;
      }
      
      fprintf(of, "<TR");
      if (ntb%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
      fprintf(of, ">");
      fprintf(of, "<TD ALIGN=\"center\">%d</TD>", year);
      fprintf(of, "<TD ALIGN=\"left\">%s</TD>",   ndb[n][i][TD_CLUBID]);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  ndb[n][i][TD_MECIURI]);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  ndb[n][i][TD_TITULAR]);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  ndb[n][i][TD_REZERVA]);
      fprintf(of, "<TD ALIGN=\"right\">%s</TD>",  ndb[n][i][TD_GOLURI]);
      fprintf(of, "</TR>\n");
      ntb++;

      if (ndb[n][i][TD_MECIURI]) em = atoi(ndb[n][i][TD_MECIURI]);
      if (ndb[n][i][TD_TITULAR]) et = atoi(ndb[n][i][TD_TITULAR]);
      if (ndb[n][i][TD_REZERVA]) er = atoi(ndb[n][i][TD_REZERVA]);
      if (ndb[n][i][TD_GOLURI])  eg = atoi(ndb[n][i][TD_GOLURI]);

      tnm += em; tnt += et; tnr += er; tng += eg;
      i++;
    }
  }

  fprintf(of, "<TFOOT>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TD>Total</TD>");
  fprintf(of, "<TD></TD>");

  if (tnm>0) {
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnm);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnt);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tnr);
    fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tng);
  }
  else fprintf(of, "<TD/><TD/><TD/><TD/>");
  fprintf(of, "</TR>\n");

  fprintf(of, "\n</TABLE>\n");

  }

	fprintf(of, "<HR>\n");
	fprintf(of, "<H3>Lista meciurilor în prima divizie</H3>\n");
  fprintf(of, "<TABLE WIDTH=\"70%%\" cellpadding=\"1\" RULES=\"groups\" frame=\"box\">\n");  
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
  fprintf(of, "<TH WIDTH=\"20%%\"> Data@Ora </TH>");
  fprintf(of, "<TH WIDTH=\"16%%\"> Gazde </TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> Scor </TH>");
  fprintf(of, "<TH WIDTH=\"16%%\"> Oaspeþi </TH>");
  fprintf(of, "<TH WIDTH=\"3%%\"> Et </TH>");

  fprintf(of, "<TH WIDTH=\"5%%\"> min</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> g</TH>");

  fprintf(of, "<TH WIDTH=\"4%%\"> j</TH>");
  fprintf(of, "<TH WIDTH=\"6%%\"> min</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> g</TH>");

  fprintf(of, "<TH WIDTH=\"4%%\"> j</TH>");
  fprintf(of, "<TH WIDTH=\"8%%\"> min</TH>");
  fprintf(of, "<TH WIDTH=\"4%%\"> g</TH>");
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
			}
      fprintf(of, "<TR");
      if (altc%2==1) fprintf(of, " BGCOLOR=\"FFFFFF\""); else fprintf(of, " BGCOLOR=\"DDFFFF\"");
      fprintf(of, ">");
			int hid = atoi(ldb[y][k][DB_HOME]);
			int aid = atoi(ldb[y][k][DB_AWAY]);
			int scr = atoi(ldb[y][k][DB_SCORE]);
			int rnd = atoi(ldb[y][k][DB_ROUND]);
			int wxl = mlist[pl][i+1]/1000;
			int min = mlist[pl][i+1]%1000;
			int gls = elist[pl][i+1];
      fprintf(of, "<TD ALIGN=\"left\">%s</TD>", ldb[y][k][DB_DATE]);
      fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NickOf(L, hid, year));
      fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../../reports/%d/%d-%d.html\">%d-%d</A></TD>", 
				fxcol[wxl], year, hid, aid, scr/100, scr%100);
      fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NickOf(L, aid, year));
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", rnd);

			scaps++;
			smins += min;
			sgols += gls;
			ccaps++;
			cmins += min;
			cgols += gls;

      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", min);
			if (gls>0) { fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gls);	} else { fprintf(of, "<TD></TD>"); }
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", scaps);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", smins);
			if (sgols>0) { fprintf(of, "<TD ALIGN=\"right\">%d</TD>", sgols);	} else { fprintf(of, "<TD></TD>"); }
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ccaps);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", cmins);
			if (cgols>0) { fprintf(of, "<TD ALIGN=\"right\">%d</TD>", cgols);	} else { fprintf(of, "<TD></TD>"); }
      fprintf(of, "</TR>\n");
  }

	fprintf(of, "</TBODY></TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

void Ranking(int cr) {
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
  if (t==-1 || t>=NC)
    fprintf(of, "<HTML>\n<TITLE>Loturi Divizia A %d-%d</TITLE>\n", FY, LY);
  else if (t==-2)
    fprintf(of, "<HTML>\n<TITLE>Prezenþe în cupele europene</TITLE>\n", ECFY, ECLY);
  else if (t==-3)
    fprintf(of, "<HTML>\n<TITLE>Prezenþe în echipa naþionalã</TITLE>\n", NFY, NLY);
  else if (t==-4)
    fprintf(of, "<HTML>\n<TITLE>Prezenþe în Cupa României</TITLE>\n", KFY, KLY);
  else
    fprintf(of, "<HTML>\n<TITLE>Loturi %s în Divizia A %d-%d</TITLE>\n", NameOf(L, t, 3000), FY, LY);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");
}

void HTMLTable(int t) {
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  if (t==-1 || t>=NC)
    fprintf(of, "<H3>Jucãtori în campionatul României %d-%d</H3>\n", FY, LY);
  else if (t==-2)
    fprintf(of, "<H3>Jucãtori ai echipelor româneºti în cupele europene %d-%d</H3>\n", ECFY, ECLY);
  else if (t==-3)
    fprintf(of, "<H3>Jucãtori în echipa naþionalã %d-%d</H3>\n", NFY, NLY);
  else if (t==-4)
    fprintf(of, "<H3>Jucãtori în Cupa României %d-%d</H3>\n", KFY, KLY);
  else 
    fprintf(of, "<H3>Jucãtori %s %d-%d</H3>\n", NameOf(L, t, 3000), FY, LY);
  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");  
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>Data naºterii</TH>");
  fprintf(of, "<TH>Naþionalitate</TH>");
  fprintf(of, "<TH>Sezoane</TH>");
  fprintf(of, "<TH>Primul</TH>");
  fprintf(of, "<TH>Ultimul</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Titular</TH>");
  fprintf(of, "<TH>Rezervã</TH>");
  fprintf(of, "<TH>Goluri</TH>");
  fprintf(of, "</TR></THEAD>\n"); 

  for (int i=0; i<NP; i++) {
    int x = rank[i];
    if (pmeci[x]==0) continue;
    fprintf(of, "<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD align=\"right\">%d.</TD>", i+1);
    fprintf(of, "<TD align=\"left\">%s</TD>", ppren[x]);
    fprintf(of, "<TD align=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"jucatori/%03d/%03d.html\">%s</A></TD>",
        pname[x], ppren[x], x/1000, x%1000, pname[x]);
    CanonicDOB(pdob[x], DOB_DD_MM_YYYY);
    fprintf(of, "<TD align=\"right\" sorttable_customkey=\"%d\">%s</TD>", NumericDOB(pdob[x], DOB_YYYYMMDD), pdob[x]);
    fprintf(of, "<TD align=\"center\">%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", pcty[x], pcty[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", psez[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pfy[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ply[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pmeci[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ptit[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", prez[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pgol[x]);
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
  ECFY = 1957; ECLY = 2013;
  NFY  = 1922; NLY  = 2013;
  tm = -1;
  pl = -1;
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
	ldb = new char***[nums];
	vdb = new char***[nums];
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

  for (year=FY; year<=LY; year++) {
    CollectDiv(year);
		CollectLineups(year);
		qSortDB(year);
		CollectEvents(year);
		ExtractLists(year);
  }
	qSortDebut();

  for (year=FY; year<=LY; year++) {
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
  }
  else if (tm<NC) {
    sprintf(ofilename, "html/catalog.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Ranking(cr);
    HTMLHeader(tm);
    HTMLTable(tm);
    HTMLFooter();
  
    fclose(of);
  }

  fprintf(stderr, "<UL>\n");

  if (tm==-1) for (int t=0; t<NC; t++) if (dva[t]>0) {
    fprintf(stderr, "<LI><A HREF=\"lot-%d.html\">Jucãtori %s</A></LI>\n", t, NameOf(L, t, 3000));
    ResetStats();
    for (year=FY; year<=LY; year++) {
      TeamStats(year, t, pl);
    } 

    Ranking(cr); 

    sprintf(ofilename, "html/lot-%d.html", t);
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      continue;
    }
    HTMLHeader(t);
    HTMLTable(t);
    HTMLFooter();
    
    fclose(of);
  }
  fprintf(stderr, "</UL>\n<HR>\n");


  ResetStats();
  for (year=KFY; year<=KLY; year++) {
    CollectCup(year);
    CupStats(year, -2);
  }

  
    sprintf(ofilename, "html/catalog-cupa.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Ranking(cr);
    HTMLHeader(-4);
    HTMLTable(-4);
    HTMLFooter();
  
    fclose(of);


  ResetStats();
  for (year=ECFY; year<=ECLY; year++) {
    CollectEC(year);
    ECStats(year, -2);
  }

  
    sprintf(ofilename, "html/catalog-euro.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Ranking(cr);
    HTMLHeader(-2);
    HTMLTable(-2);
    HTMLFooter();
  
    fclose(of);

  ResetStats();
  for (year=NFY; year<=NLY; year++) {
    CollectNat(year);
    NatStats(year, -3);
  }

  
    sprintf(ofilename, "html/catalog-nat.html");
    of = fopen(ofilename, "wt");
    if (of==NULL) {
      fprintf(stderr, "ERROR: could not open file %s.\n", ofilename);
      return 1;
    }
    Ranking(cr);
    HTMLHeader(-3);
    HTMLTable(-3);
    HTMLFooter();
  
    fclose(of);


  ResetStats();
  if (pl<0) for (int p=0; p<NP; p++) {
    PlayerStats(p);
  }
  else if (pl<NP) PlayerStats(pl);
 
  return 0;
}

