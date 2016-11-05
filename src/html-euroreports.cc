#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"
#include "alias.hh"

#define MAX_NPL     300
#define MAX_N        64
#define MAX_TEAMS  2000
#define MAX_ROSTER	100
#define CAT_ROWS   1000
#define CAT_COLS     20
#define ROSTER_SIZE	 22

#define MAX_NAMES  20000
#define DB_ROWS      60
#define DB_COLS      60
#define DB_CELL      40
#define EV_COLS      30
#define PL_INITIAL	  0
#define PL_FULL_NAME  1
#define EURO       1000

#define DB_HOME		 0
#define DB_AWAY		 1
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

#define EV_GOAL		 0
#define EV_OWNGOAL	 1
#define EV_PKGOAL	 2
#define EV_PKMISS	 3
#define EV_YELLOW	 4
#define EV_RED	 	 5
#define EV_YELLOWRED 6
#define PSO_TIME    200

//burdca,Carol,Burdan,00/00/1912,ROM,173, , ,3,3,0,-11
#define CAT_MNEM	 0
#define CAT_PREN	 1
#define CAT_NAME	 2
#define CAT_DOB		 3
#define CAT_CTTY	 4
#define CAT_TEAM	 5
#define CAT_NR		 6
#define CAT_POS		 7
#define CAT_CAPS	 8
#define CAT_TIT		 9
#define CAT_REZ		10
#define CAT_GSC		11
#define CAT_MIN	 	12
#define CAT_INT		13
#define CAT_BENCH	14
#define CAT_GREC	15

int ECFY, ECLY;
const char *month[] = {"", "Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
                     "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"};
const char *romonth[] = {"", "ianuarie", "februarie", "martie", "aprilie", "mai", "iunie",
                     "iulie", "august", "septembrie", "octombrie", "noiembrie", "decembrie"};

const char *evicon[] = {"g", "og", "pg", "pm", "cg" , "cr", "cgr"};
const char *cupmnem[] = {"SC", "CCE", "CC", "UEFA"};
const char *cupround[] = {"C�tig�toare", "Finala", "Semifinale", "Sferturi", "Optimi", "�aisprezecimi", "1/32", "1/64"};
const char *cuprmnem[] = {" ", "F", "S", "Q", "O", "�", "32", "64"};
const char* fxcol[] = {"F0F0B0", "AAFFAA", "FF8888"};

char **club;
char **mnem;
char **ctty;
char **dir;
int  NN, NC, NM, NT;
int  asez[MAX_TEAMS], awin[MAX_TEAMS], adrw[MAX_TEAMS], alos[MAX_TEAMS], agre[MAX_TEAMS], agsc[MAX_TEAMS], arank[MAX_TEAMS];
char ssn[32], ecupname[64], ecupmnem[64], flag[12], roundname[64];
int  year, score, home, away;

int  topsc[MAX_NAMES];
int  num_winter;
int  *start_winter, *end_winter;
int  roster[2*ROSTER_SIZE], annotation[2*ROSTER_SIZE], rmin[2*ROSTER_SIZE];
char rname[2*ROSTER_SIZE][DB_CELL];
int  mpl[2*ROSTER_SIZE], overtime;
int  nev, nrev, pso, evp[EV_COLS], evm[EV_COLS], evt[EV_COLS];

/* *************************************** */

char  db[DB_ROWS][DB_COLS][DB_CELL];
char edb[DB_ROWS][EV_COLS][DB_CELL];
char catalog[CAT_ROWS][128];
int  tid[MAX_N];
int  plid[MAX_N][MAX_ROSTER];
char rmnem[MAX_N][MAX_ROSTER][7];
int  npl[MAX_N];
int nnp, npid[MAX_NPL], rank[MAX_NPL];

int NP, NEP;
int *pmeci, *ptit, *pint, *prez, *pban, *ppen, *pown, *prec, *pred, *pmin, *pgol, *pgre, *prnk;
int *cmeci, *ctit, *cint, *crez, *cban, *cmin, *cgol, *cgre, *crnk;
int *pemeci, *petit, *peint, *perez, *peban, *pemin, *pegol, *pegre, *pernk;
int *cemeci, *cetit, *ceint, *cerez, *ceban, *cemin, *cegol, *cegre, *cernk;

Catalog Pl, Ant;
Locations Loc;
Catalog EPl, EAnt, EArb;
Locations ELoc;

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

int borna[256];

FILE *of;
char sf[128];

Aliases **L;

//--------------------------------------

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;

  f = fopen("country.dat", "rt");
  if (!f) return 0;
  fscanf(f, "%d\n", &NN);
  ctty = new char*[NN];
  dir = new char*[NN];
  for (int i=0; i<NN; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, ":");
    tok[1] = strtok(NULL, ",");
    dir[i] = strdup(tok[0]);
    ctty[i] = strdup(tok[1]+2);
    s[0] = 0;
  }
  fclose(f);

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
     pmeci[i] = ptit[i] = pint[i] = prez[i] = pban[i] = pmin[i] = pgol[i] = pgre[i] = 0;
     pemeci[i] = petit[i] = peint[i] = perez[i] = peban[i] = pemin[i] = pegol[i] = pegre[i] = 0;
     pernk[i] = i;
  }
}

void InitStats() {
  pmeci = new int[MAX_NAMES];
  ptit  = new int[MAX_NAMES];
  pint  = new int[MAX_NAMES];
  prez  = new int[MAX_NAMES];
  pban  = new int[MAX_NAMES];
  pmin  = new int[MAX_NAMES];
  pgol  = new int[MAX_NAMES];
  pgre  = new int[MAX_NAMES];
  prnk  = new int[MAX_NAMES];
  ppen  = new int[MAX_NAMES];
  pown  = new int[MAX_NAMES];
  prec  = new int[MAX_NAMES];
  pred  = new int[MAX_NAMES];

  cmeci = new int[MAX_NAMES];
  ctit  = new int[MAX_NAMES];
  cint  = new int[MAX_NAMES];
  crez  = new int[MAX_NAMES];
  cban  = new int[MAX_NAMES];
  cmin  = new int[MAX_NAMES];
  cgol  = new int[MAX_NAMES];
  cgre  = new int[MAX_NAMES];
  crnk  = new int[MAX_NAMES];

  pemeci = new int[MAX_NAMES];
  petit  = new int[MAX_NAMES];
  peint  = new int[MAX_NAMES];
  perez  = new int[MAX_NAMES];
  peban  = new int[MAX_NAMES];
  pemin  = new int[MAX_NAMES];
  pegol  = new int[MAX_NAMES];
  pegre  = new int[MAX_NAMES];
  pernk  = new int[MAX_NAMES];

  cemeci = new int[MAX_NAMES];
  cetit  = new int[MAX_NAMES];
  ceint  = new int[MAX_NAMES];
  cerez  = new int[MAX_NAMES];
  ceban  = new int[MAX_NAMES];
  cemin  = new int[MAX_NAMES];
  cegol  = new int[MAX_NAMES];
  cegre  = new int[MAX_NAMES];
  cernk  = new int[MAX_NAMES];

  for (int i=0; i<NP; i++) {
     cmeci[i] = ctit[i] = cint[i] = crez[i] = cban[i] = cmin[i] = cgol[i] = cgre[i] = 0;
     cemeci[i] = cetit[i] = ceint[i] = cerez[i] = ceban[i] = cemin[i] = cegol[i] = cegre[i] = 0;
     topsc[i] = cernk[i] = i;
  }
}

int LoadAccumulatedStats() {
  char filename[128];
  sprintf(filename, "data/euro-career-%d.dat", year-1);
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
  for (int i=0; i<NEP; i++) {
    fscanf(f, "%d %d %d %d %d %d %d %d\n", &imc, &itt, &irz, &igl, &imn, &iin, &ibn, &igr);
    cemeci[i] = imc;
    cetit[i]  = itt;
    cerez[i]  = irz;
    cegol[i]  = igl;
    cemin[i]  = imn;
    ceint[i]  = iin;
    ceban[i]  = ibn;
    cegre[i]  = igr;
  }
  fclose(f);

  sprintf(filename, "data/euro-alltime-%d.dat", year-1);
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
  sprintf(filename, "data/euro-career-%d.dat", year-1);
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
  for (int i=0; i<NEP; i++) {
    fscanf(f, "%d %d %d %d %d %d %d %d\n", &imc, &itt, &irz, &igl, &imn, &iin, &ibn, &igr);
    cemeci[i] = imc;
    cetit[i]  = itt;
    cerez[i]  = irz;
    cegol[i]  = igl;
    cemin[i]  = imn;
    ceint[i]  = iin;
    ceban[i]  = ibn;
    cegre[i]  = igr;
  }
  fclose(f);
  return 0;
}

int LoadAlltimeStats() {
  char filename[128];
  sprintf(filename, "data/euro-alltime-%d.dat", year-1);
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
  sprintf(filename, "data/euro-career-%d.dat", year);
  FILE *f = fopen(filename, "wt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: Could not open %s.\n", filename);
     return -1;
  }
  for (int i=0; i<NP; i++) {
    fprintf(f, "%3d %3d %3d %3d %6d %3d %3d %3d\n",  cmeci[i], ctit[i], crez[i], cgol[i], cmin[i], cint[i], cban[i], cgre[i]);
  }
  for (int i=0; i<NEP; i++) {
    fprintf(f, "%3d %3d %3d %3d %6d %3d %3d %3d\n",  cemeci[i], cetit[i], cerez[i], cegol[i], cemin[i], ceint[i], ceban[i], cegre[i]);
  }
  fclose(f);
  sprintf(filename, "data/euro-alltime-%d.dat", year);
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
  else {
    if (m>0)
      { prez[px]++; crez[px]++; }
    else
      { pban[px]++; cban[px]++; }
  }
}

void AddEuroStats(int px, int k, int m) {
  if (px<0 || px>=NEP) return;
  if (m<0) return;
  pemin[px] += m;
  cemin[px] += m;
  if (m>0) { pemeci[px]++; cemeci[px]++; }
  if (k>=1 && k<=11) {
    petit[px]++; cetit[px]++;
    if (m==90) { peint[px]++; ceint[px]++; }
  }
  else {
    if (m>0)
      { perez[px]++; cerez[px]++; }
    else
      { peban[px]++; ceban[px]++; }
  }
}

int FindTid(int t) {
	for (int i=0; i<NT; i++) {
		if (tid[i]==t) return i;
	}
	return -1;
}

void LoadCatalog() {
  char filename[64], s[5000], *tk[CAT_COLS];
  FILE *f;
  NT = 0;
  sprintf(filename, "euro-%d.dat", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  int i = 0;
  while (!feof(f)) {
    fgets(s, 5000, f);
    if (strlen(s)<20) continue;
    strncpy(catalog[i], s, 127);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<CAT_COLS; j++) tk[j]=strtok(NULL, ",\n");
		int tm = atoi(tk[CAT_TEAM]);
		int tix = FindTid(tm);
		if (tix<0) { tix = NT; tid[NT] = tm; npl[tix] = 0; NT++; }
		int pix = Pl.FindMnem(tk[CAT_MNEM]);
		plid[tix][npl[tix]] = pix;
		strncpy(rmnem[tix][npl[tix]], tk[CAT_MNEM], 6);
		npl[tix]++;
		s[0] = 0;
		i++;
  }
  fclose(f);
}

void LoadDB() {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "euro-lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
	int i = 0;
  while (!feof(f)) {
    fgets(s, 5000, f);
    if (strlen(s)<100) continue;
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(db[i][j], tk[j]);
      else strcpy(db[i][j], " ");
    }
    i++;
    s[0] = 0;
  }
  fclose(f);
	NM = i;
}

void LoadEvents() {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "euro-events-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  for (int i=0; i<NM; i++) {
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

char *EuroName(int t, int nick, int y) {
  int ct = t/EURO;
  int cl = t%EURO;
  if (cl==0) return ctty[ct];
  char filename[128];
  char s[1024];
	char *ename = new char[64];
  sprintf(ename, "%s/%d", ctty[ct], cl);
//  sprintf(filename, "../%s/euroteams.dat", dir[ct]);
  sprintf(filename, "../%s/alias.dat", dir[ct]);
  FILE *f = fopen(filename, "rt");
  if (!f) return ename;
  for (int i=0; i<cl; ++i) fgets(s, 1024, f);
  fclose(f);

	char *tok[20], *ty, *tn, *tm;
	int j, ay;
	tok[0] = strtok(s, "*\n");
	for (j=1; j<20; ++j) tok[j] = strtok(NULL, "*\n");
	j = 0;
  do {
		ty = strtok(tok[j], " ");
		ay = strtol(ty, NULL, 10);
		if (ay<year) {
			tn = strtok(NULL, "~");
			tm = strtok(NULL, "\n");
		}
		j++;
	} while (ay<y && tok[j]!=NULL);
	if (nick) strcpy(ename, tm);
	else strcpy(ename, tn);
  return ename;
}

void FlagOf(int t) {
	if (t<EURO) strcpy(flag, "ROM");
	else {
		int ct = t/1000;
		strcpy(flag, dir[ct]);
	}
	for (int i=0; i<strlen(flag); ++i) {
		if (flag[i]>='a' && flag[i]<='z') flag[i] -= 32;
	}
}

char *NameOf(Aliases **L, int t, int y) {
  if (t>=EURO) return EuroName(t,0,y);
  char *s = L[t]->GetName(y);
  if (!s) return club[t];
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
  if (t>=EURO) return EuroName(t,1,y);
  char *s = L[t]->GetNick(y);
  if (!s) return mnem[t];
  return s;
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

int isWinterSeason(int y) {
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) return 1;
  }
  return 0;
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
  fprintf(of, "<!DOCTYPE html>\n");
  fprintf(of, "<head>\n");
  fprintf(of, "  <title>%s vs. %s</title>\n", NameOf(L, a, year), NameOf(L, b, year));
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
  fprintf(of, "        <a href=\"../../istoric-%d.html\">%s</a>\n", a, NameOf(L, a, year));
  fprintf(of, "    </h3>\n");
  fprintf(of, "  </div>\n\n");

	int sc = atoi(db[r][DB_SCORE]);
  fprintf(of, "  <div class=\"container middle\">\n");
  fprintf(of, "    <h1 class=\"thick scoretime \">\n");
  fprintf(of, "    <a href=\"../../vs-%04d/vs-%d-%d.html\">", a, a, b);
  if (sc >= 0) {
    fprintf(of, "      %d - %d\n", sc/100, sc%100);
  } else {
    fprintf(of, " - \n");
  }
  fprintf(of, "    </a></h1>\n");
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container right\">\n");
  fprintf(of, "    <h3 class=\"thick\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">%s</a>\n", b, NameOf(L, b, year));
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

  fprintf(of, "  <div class=\"container left\">\n");
  fprintf(of, "    <div class=\"logo\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">\n", a);
  fprintf(of, "        <img src=\"../../logo/logo-%d.png\" height=\"150\"/>\n", a);
  fprintf(of, "      </a>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container middle\">\n");
  fprintf(of, "    <div class=\"details clearfix\">\n");
  fprintf(of, "      <dl>\n");
	int cr = atoi(db[r][DB_ROUND]);
	if (cr>0 && cr<6) {
		fprintf(of, "        <dd><A HREF=\"../../euro-%d.html\"></A></dd>\n\n", year);
	}
  fprintf(of, "        <dt>%s</dt>\n", ecupname);
  fprintf(of, "        <dd>%s</dd>\n", roundname);

	strcpy(sw, db[r][DB_DATE]);
	sd = strtok(sw, "@");
	sh = strtok(NULL, ",");
	char *sz = strtok(sd, "-");
	char *sm = strtok(NULL, "-");
	char *sy = strtok(NULL, "-");
	int nz=0, nm=0, ny=0;
	if (sz) nz = atoi(sz);
	if (sm) nm = atoi(sm);
	if (sy) ny = atoi(sy);
  fprintf(of, "        <dt>Data</dt>\n");
  fprintf(of, "        <dd>%d %s %d</dd>\n", nz, romonth[nm], ny);
  fprintf(of, "        <dd>Ora %s</dd>\n", (sh?sh:" "));
  fprintf(of, "      </dl>\n\n");

  fprintf(of, "      <dl>\n");
	if (home<EURO) {
		int stad = Loc.FindVenue(db[r][DB_VENUE]);
		if (stad>=0) {
			int ci = Loc.V[stad].city;
			fprintf(of, "        <dt>Stadion</dt>\n");
	  	fprintf(of, "        <dd> %s, %s </dd>\n", Loc.V[stad].getName(year), Loc.C[ci].name);
		} else {
			char sci[7];
			strncpy(sci,db[r][DB_VENUE],4); sci[4] = 0;
			int ci = Loc.FindCity(sci);
			if (ci>=0) {
				fprintf(of, "        <dt>Locul</dt>\n");
		  	fprintf(of, "        <dd> %s </dd>\n", Loc.C[ci].name);
			}
		}
	}
	else {
		int stad = ELoc.FindVenue(db[r][DB_VENUE]);
		if (stad>=0) {
			int ci = ELoc.V[stad].city;
			fprintf(of, "        <dt>Stadion</dt>\n");
	  	fprintf(of, "        <dd> %s, %s </dd>\n", ELoc.V[stad].getName(year), ELoc.C[ci].name);
		} else {
			char sci[7];
			strncpy(sci,db[r][DB_VENUE],4); sci[4] = 0;
			int ci = ELoc.FindCity(sci);
			if (ci>=0) {
				fprintf(of, "        <dt>Loc</dt>\n");
		  	fprintf(of, "        <dd> %s </dd>\n", ELoc.C[ci].name);
			}
		}
	}
  fprintf(of, "        <dd> </dd>\n");
  fprintf(of, "        <dt>Spectatori</dt>\n");
	toThousands(db[r][DB_ATTEND], satt);
  fprintf(of, "        <dd> %s </dd>\n", satt);
  fprintf(of, "      </dl>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container right\">\n");
  fprintf(of, "    <div class=\"logo\">\n");
  fprintf(of, "        <a href=\"../../istoric-%d.html\">\n", b);
  fprintf(of, "        <img src=\"../../logo/logo-%d.png\" height=\"150\"/>\n", b);
  fprintf(of, "      </a>\n");
  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n\n");
  fprintf(of, "</div>\n");

  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void ResetRoster() {
  for (int i=0; i<2*ROSTER_SIZE; i++) {
     roster[i] = -1; mpl[i] = 0;
     annotation[i] = 0;
     rmin[i] = -1;
  }
  overtime = 0;
}

void GetRoster(int r, int a, int b) {
  char s[DB_CELL], *sp, *sm;
  int rp, rm;
  ResetRoster();
  for (int i=DB_ROSTER1; i<DB_COACH1; i++) {
    strcpy(s, db[r][i]);
    sp = strtok(s, ":");
    sm = strtok(NULL, ",\n");
    if (sp) strncpy(rname[i-DB_ROSTER1], sp, DB_CELL-1); else strcpy(rname[i-DB_ROSTER1], "?");
    if (sm) rm = atoi(sm); else rm = -1;
    if (sp) {
      if (home<EURO) rp = Pl.FindMnem(sp); else rp =  EPl.FindMnem(sp)+MAX_NAMES;
    }
    else rp=-1;
    if (rm>=0 && rp>=0) roster[i-DB_ROSTER1] = rp;
    rmin[i-DB_ROSTER1] = rm;
  }
  for (int i=DB_ROSTER2; i<DB_COACH2; i++) {
    strcpy(s, db[r][i]);
    sp = strtok(s, ":");
    sm = strtok(NULL, ",\n");
    if (sp) strncpy(rname[ROSTER_SIZE+i-DB_ROSTER2], sp, DB_CELL-1); else strcpy(rname[ROSTER_SIZE+i-DB_ROSTER2], "?");
    if (sm) rm = atoi(sm); else rm=-1;
    if (sp) {
      if (away<EURO) rp = Pl.FindMnem(sp); else rp = EPl.FindMnem(sp)+MAX_NAMES;
    }
    else rp=-1;
    if (rm>=0 && rp>=0) roster[ROSTER_SIZE+i-DB_ROSTER2] = rp;
    rmin[ROSTER_SIZE+i-DB_ROSTER2] = rm;
  }
}

int RosterIdx(int px) {
  if (px<0) return -1;
  for (int i=0; i<2*ROSTER_SIZE; i++)
    if (roster[i] == px) return i;
  return -1;
}

int RosterMnem(int r, int i) {
	char *escn = edb[r][i];
	char scn[128];
	strncpy(scn, escn, 100);
	strtok(scn, "'`\"/#!,\n");
	for (int j=0; j<2*ROSTER_SIZE; j++) {
		if (strcmp(scn, rname[j])==0) return j;
	}
	return -1;
}

int Gkid(int t, int m) {
  return rmin[ROSTER_SIZE*t]>=m || m>=999 ? roster[t*ROSTER_SIZE] : roster[t*ROSTER_SIZE+11];
}

void ResetEvents() {
  nev = 0; nrev = 0; pso = 0;
  for (int i=0; i<EV_COLS; i++) { evp[i] = -1; evm[i] = -1; evt[i] = -1; }
}

void GetEvents(int r, int a, int b) {
  int ep, em;
  char s[DB_CELL], *sp, *sm;
  ResetEvents();
  for (int i=0; i<EV_COLS; i++) {
    strcpy(s, edb[r][i]);
    sp = strtok(s, "'`\"/#!,\n");
    sm = strtok(NULL, "'`\"/#!,\n");
    if (sm!=NULL) em = atoi(sm); else em=-1;
    if (sp!=NULL) {
      ep = RosterMnem(r,i);
    } else ep=-100;
    if (ep>=0 || em>=0) {
      if (em==0) em=999;
      evp[i] = ep; evm[i] = em; evt[i] = EV_GOAL;
		if (edb[r][i][0]=='~') {
		  if (strchr(edb[r][i], '`')) evt[i] = EV_OWNGOAL;
		  else if (strchr(edb[r][i], '"')) evt[i] = EV_PKGOAL;
		  else if (strchr(edb[r][i], '/')) evt[i] = EV_PKMISS;
		} else {
      	     if (edb[r][i][6]=='`') evt[i] = EV_OWNGOAL;
      	else if (edb[r][i][6]=='"') evt[i] = EV_PKGOAL;
      	else if (edb[r][i][6]=='/') evt[i] = EV_PKMISS;
        else if (edb[r][i][6]=='!') evt[i] = EV_RED;
        else if (edb[r][i][6]=='#') evt[i] = EV_YELLOW;
        else if (edb[r][i][6]=='*') evt[i] = EV_YELLOWRED;
      }
      nev++;
      if (em<PSO_TIME) nrev++; else pso = 1;
    }
  }
}

void SortEvents(int r) {
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
    Pl.GetInitial(px, pini);
    makeHexlink(Pl.P[px].mnem);
    fprintf(of, "<a href=\"../../jucatori/%s.html\">%s %s</a>\n",
      hexlink, (full?Pl.P[px].pren:pini), Pl.P[px].name);
  }
}

void HTMLEuroPlayerLink(int px, int full) {
  char pini[12];
  if (px<0) {
    fprintf(of, "?");
  }
  else {
    EPl.GetInitial(px, pini);
    makeHexlink(EPl.P[px].mnem);
    fprintf(of, "<a href=\"../../eurojucatori/%s.html\">%s %s</a>\n",
      hexlink, (full?EPl.P[px].pren:pini), EPl.P[px].name);
  }
}

void HTMLEventsBlock(int r, int a, int b) {
  int x = score/100;
  int y = score%100;
  fprintf(of, " <div class=\"block  clearfix block_match_goals-wrapper\" id=\"g10w\">\n");
  fprintf(of, "  <h2>Goluri</h2>\n\n");

  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_goals real-content clearfix \" id=\"g10\">\n\n");

  fprintf(of, "      <div class=\"fully-padded\">\n");
  fprintf(of, "  <table class=\"matches events\">\n");

  int cx = 0;
  int cy = 0;
  int pid = -1;
  int hsc;
  char sm[12];
  for (int e=0; e<nrev; e++) {
    if (evm[e] > PSO_TIME) continue;
    if (evm[e]>0 && evm[e]<=150) sprintf(sm, "%2d'", evm[e]); else sprintf(sm, " ");
    /* only for regular time */
//    if (evm[e]>90 && evm[e]<120) sprintf(sm, "90+%d'", evm[e]%90);
    if (evp[e]>=0 && evp[e]<2*ROSTER_SIZE) pid = roster[evp[e]]; else pid = -1;
    hsc = 2;
    if (evp[e]>= 0 && evp[e]<ROSTER_SIZE) hsc = 1;
    if (evp[e]>=ROSTER_SIZE && evp[e]<2*ROSTER_SIZE) hsc = 0;
    if (evt[e]==EV_OWNGOAL) {
      hsc = 1-hsc;
    }
    if (hsc==1 && evt[e]!=EV_YELLOW) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div>");
      if (pid>=0) {
        if (pid<MAX_NAMES) HTMLPlayerLink(pid, PL_FULL_NAME); else HTMLEuroPlayerLink(pid-MAX_NAMES, PL_FULL_NAME);
      }
      else fprintf(of, "%s", rname[evp[e]]+1);
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/>%s</span>  &nbsp;</div>\n", evicon[evt[e]], sm);
      fprintf(of, "      </td>\n");
      int gki = Gkid(1, evm[e]);
      if (evt[e]==EV_GOAL || evt[e]==EV_PKGOAL) {
        cx++;
        if (away<EURO && gki>=0) prec[gki]++;
        if (pid>=0) {
          if (home<EURO) {
		    pgol[pid]++;
  		    cgol[pid]++;
            if (evt[e] == EV_PKGOAL) { ppen[pid]++; }
          } else {
		    pegol[pid-MAX_NAMES]++;
  		    cegol[pid-MAX_NAMES]++;
          }
        }
      }
      else if (evt[e]==EV_OWNGOAL) {
        cx++;
        if (away<EURO && pid>=0) { pown[pid]++; }
        if (away<EURO && gki>=0) { prec[gki]++; }
      }
      else if (evt[e]==EV_RED) {
        if (home<EURO && pid>=0) { pred[pid]++; }
      }
        fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", cx, cy);
        fprintf(of, "      <td class=\"player player-b\">\n");
        fprintf(of, "        <div></div>\n");
        fprintf(of, "      </td>\n");
        fprintf(of, "    </tr>\n");
    }
    else if (hsc==0 && evt[e]!=EV_YELLOW) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div></div>\n");
      fprintf(of, "      </td>\n");
      int gki = Gkid(0, evm[e]);
      if (evt[e]==EV_GOAL || evt[e]==EV_PKGOAL) {
        if (home<EURO && gki>=0) prec[gki]++;
        if (pid>=0) {
          if (away<EURO) {
		    pgol[pid]++;
 	 		cgol[pid]++;
            if (evt[e] == EV_PKGOAL) { ppen[pid]++; }
          } else {
		    pegol[pid-MAX_NAMES]++;
 	 		cegol[pid-MAX_NAMES]++;
          }
        }
        cy++;
      }
      else if (evt[e]==EV_OWNGOAL) {
        cy++;
        if (home<EURO && pid>=0) { pown[pid]++; }
        if (home<EURO && gki>=0) { prec[gki]++; }
      }
      else if (evt[e]==EV_RED) {
        if (away<EURO && pid>=0) { pred[pid]++; }
      }
      fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", cx, cy);
      fprintf(of, "      <td class=\"player player-b\">\n");
      fprintf(of, "        <div>");
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/>%s</span>  ", evicon[evt[e]], sm);
      if (pid>=0) {
				if (pid<MAX_NAMES) {
					HTMLPlayerLink(pid, PL_FULL_NAME);
				}
				else {
					HTMLEuroPlayerLink(pid-MAX_NAMES, PL_FULL_NAME);
				}
			}
			else fprintf(of, "%s", rname[evp[e]]+1);
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
    fprintf(of, "        <div>Marcator oaspe�i #%d <span class=\"minute\">m'</span>  &nbsp;</div>\n", i+1);
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

void HTMLPenaltyBlock(int r, int a, int b) {
  fprintf(of, " <div class=\"block  clearfix block_match_goals-wrapper\" id=\"g10w\">\n");
  fprintf(of, "  <h2>Lovituri de departajare</h2>\n\n");

  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_goals real-content clearfix \" id=\"g10\">\n\n");

  fprintf(of, "      <div class=\"fully-padded\">\n");
  fprintf(of, "  <table class=\"matches events\">\n");

  int cx = 0;
  int cy = 0;
  int pid = -1;
  int hsc;
  char sm[12];
  for (int e=nrev; e<nev; e++) {
    if (evm[e] < PSO_TIME) continue;
    if (evt[e]!=EV_PKGOAL && evt[e]!=EV_PKMISS)  continue;
    if (evp[e]>=0 && evp[e]<2*ROSTER_SIZE) pid = roster[evp[e]]; else pid = -1;
    hsc = 2;
    if (evp[e]>= 0 && evp[e]<ROSTER_SIZE) hsc = 1;
    if (evp[e]>=ROSTER_SIZE && evp[e]<2*ROSTER_SIZE) hsc = 0;
    if (hsc==1) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div>");
      if (pid>=0) {
				if (pid<MAX_NAMES) HTMLPlayerLink(pid, PL_FULL_NAME);
				else HTMLEuroPlayerLink(pid-MAX_NAMES, PL_FULL_NAME);
			}
			else fprintf(of, "%s", rname[evp[e]]+1);
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/></span>  &nbsp;</div>\n", evt[e]==EV_PKGOAL?"g":"pm");
      fprintf(of, "      </td>\n");
      if (evt[e]==EV_PKGOAL) {
        cx++;
      }
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
      if (evt[e]==EV_PKGOAL) {
        cy++;
      }
      fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", cx, cy);
      fprintf(of, "      <td class=\"player player-b\">\n");
      fprintf(of, "        <div>");
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/></span>  ", evt[e]==EV_PKGOAL?"g":"pm");
      if (pid>=0) {
				if (pid<MAX_NAMES) {
					HTMLPlayerLink(pid, PL_FULL_NAME);
				}
				else {
					HTMLEuroPlayerLink(pid-MAX_NAMES, PL_FULL_NAME);
				}
			}
			else fprintf(of, "%s", rname[evp[e]]+1);
      fprintf(of, "</div>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "    </tr>\n");
    }
  }
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

void HTMLUnknownPlayerTR(int sn, char *pn, int m) {
    fprintf(of, "      <tr class=\"%s\">\n", (sn%2==1?"odd":"even"));
    fprintf(of, "        <td class=\"shirtnumber\">%d</td>\n", sn);
    fprintf(of, "        <td class=\"player large-link\"><img src=\"../../../../thumbs/22/3/ROM.png\"/>%s</td>\n", pn+1);
    fprintf(of, "        <td class=\"label\"></td>\n");
    fprintf(of, "        <td class=\"season_caps\"></td>\n");
    fprintf(of, "        <td class=\"career_caps\"></td>\n");
    fprintf(of, "        <td class=\"label\"></td>\n");
    fprintf(of, "        <td class=\"season_goals\"></td>\n");
    fprintf(of, "        <td class=\"career_goals\"></td>\n");
    fprintf(of, "        <td class=\"bookings\">");
    if (sn <=11 && m>0 && m<overtime) fprintf(of, "<img src=\"../../so.png\"/>%d'", m+1);
    else if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", overtime+1-m);
    fprintf(of, "        </td>\n");
    fprintf(of, "      </tr>\n\n");
}

void HTMLPlayerTR(int pn, int px, int m) {
    if (px < 0 && px>=-45) {
      fprintf(of, "      <tr class=\"%s\"></tr>\n", (pn%2==1?"odd":"even"));
      return;
    }
    int sn = (pn <= ROSTER_SIZE? pn : pn - ROSTER_SIZE);
    char pini[12];
    Pl.GetInitial(px, pini);
    fprintf(of, "      <tr class=\"%s\">\n", (sn%2==1?"odd":"even"));
    fprintf(of, "        <td class=\"shirtnumber\">%d</td>\n", sn);
    fprintf(of, "        <td class=\"player large-link\">\n");
	fprintf(of, "          <img src=\"../../../../thumbs/22/3/%s.png\"/>\n", Pl.P[px].cty);
    makeHexlink(Pl.P[px].mnem);
    fprintf(of, "          <a href=\"../../jucatori/%s.html\">%s %s</a>\n", hexlink, pini, Pl.P[px].name);
    fprintf(of, "        </td>\n");
    fprintf(of, "        <td class=\"label\">M:</td>\n");
    fprintf(of, "        <td class=\"season_caps\">%d</td>\n", pmeci[px]);
    fprintf(of, "        <td class=\"career_caps\"> (%d)</td>\n", cmeci[px]);
    fprintf(of, "        <td class=\"label\">G:</td>\n");
    fprintf(of, "        <td class=\"season_goals\">%d</td>\n", pgol[px]);
    fprintf(of, "        <td class=\"career_goals\"> (%d)</td>\n", cgol[px]);
    fprintf(of, "        <td class=\"bookings\">");
    if (annotation[pn-1] > 0) {
      if (sn >= 12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", annotation[pn-1]-m);
      fprintf(of,  "<img src=\"../../cr.png\"/>%d'", annotation[pn-1]);
    }
    else {
      if (sn <=11 && m<overtime) fprintf(of, "<img src=\"../../so.png\"/>%d'", m+1);
      else if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", overtime+1-m);
    }
    fprintf(of, "        </td>\n");
    fprintf(of, "      </tr>\n\n");
}

void HTMLEuroPlayerTR(int pn, int px, int m) {
    if (px < 0 && px>=-45) {
      fprintf(of, "      <tr class=\"%s\"></tr>\n", (pn%2==1?"odd":"even"));
      return;
    }
    int sn = (pn <= ROSTER_SIZE? pn : pn - ROSTER_SIZE);
    char pini[12];
    EPl.GetInitial(px, pini);
    fprintf(of, "      <tr class=\"%s\">\n", (sn%2==1?"odd":"even"));
    fprintf(of, "        <td class=\"shirtnumber\">%d</td>\n", sn);
    fprintf(of, "        <td class=\"player large-link\">\n");
		fprintf(of, "          <img src=\"../../../../thumbs/22/3/%s.png\"/>\n", EPl.P[px].cty);
     makeHexlink(EPl.P[px].mnem);
    fprintf(of, "          <a href=\"../../eurojucatori/%s.html\">%s %s</a>\n", hexlink, pini, EPl.P[px].name);
    fprintf(of, "        </td>\n");
    fprintf(of, "        <td class=\"label\">M:</td>\n");
    fprintf(of, "        <td class=\"season_caps\">%d</td>\n", pemeci[px]);
    fprintf(of, "        <td class=\"career_caps\"> (%d)</td>\n", cemeci[px]);
    fprintf(of, "        <td class=\"label\">G:</td>\n");
    fprintf(of, "        <td class=\"season_goals\">%d</td>\n", pegol[px]);
    fprintf(of, "        <td class=\"career_goals\"> (%d)</td>\n", cegol[px]);
    fprintf(of, "        <td class=\"bookings\">");
    if (annotation[pn-1] > 0) {
      if (sn >= 12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", annotation[pn-1]-m);
      fprintf(of,  "<img src=\"../../cr.png\"/>%d'", annotation[pn-1]);
    }
    else {
      if (sn <=11 && m<overtime) fprintf(of, "<img src=\"../../so.png\"/>%d'", m+1);
      else if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", overtime+1-m);
    }
    fprintf(of, "        </td>\n");
    fprintf(of, "      </tr>\n\n");
}

void HTMLCoachTR(char *sc, int e) {
	int cix;
	if (e) {
	  cix = Ant.FindMnem(sc);
	  fprintf(of, "      <tr class=\"even\">\n");
		fprintf(of, "        <td colspan=\"2\" style=\"padding: 0.5em;\">\n");
		if (cix>=0) {
            makeHexlink(Ant.P[cix].mnem);
			fprintf(of, "          <strong>Antrenor:</strong> <a href=\"../../antrenori/%s.html\">%s %s</a>\n",
         hexlink, Ant.P[cix].pren, Ant.P[cix].name);
		}
  	else {
			fprintf(of, "          <strong>Antrenor:</strong> %s\n", sc);
  	}
	}
	else {
	  cix = EAnt.FindMnem(sc);
	  fprintf(of, "      <tr class=\"even\">\n");
		fprintf(of, "        <td colspan=\"2\" style=\"padding: 0.5em;\">\n");
		if (cix>=0) {
          makeHexlink(EAnt.P[cix].mnem);
			fprintf(of, "          <strong>Antrenor:</strong> <a href=\"../../euroantrenori/%s.html\">%s %s</a>\n",
         hexlink, EAnt.P[cix].pren, EAnt.P[cix].name);
		}
  	else {
			fprintf(of, "          <strong>Antrenor:</strong> %s\n", sc);
  	}
	}
  fprintf(of, "        </td>\n");
  fprintf(of, "      </tr>\n");
}

void Overtime(int r) {
	char spn[64], *spm;
	int m;
  for (int i=1; i<=11; i++) {
    strcpy(spn, db[r][DB_T1+i]);
    strtok(spn, ":");
    spm = strtok(NULL, ",");
    if (spm) {
      m = atoi(spm);
			if (m>overtime) overtime = m;
		}
	}
  for (int i=1; i<=11; i++) {
    strcpy(spn, db[r][DB_T2+i]);
    strtok(spn, ":");
    spm = strtok(NULL, ",");
    if (spm) {
      m = atoi(spm);
			if (m>overtime) overtime = m;
    }
	}
}

void HTMLLineupsBlock(int r, int a, int b) {
  fprintf(of, " <div class=\"block  clearfix block_match_lineups-wrapper\" id=\"l7w\">\n");
  fprintf(of, "  <h2>Forma�ii</h2>\n\n");

  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_lineups real-content clearfix \" id=\"l7\">\n\n");

  char spn[32], *spm;
  char pini[5];
  int m, px;

	Overtime(r);

  fprintf(of, "<div class=\"container left\">\n");
  fprintf(of, "  <table class=\"playerstats lineups table\">\n");
  HTMLPlayerTH();

  for (int i=1; i<=11; i++) {
    strcpy(spn, db[r][DB_T1+i]);
    strtok(spn, ":");
    if (home<EURO) px = Pl.FindMnem(spn);
		else px = EPl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    if (px>=0) {
      if (home<EURO) {
        AddStats(px, i, m);
        HTMLPlayerTR(i, px, m);
      }
      else {
        AddEuroStats(px, i, m);
        HTMLEuroPlayerTR(i, px, m);
      }
    }
    else HTMLUnknownPlayerTR(i, spn, m);
  }

  HTMLCoachTR(db[r][DB_COACH1], (home<EURO));

  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "<div class=\"container right\">\n");

  fprintf(of, "  <table class=\"playerstats lineups table\">\n");
  HTMLPlayerTH();

  for (int i=1; i<=11; i++) {
    strcpy(spn, db[r][DB_T2+i]);
    strtok(spn, ":");
    if (away<EURO) px = Pl.FindMnem(spn);
    else px = EPl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    if (px>=0) {
	    if (away<EURO) {
				AddStats(px, i, m);
				HTMLPlayerTR(i+ROSTER_SIZE, px, m);
			}
			else {
				AddEuroStats(px, i, m);
				HTMLEuroPlayerTR(i+ROSTER_SIZE, px, m);
			}
		}
		else HTMLUnknownPlayerTR(i, spn, m);
  }

  HTMLCoachTR(db[r][DB_COACH2], (away<EURO));

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

  char spn[32], *spm;
  char pini[5];
  int m, px;

  fprintf(of, "<div class=\"container left\">\n");
  fprintf(of, "  <table class=\"playerstats lineups substitutions table\">\n");
  HTMLPlayerTH();

  for (int i=12; i<=ROSTER_SIZE; i++) {
    strcpy(spn, db[r][DB_T1+i]);
		if (spn[0]==' ') continue;

    strtok(spn, ":");
    if (home<EURO) px = Pl.FindMnem(spn);
    else px = EPl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    if (px>=0) {
	    if (home<EURO) {
				AddStats(px, i, m);
				HTMLPlayerTR(i, px, m);
			}
			else {
				AddEuroStats(px, i, m);
				HTMLEuroPlayerTR(i, px, m);
			}
		}
		else HTMLUnknownPlayerTR(i, spn, m);
  }
  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "<div class=\"container right\">\n");

  fprintf(of, "  <table class=\"playerstats lineups substitutions table\">\n");
  HTMLPlayerTH();

  for (int i=12; i<=ROSTER_SIZE; i++) {
    strcpy(spn, db[r][DB_T2+i]);
		if (spn[0]==' ') continue;
    strtok(spn, ":");
    if (away<EURO) px = Pl.FindMnem(spn);
    else px = EPl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    if (px>=0) {
	    if (away<EURO) {
				AddStats(px, i, m);
				HTMLPlayerTR(i+ROSTER_SIZE, px, m);
			}
			else {
				AddEuroStats(px, i, m);
				HTMLEuroPlayerTR(i+ROSTER_SIZE, px, m);
			}
		}
		else HTMLUnknownPlayerTR(i, spn, m);
  }

  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void HTMLRef(char *sc) {
  int rix = EArb.FindMnem(sc);
  fprintf(of, "          <dt><strong>Arbitru central:  </strong></dt>");
  if (rix>=0) {
        makeHexlink(EArb.P[rix].mnem);
		fprintf(of, "<dd><IMG src=\"../../../../thumbs/22/3/%s.png\"><A HREF=\"../../euroarbitri/%s.html\">%s %s (%s)</A></dd>",
				EArb.P[rix].cty,
          hexlink, EArb.P[rix].pren, EArb.P[rix].name, EArb.P[rix].cty);
  } else {
		fprintf(of, "<dd>%s</dd>", sc);
	}
  fprintf(of, "</dt>\n");
}

void HTMLRefsBlock(int r, int a, int b) {
  fprintf(of, "  <div class=\"block  clearfix block_match_additional_info-wrapper\" id=\"i6w\">\n");
  fprintf(of, "  <h2>Arbitri �i observatori</h2>\n");
  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_additional_info real-content clearfix \" id=\"i6\">\n\n");

  fprintf(of, "      <div class=\"fully-padded clearfix\">\n");
  fprintf(of, "  <dl class=\"details\">\n");
	HTMLRef(db[r][DB_REF]);
  fprintf(of, "    <dd></dd>\n\n");

  fprintf(of, "    <dt>Asisten�i:</dt>\n");
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

void EcupName(int r) {
	if (db[r][DB_COMP][0]=='I') {
		sprintf(ecupname, "Cupa Interncontinental�");
	}
	if (db[r][DB_COMP][0]=='0') {
		sprintf(ecupname, "Supercupa Europei");
	}
	else if (db[r][DB_COMP][0]=='1') {
		if (year<1992) sprintf(ecupname, "Cupa Campionilor");
		else sprintf(ecupname, "Liga Campionilor");
	}
	else if (db[r][DB_COMP][0]=='2') {
		sprintf(ecupname, "Cupa Cupelor");
	}
	else if (db[r][DB_COMP][0]=='3') {
		if (year < 2008) sprintf(ecupname, "Cupa UEFA");
		else sprintf(ecupname, "Liga Europa");
	}
}

void EcupMnem(int r) {
	if (db[r][DB_COMP][0]=='I') {
		sprintf(ecupname, "CI");
	}
	else if (db[r][DB_COMP][0]=='0') {
		sprintf(ecupname, "SE");
	}
	else if (db[r][DB_COMP][0]=='1') {
		if (year<1992) sprintf(ecupname, "CCE");
		else sprintf(ecupname, "LC");
	}
	else if (db[r][DB_COMP][0]=='2') {
		sprintf(ecupname, "CC");
	}
	else if (db[r][DB_COMP][0]=='3') {
		if (year < 2008) sprintf(ecupname, "UEFA");
		else sprintf(ecupname, "LE");
	}
}

void RoundName(char *rs) {
	roundname[0] = 0;
	if (!rs) return;
	int len = strlen(rs);
	strcpy(roundname, rs);
	char *sk = rs;
	while (sk && sk[0]==' ') sk++;
	if (sk[0]>='1' && sk[0]<='7') {
		int nr = strtol(sk, NULL, 10);
		if (nr>=0 && nr<=7) strcpy(roundname, cupround[nr]);
	}
	else if (sk[0]=='P')
		sprintf(roundname, "Tur preliminar %c", sk[1]);
	else if (sk[0]=='Q')
		sprintf(roundname, "Calific�ri grupe");
	else if (sk[0]=='G')
		sprintf (roundname, "Grupa %c", sk[1]);
}

void PrintReport(int r) {
  char rfilename[128];
	EcupName(r);
	RoundName(db[r][DB_ROUND]);
	int a = atoi(db[r][DB_HOME]);
	int b = atoi(db[r][DB_AWAY]);
    int z = CompactDate(db[r][DB_DATE]);
	home = a;
	away = b;
	score = atoi(db[r][DB_SCORE]);
  sprintf(rfilename, "html/reports/%d/e%d-%d-%d.html", year, a, b, z);
  of = fopen(rfilename, "wt");
  if (of==NULL) { fprintf(stderr, "ERROR: Could not open file %s.\n", rfilename); return; }

  GetRoster(r, a, b);
  GetEvents(r, a, b);
  SortEvents(r);
  HTMLHeader(r, a, b);
  HTMLScoreBlock(r, a, b);
  HTMLInfoBlock(r, a, b);
  if (score>0 || nrev>0) {
    HTMLEventsBlock(r, a, b);
  }
  if (pso > 0) {
    HTMLPenaltyBlock(r, a, b);
  }
  HTMLLineupsBlock(r, a, b);
  HTMLSubsBlock(r, a, b);
  HTMLRefsBlock(r, a, b);
  HTMLFooter();
  fclose(of);
}

void PlayerRanking(int cr) {
  int sorted, last;
  nnp = 0;
  for (int i=0; i<NP; ++i) {
    if (pmeci[i]>0) {
        npid[nnp++] = i;
    }
  }
  for (int i=0; i<nnp; i++) rank[i] = i;

  last = nnp-1;
  do {
    sorted = 1;
    for (int i=0; i<nnp-1; i++) {
      if (pmin[npid[rank[i+1]]] > pmin[npid[rank[i]]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
}

#define DOB_DD_MM_YYYY  0
#define DOB_YYYYMMDD  1

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
    sprintf(dob, "%04d%02d%02d", xy, xm, xd);
  }
}

int Wxl(int home, int away, int score) {
  int x = score/100;
  int y = score%100;
  if (score<0 || x==y) return 0;
  if (home < EURO) {
    return (x>y? 1 : 2);
  } else {
    return (x>y? 2 : 1);
  }
}

void SynopticTable() {
	char sfilename[128];
	sprintf(sfilename, "html/euro-%d.html", year);
	FILE *f = fopen(sfilename, "wt");
	if (!f) {
		fprintf(stderr, "ERROR: could not open file %s.\n", sfilename);
		return;
	}

  fprintf(f, "<HTML>\n");
  fprintf(f, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(f, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(f, "</HEAD>\n");
  fprintf(f, "<BODY>\n");

  fprintf(f, "<H2>");
  if (year>ECFY)
    fprintf(f, "<A HREF=euro-%d.html><IMG HEIGHT=\"20\" SRC=\"prev.gif\"></A>", year-1);
  fprintf(f, "Echipele rom�ne�ti �n cupele europene %d/%d", year-1, year);
  if (year<ECLY)
    fprintf(f, "<A HREF=euro-%d.html><IMG HEIGHT=\"20\" SRC=\"next.gif\"></A>", year+1);
  fprintf(f, "</H2>");
  fprintf(f, "<TABLE WIDTH=\"75%%\" cellpadding=\"1\" frame=\"box\">\n");
  fprintf(f, "<THEAD>\n");
  fprintf(f, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(f, "<TH> Data@Ora </TH>");
  fprintf(f, "<TH> Gazde </TH>");
  fprintf(f, "<TH> Scor </TH>");
  fprintf(f, "<TH> Oaspe�i </TH>");
  fprintf(f, "<TH> Cupa </TH>");
  fprintf(f, "<TH> Runda </TH>");
  fprintf(f, "</TR>\n");
  fprintf(f, "</THEAD>\n");

  fprintf(f, "<TBODY>\n");
	for (int i=0; i<NM; ++i) {
		int home  = atoi(db[i][DB_HOME]);
		int away  = atoi(db[i][DB_AWAY]);
        int zi    = CompactDate(db[i][DB_DATE]);
		int score = atoi(db[i][DB_SCORE]);
		int ecp   = atoi(db[i][DB_COMP]);
		int clen  = strlen(db[i][DB_ROUND]);
    int wxl = Wxl(home, away, score);
    fprintf(f, "<TR ");
    if (i%2==1) fprintf(f, "BGCOLOR=\"DDFFFF\" ");
    fprintf(f, ">\n");
		fprintf(f, "<TD ALIGN=\"left\">%s</TD>", db[i][DB_DATE]);
		FlagOf(home);
        fprintf(f, "<TD><IMG SRC=\"../../thumbs/22/3/%s.png\"/>", flag);
		fprintf(f, "%s</TD>", NameOf(L, home, year));
        fprintf(f, "<TD ALIGN=\"center\" BGCOLOR=\"%s\"><A HREF=\"reports/%d/e%d-%d-%d.html\">",
          fxcol[wxl], year, home, away, zi);
        if (score >= 0) {
            fprintf(f, "%d-%d</A></TD>", score/100, score%100);
        } else {
            fprintf(f, "-</A></TD>");
        }
		FlagOf(away);
    fprintf(f, "<TD><IMG SRC=\"../../thumbs/22/3/%s.png\"/>", flag);
		fprintf(f, "%s</TD>", NameOf(L, away, year));
		fprintf(f, "<TD ALIGN=\"center\">%s</TD>", cupmnem[ecp]);
		fprintf(f, "<TD ALIGN=\"center\">%s</TD>", db[i][DB_ROUND]);
		fprintf(f, "</TR>");
  }
  fprintf(f, "</TBODY></TABLE>\n");

  fprintf(f, "<H3>Juc�tori folosi�i</H3>\n");
  fprintf(f, "<script src=\"sorttable.js\"></script>\n");
  fprintf(f, "<TABLE WIDTH=\"75%%\" class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(f, "<THEAD><TR BGCLOR=\"DDDDDD\">\n");
  fprintf(f, "<TH>#</TH>");
  fprintf(f, "<TH>Prenume</TH>");
  fprintf(f, "<TH>Nume</TH>");
  fprintf(f, "<TH>Data na�terii</TH>");
  fprintf(f, "<TH>Meciuri</TH>");
  fprintf(f, "<TH>Minute</TH>");
  fprintf(f, "<TH>Titular</TH>");
  fprintf(f, "<TH>Rezerv�</TH>");
  fprintf(f, "<TH>Goluri</TH>");
  fprintf(f, "<TH>Pen</TH>");
  fprintf(f, "<TH>Auto</TH>");
  fprintf(f, "<TH>Gol/-</TH>");
  fprintf(f, "<TH>Elim/-</TH>");
  fprintf(f, "</TR></THEAD>\n");
  fprintf(f, "<TBODY>\n");

  PlayerRanking(1);

  for (int i=0; i<nnp; i++) {
    int x = npid[rank[i]];
    if (pmeci[x]==0) continue;
    fprintf(of, "<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"DDFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD align=\"right\">%d.</TD>", i+1);
    fprintf(of, "<TD align=\"left\">%s</TD>", Pl.P[x].pren);
    makeHexlink(Pl.P[x].mnem);
    fprintf(of, "<TD align=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"jucatori/%s.html\">%s</A></TD>",
        Pl.P[x].name, Pl.P[x].pren, hexlink, Pl.P[x].name);
    CanonicDOB(Pl.P[x].dob, DOB_DD_MM_YYYY);
    fprintf(of, "<TD align=\"right\" sorttable_customkey=\"%d\">%s</TD>", NumericDOB(Pl.P[x].dob, DOB_YYYYMMDD), Pl.P[x].dob);
    fprintf(of, "<TD align=\"right\">%d</TD>", pmeci[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pmin[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ptit[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", prez[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pgol[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", ppen[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", -pown[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", -prec[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pred[x]);
    fprintf(of, "</TR>\n");
  }

  fprintf(f, "</TBODY>\n");
  fprintf(f, "</TABLE>");
  fprintf(f, "</BODY>\n</HTML>\n");
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  char filename[256];
	ECFY = 1957;
	ECLY = 2017;

  if (!Load()) {
    printf("ERROR: called from invalid drectory.\n");
    return -1;
  }
  if (argc < 2) {
    printf("ERROR: No edition specified.\n");
    return -1;
  }

  if (argc > 2) {
    for (int k=1; k<argc; k++) {
      if (strcmp(argv[k], "-fy")==0 && k<argc-1)  {
        ECFY = atoi(argv[++k]);
      }
      else if (strcmp(argv[k], "-ly")==0 && k<argc-1)  {
        ECLY = atoi(argv[++k]);
      }
    }
  }
  char sarg1[128];
  strcpy(sarg1, argv[argc-1]);
  year = atoi(sarg1);
  SeasonName(year, ssn);
  LoadAlltimeStats();
  Pl.Load("players.dat");
  EPl.Load("europlayers.dat");
  NP  = Pl.Size();
  NEP = EPl.Size();
  InitStats();
  ResetStats();
  LoadPlayerStats();
  LoadDB();
  LoadCatalog();
  LoadEvents();
  Ant.Load("coaches.dat");
  EAnt.Load("eurocoaches.dat");
  EArb.Load("euroreferees.dat");
  Loc.Load("city.dat", "stadium.dat");
  ELoc.Load("eurocity.dat", "eurostadium.dat");

  for (int i=0; i<NM; i++) {
    PrintReport(i);
  }
  SynopticTable();
  SaveAccumulatedStats();

  return 0;
}
