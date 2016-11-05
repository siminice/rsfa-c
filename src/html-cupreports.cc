#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"

#define MAX_LEVELS    12
#define MAX_RR         4
#define MAX_N         64
#define MAX_TEAMS   2000
#define MAX_ROSTER	 100
#define CAT_ROWS    1000
#define CAT_COLS      20
#define ROSTER_SIZE   22

#define MAX_NAMES  20000
#define DB_KROWS      60
#define DB_KCOLS      60
#define DB_KCELL      40
#define EV_COLS       30
#define PL_INITIAL     0
#define PL_FULL_NAME   1

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

#define EV_GOAL		   0
#define EV_OWNGOAL	   1
#define EV_PKGOAL	   2
#define EV_PKMISS	   3
#define EV_PKSAVED	   4
#define EV_YELLOW	   5
#define EV_RED	 	   6
#define EV_YELLOWRED   7
#define PSO_TIME     200
#define UNKNOWN_TIME 999

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

const char *month[] = {"", "Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
                     "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"};
const char *romonth[] = {"", "ianuarie", "februarie", "martie", "aprilie", "mai", "iunie",
                     "iulie", "august", "septembrie", "octombrie", "noiembrie", "decembrie"};

const char *evicon[] = {"g", "og", "pg", "pm", "ps", "cg" , "cr", "cgr"};
const char *cupround[] = {"Câºtigãtoare", "Finala", "Semifinale", "Sferturi", "Optimi", "ªaisprezecimi"};

char **club;
char **mnem;
int  NC, NM, NT;
int  asez[MAX_TEAMS], awin[MAX_TEAMS], adrw[MAX_TEAMS], alos[MAX_TEAMS], agre[MAX_TEAMS], agsc[MAX_TEAMS], arank[MAX_TEAMS];
char ssn[32];
int  year, score, home, guest;

int  lid[MAX_N], lwin[MAX_N], ldrw[MAX_N], llos[MAX_N], lgsc[MAX_N], lgre[MAX_N], lpts[MAX_N], lpen[MAX_N], lpdt[MAX_N];
int  hwin[MAX_N], hdrw[MAX_N], hlos[MAX_N], hgsc[MAX_N], hgre[MAX_N], hpts[MAX_N];
int  gwin[MAX_N], gdrw[MAX_N], glos[MAX_N], ggsc[MAX_N], ggre[MAX_N], gpts[MAX_N];
int  prank[MAX_N], hrank[MAX_N], grank[MAX_N];
int  topsc[MAX_NAMES];
int  num_winter;
int  *start_winter, *end_winter;
int  roster[2*ROSTER_SIZE], annotation[2*ROSTER_SIZE];
int  overtime;
int  nev, nrev, pso, evp[EV_COLS], evm[EV_COLS], evt[EV_COLS];
char rname[2*ROSTER_SIZE][DB_KCELL];

/* *************************************** */

char  db[DB_KROWS][DB_KCOLS][DB_KCELL];
char edb[DB_KROWS][EV_COLS][DB_KCELL];
char catalog[CAT_ROWS][128];
int  tid[MAX_N];
int  plid[MAX_N][MAX_ROSTER];
char rmnem[MAX_N][MAX_ROSTER][7];
int  npl[MAX_N];

int NP;
int *psez, *pmeci, *ptit, *pint, *prez, *pban, *pmin, *pgol, *pgre, *prnk;
int *csez, *cmeci, *ctit, *cint, *crez, *cban, *cmin, *cgol, *cgre, *crnk;

Catalog Pl, Ant, Arb;
Locations Loc;

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
     csez[i] = cmeci[i] = ctit[i] = cint[i] = crez[i] = cban[i] = cmin[i] = cgol[i] = cgre[i] = 0;
     topsc[i] = crnk[i] = i;
  }
}

int LoadAccumulatedStats() {
  char filename[128];
  sprintf(filename, "data/cup-career-%d.dat", year-1);
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
  sprintf(filename, "data/cup-alltime-%d.dat", year-1);
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
  sprintf(filename, "data/cup-career-%d.dat", year-1);
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
  sprintf(filename, "data/cup-alltime-%d.dat", year-1);
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
  sprintf(filename, "data/cup-career-%d.dat", year);
  FILE *f = fopen(filename, "wt");
  if (f==NULL) {
     fprintf(stderr, "ERROR: Could not open %s.\n", filename);
     return -1;
  }
  for (int i=0; i<NP; i++) {
    fprintf(f, "%3d %3d %3d %3d %6d %3d %3d %3d\n",  cmeci[i], ctit[i], crez[i], cgol[i], cmin[i], cint[i], cban[i], cgre[i]);
  }
  fclose(f);
  sprintf(filename, "data/cup-alltime-%d.dat", year);
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
  else if (k>=12 && k<=14) {
    if (m>0) { prez[px]++; crez[px]++; }
  }
  else if (k<=22) {
    if (m==0) { pban[px]++; cban[px]++; }
  }
}

int FindTid(int t) {
	for (int i=0; i<NT; i++) {
		if (tid[i]==t) return i;
	}
	return -1;
}

void LoadCatalog() {
//burdca,Carol,Burdan,00/00/1912,ROM,173, , ,3,3,0,-11
  char filename[64], s[5000], *tk[CAT_COLS];
  FILE *f;
	NT = 0;
  sprintf(filename, "cupa-%d.dat", year);
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
  char filename[64], s[5000], *tk[DB_KCOLS];
  FILE *f;
  sprintf(filename, "cup-lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
	int i = 0;
  while (!feof(f)) {
    fgets(s, 5000, f);
		if (strlen(s)<100) continue;
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_KCOLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_KCOLS; j++) {
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
  char filename[64], s[5000], *tk[DB_KCOLS];
  FILE *f;
  sprintf(filename, "cup-events-%d.db", year);
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
  fprintf(of, "      %d - %d\n", sc/100, sc%100);
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
/*
  fprintf(of, "    <table class=\"seasonform\">\n");
  fprintf(of, "      <tr><td>All-time (%d)</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     asez[ia], ar1, awin[ia]+adrw[ia]+alos[ia], awin[ia], adrw[ia], alos[ia], agsc[ia], agre[ia], 2*awin[ia]+adrw[ia]);
  fprintf(of, "      <tr><td>Sezon</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rg1, win[a]+drw[a]+los[a], win[a], drw[a], los[a], gsc[a], gre[a], ppv*win[a]+drw[a]);
  fprintf(of, "      <tr><td>Acasã</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rh, hwin[a]+hdrw[a]+hlos[a], hwin[a], hdrw[a], hlos[a], hgsc[a], hgre[a], ppv*hwin[a]+hdrw[a]);
  fprintf(of, "    </table>\n");
*/
  fprintf(of, "  </div>\n\n");

  fprintf(of, "  <div class=\"container middle\">\n");
  fprintf(of, "    <div class=\"details clearfix\">\n");
  fprintf(of, "      <dl>\n");
  fprintf(of, "        <dt>Cupa României</dt>\n");
	int cr = atoi(db[r][DB_ROUND]);
	if (cr>0 && cr<6) {
		fprintf(of, "        <dd><A HREF=\"../../cupa-%d.html\">%s</A></dd>\n\n", year, cupround[cr]);
	}

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
/*
  fprintf(of, "    <table class=\"seasonform\">\n");
  fprintf(of, "      <tr><td>All-time (%d) </td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     asez[ib], ar2, awin[ib]+adrw[ib]+alos[ib], awin[ib], adrw[ib], alos[ib], agsc[ib], agre[ib], 2*awin[ib]+adrw[ib]);
  fprintf(of, "      <tr><td>Sezon</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rg2, win[b]+drw[b]+los[b], win[b], drw[b], los[b], gsc[b], gre[b], ppv*win[b]+drw[b]);
  fprintf(of, "      <tr><td>Deplasare</td><td>loc #%d.</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>-</td><td>%d</td><td>%dp</td></tr>",
                     rg, gwin[b]+gdrw[b]+glos[b], gwin[b], gdrw[b], glos[b], ggsc[b], ggre[b], ppv*gwin[b]+gdrw[b]);
  fprintf(of, "    </table>\n");
*/
  fprintf(of, "  </div>\n\n");
  fprintf(of, "</div>\n");

  fprintf(of, "    </div>\n");
  fprintf(of, "  </div>\n");
  fprintf(of, "</div>\n");
}

void ResetRoster() {
  for (int i=0; i<44; i++) { roster[i] = -1; annotation[i] = 0; }
	overtime = 0;
}

void GetRoster(int r, int a, int b) {
  char s[DB_KCELL], *sp, *sm;
  int rp, rm;
  ResetRoster();
  for (int i=DB_ROSTER1; i<DB_COACH1; i++) {
    strcpy(s, db[r][i]);
    sp = strtok(s, ":");
    sm = strtok(NULL, ",\n");
		if (sp) strncpy(rname[i-DB_ROSTER1], sp, DB_KCELL-1);
			else strcpy(rname[i-DB_ROSTER1], "?");
    if (sm) rm = atoi(sm); else rm=-1;
    if (sp) rp = Pl.FindMnem(sp); else rp=-1;
    if (rm>=0 && rp>=0) roster[i-DB_ROSTER1] = rp;
  }
  for (int i=DB_ROSTER2; i<DB_COACH2; i++) {
    strcpy(s, db[r][i]);
    sp = strtok(s, ":");
    sm = strtok(NULL, ",\n");
		if (sp) strncpy(rname[ROSTER_SIZE+i-DB_ROSTER2], sp, DB_KCELL-1);
			else strcpy(rname[ROSTER_SIZE+i-DB_ROSTER2], "?");
    if (sm) rm = atoi(sm); else rm=-1;
    if (sp) rp = Pl.FindMnem(sp); else rp=-1;
    if (rm>=0 && rp>=0) roster[22+i-DB_ROSTER2] = rp;
  }
}

int RosterIdx(int px) {
  if (px<0) return -1;
  for (int i=0; i<44; i++)
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

void ResetEvents() {
  nev = 0; nrev = 0; pso = 0;
  for (int i=0; i<EV_COLS; i++) { evp[i] = -1; evm[i] = -1; evt[i] = -1; }
}

void GetEvents(int r, int a, int b) {
  int ep, em;
  char s[DB_KCELL], *sp, *sm;
  ResetEvents();
  for (int i=0; i<EV_COLS; i++) {
    strcpy(s, edb[r][i]);
    sp = strtok(s, "'`\"/#!,\n");
    sm = strtok(NULL, "'`\"/#!,\n");
    if (sm!=NULL) em = atoi(sm); else em=-1;
    if (sp!=NULL) {
			ep = Pl.FindMnem(sp);
			if (ep<0) ep = -(i+1);
		} else ep=-100;
    if (ep>=0 || em>=0) {
      if (em==0) em=UNKNOWN_TIME;
      evp[i] = ep; evm[i] = em; evt[i] = EV_GOAL;
			if (edb[r][i][0]=='~') {
				if (strchr(edb[r][i], '`')) evt[i] = EV_OWNGOAL;
				else if (strchr(edb[r][i], '"')) evt[i] = EV_PKGOAL;
				else if (strchr(edb[r][i], '/')) evt[i] = EV_PKMISS;
				else if (strchr(edb[r][i], '!')) evt[i] = EV_RED;
			} else {
      	     if (edb[r][i][6]=='`') evt[i] = EV_OWNGOAL;
      	else if (edb[r][i][6]=='"') evt[i] = EV_PKGOAL;
      	else if (edb[r][i][6]=='/') evt[i] = EV_PKMISS;
      	else if (edb[r][i][6]=='!') evt[i] = EV_RED;
        else if (edb[r][i][6]=='#') evt[i] = EV_YELLOW;
        else if (edb[r][i][6]=='*') evt[i] = EV_YELLOWRED;
			}
      if (em < PSO_TIME || em == UNKNOWN_TIME) nrev++; else pso = 1;
      nev++;
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
    int rid;
		if (evp[i]>=0) rid = RosterIdx(evp[i]);
		else rid = RosterMnem(r,-evp[i]-1);
    evp[i] = rid;
    if (evt[i] == EV_RED) {
      annotation[evp[i]] = evm[i];
    }
  }
}

void HTMLPlayerLink(int px, int full) {
  char pini[12];
  if (px<0) {
    fprintf(of, "?");
  }
  else {
    makeHexlink(Pl.P[px].mnem);
    Pl.GetInitial(px, pini);
    fprintf(of, "<a href=\"../../jucatori/%s.html\">%s %s</a>\n",
      hexlink, (full?Pl.P[px].pren:pini), Pl.P[px].name);
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
    if (evm[e] > PSO_TIME && evm[e] < PSO_TIME+100) continue;
    if (evm[e]>0 && evm[e]<PSO_TIME) sprintf(sm, "%2d'", evm[e]); else sprintf(sm, " ");
    if (evp[e]>=0 && evp[e]<2*ROSTER_SIZE) pid = roster[evp[e]]; else pid = -1;
    hsc = 2;
    if (evp[e]>= 0 && evp[e]<ROSTER_SIZE) hsc = 1;
    if (evp[e]>=ROSTER_SIZE && evp[e]<2*ROSTER_SIZE) hsc = 0;
    if (evt[e]==EV_OWNGOAL) {
      hsc = 1-hsc;
    }
    if (evt[e]!=EV_YELLOW) {
    if (hsc==1) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div>");
      if (pid>=0) HTMLPlayerLink(pid, PL_FULL_NAME); else fprintf(of, "%s", rname[evp[e]]+1);
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/>%s</span>  &nbsp;</div>\n", evicon[evt[e]], sm);
      fprintf(of, "      </td>\n");
      if (evt[e]==EV_GOAL || evt[e]==EV_PKGOAL) {
        cx++;
        if (pid>=0) {
          pgol[pid]++;
  	      cgol[pid]++;
        }
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
        if (pid>=0) {
	      pgol[pid]++;
  	      cgol[pid]++;
        }
        cy++;
      }
      if (evt[e]==EV_OWNGOAL) { cy++; }
      fprintf(of, "      <td class=\"event-icon\"><div>%d - %d</div></td>\n", cx, cy);
      fprintf(of, "      <td class=\"player player-b\">\n");
      fprintf(of, "        <div>");
      fprintf(of, "<span class=\"minute\"><img src=\"../../%s.png\"/>%s</span>  ", evicon[evt[e]], sm);
      if (pid>=0) HTMLPlayerLink(pid, PL_FULL_NAME); else fprintf(of, "%s", rname[evp[e]]+1);
      fprintf(of, "</div>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "      </td>\n");
      fprintf(of, "    </tr>\n");
    }
    } // not yellow
  }
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
    if (evp[e]>=0 && evp[e]<44) pid = roster[evp[e]]; else pid = -1;
    hsc = 2;
    if (evp[e]>= 0 && evp[e]<22) hsc = 1;
    if (evp[e]>=22 && evp[e]<44) hsc = 0;
    if (hsc==1) {
      fprintf(of, "    <tr class=\"event    expanded\">\n");
      fprintf(of, "      <td class=\"player player-a\">\n");
      fprintf(of, "        <div>");
      if (pid>=0) HTMLPlayerLink(pid, PL_FULL_NAME);
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
      if (pid>=0) HTMLPlayerLink(pid, PL_FULL_NAME);
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

void HTMLUnknownPlayerTR(int pnum, char *pn, int m) {
    int sn = (pnum<=ROSTER_SIZE? pnum : pnum-ROSTER_SIZE);
    fprintf(of, "      <tr class=\"%s\">\n", (sn%2==1?"odd":"even"));
    fprintf(of, "        <td class=\"shirtnumber\">%d</td>\n", sn);
    fprintf(of, "        <td class=\"player large-link\"><img src=\"../../../../thumbs/22/3/ROM.png\"/>&nbsp;%s</td>\n", pn+1);
    fprintf(of, "        <td class=\"label\"></td>\n");
    fprintf(of, "        <td class=\"season_caps\"></td>\n");
    fprintf(of, "        <td class=\"career_caps\"></td>\n");
    fprintf(of, "        <td class=\"label\"></td>\n");
    fprintf(of, "        <td class=\"season_goals\"></td>\n");
    fprintf(of, "        <td class=\"career_goals\"></td>\n");
    fprintf(of, "        <td class=\"bookings\">");
    if (annotation[pnum-1] > 0) {
      if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", annotation[pnum-1]-m);
      fprintf(of,  "<img src=\"../../cr.png\"/>%d'", annotation[pnum-1]);
    } else {
      if (sn <=11 && m<90) fprintf(of, "<img src=\"../../so.png\"/>%d'", m+1);
      else if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", 91-m);
    }
    fprintf(of, "        </td>\n");
    fprintf(of, "      </tr>\n\n");
}

void HTMLPlayerTR(int pnum, int px, int m) {
    if (px < 0 && px>=-45) {
      fprintf(of, "      <tr class=\"%s\"></tr>\n", (pnum%2==1?"odd":"even"));
      return;
    }
    int sn = (pnum <= ROSTER_SIZE? pnum : pnum - ROSTER_SIZE);
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
    if (annotation[pnum-1] > 0) {
      if (sn >= 12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", annotation[pnum-1]-m);
      fprintf(of,  "<img src=\"../../cr.png\"/>%d'", annotation[pnum-1]);
    }
    else {
      if (sn <=11 && m<overtime) fprintf(of, "<img src=\"../../so.png\"/>%d'", m+1);
      else if (sn >=12 && m>0) fprintf(of, "<img src=\"../../si.png\"/>%d'", overtime+1-m);
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
    fprintf(of, "          <strong>Antrenor:</strong> %s\n", sc);
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
  fprintf(of, "  <h2>Formaþii</h2>\n\n");

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
    px = Pl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    if (px>=0) HTMLPlayerTR(i, px, m);
		else HTMLUnknownPlayerTR(i, spn, m);
  }

  HTMLCoachTR(db[r][DB_COACH1]);

  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "<div class=\"container right\">\n");

  fprintf(of, "  <table class=\"playerstats lineups table\">\n");
  HTMLPlayerTH();

  for (int i=1; i<=11; i++) {
    strcpy(spn, db[r][DB_T2+i]);
    strtok(spn, ":");
    px = Pl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    if (px>=0) HTMLPlayerTR(i+ROSTER_SIZE, px, m);
		else HTMLUnknownPlayerTR(i+ROSTER_SIZE, spn, m);
  }

  HTMLCoachTR(db[r][DB_COACH2]);

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

  for (int i=12; i<=22; i++) {
    strcpy(spn, db[r][DB_T1+i]);
		if (spn[0]==' ') continue;

    strtok(spn, ":");
    px = Pl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    if (px>=0) HTMLPlayerTR(i, px, m);
		else HTMLUnknownPlayerTR(i, spn, m);
  }
  fprintf(of, "    </tbody>\n");
  fprintf(of, "  </table>\n");
  fprintf(of, "</div>\n\n");

  fprintf(of, "<div class=\"container right\">\n");

  fprintf(of, "  <table class=\"playerstats lineups substitutions table\">\n");
  HTMLPlayerTH();

  for (int i=12; i<=22; i++) {
    strcpy(spn, db[r][DB_T2+i]);
		if (spn[0]==' ') continue;
    strtok(spn, ":");
    px = Pl.FindMnem(spn);
    spm = strtok(NULL, ",");
    if (spm==NULL) {
      fprintf(stderr, "ERROR: void mins for #%d %s.\n", i, Pl.P[px].name);
      m = 0;
    }
    else {
      m = atoi(spm);
    }
    AddStats(px, i, m);
    if (px>=0) HTMLPlayerTR(i+ROSTER_SIZE, px, m);
		else HTMLUnknownPlayerTR(i+ROSTER_SIZE, spn, m);
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
  } else {
		fprintf(of, "<dd>%s</dd>", sc);
	}
  fprintf(of, "</dt>\n");
}

void HTMLRefsBlock(int r, int a, int b) {
  fprintf(of, "  <div class=\"block  clearfix block_match_additional_info-wrapper\" id=\"i6w\">\n");
  fprintf(of, "  <h2>Arbitri ºi observatori</h2>\n");
  fprintf(of, "  <div class=\"content  \">\n");
  fprintf(of, "    <div class=\"block_match_additional_info real-content clearfix \" id=\"i6\">\n\n");

  fprintf(of, "      <div class=\"fully-padded clearfix\">\n");
  fprintf(of, "  <dl class=\"details\">\n");
	HTMLRef(db[r][DB_REF]);
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

void PrintReport(int r) {
  char rfilename[128];
	int a = atoi(db[r][DB_HOME]);
	int b = atoi(db[r][DB_AWAY]);
    int z = CompactDate(db[r][DB_DATE]);
	home = a;
	guest = b;
	score = atoi(db[r][DB_SCORE]);
  sprintf(rfilename, "html/reports/%d/c%d-%d-%d.html", year, a, b, z);
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

//---------------------------------------------

int main(int argc, char* argv[]) {
  char filename[256];
  if (!Load()) {
    printf("ERROR: called from invalid drectory.\n");
    return -1;
  }
  if (argc < 2) {
    printf("ERROR: No edition specified.\n");
    return -1;
  }

  char sarg1[128];
  strcpy(sarg1, argv[1]);
  year = atoi(sarg1);
  SeasonName(year, ssn);

  LoadAlltimeStats();
  Pl.Load("players.dat");
	NP = Pl.Size();
	InitStats();
	ResetStats();
  LoadPlayerStats();
  LoadDB();
	LoadCatalog();
  LoadEvents();
  Ant.Load("coaches.dat");
  Arb.Load("referees.dat");
	Loc.Load("city.dat", "stadium.dat");

	for (int i=0; i<NM; i++)
	PrintReport(i);
  SaveAccumulatedStats();

  return 0;
}
