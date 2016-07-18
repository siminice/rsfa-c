#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"
#include "alias.hh"

#define MAX_NPL     1000
#define CAT_ROWS    1000
#define CAT_COLS      20
#define ROSTER_SIZE   22

#define MAX_NAMES 20000
#define MAX_SEASONS 200
#define DB_ROWS      60
#define DB_COLS      60
#define DB_CELL      40
#define EV_COLS      30
#define PL_INITIAL    0
#define PL_FULL_NAME  1
#define EURO       1000
#define CTTY_ROM  39000

#define DB_HOME     0
#define DB_AWAY     1
#define DB_SCORE    2
#define DB_DATE     3
#define DB_COMP     4
#define DB_ROUND    5
#define DB_VENUE    6
#define DB_ATTEND   7
#define DB_WEATHER  8
#define DB_ROSTER1  9
#define DB_COACH1  31
#define DB_ROSTER2 32
#define DB_COACH2  54
#define DB_REF     55
#define DB_ASIST1  56
#define DB_ASIST2  57
#define DB_OBSERV  58
#define DB_T1       8
#define DB_T2      31

#define EV_GOAL      0
#define EV_OWNGOAL   1
#define EV_PKGOAL    2
#define EV_PKMISS    3
#define EV_YELLOW    4
#define EV_RED       5
#define EV_YELLOWRED 6
#define PSO_TIME   200

#define ROSTER_SIZE 22

int NFY, NLY, total;
const char *month[] = {"", "Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
                     "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"};
const char *romonth[] = {"", "ianuarie", "februarie", "martie", "aprilie", "mai", "iunie",
                     "iulie", "august", "septembrie", "octombrie", "noiembrie", "decembrie"};

const char *cupmnem[] = {"SC", "CCE", "CC", "UEFA"};
const char *cupround[] = {"Câºtigãtoare", "Finala", "Semifinale", "Sferturi", "Optimi", "ªaisprezecimi", "1/32", "1/64"};
const char* fxcol[] = {"F0F0B0", "AAFFAA", "FF8888"};

char **ctty;
char **dativ;
char **dir;
int  NN, NC, NM, NT;
int nnpl, vpl[MAX_NPL];
char flag[12], compname[64], roundname[64];
int  score, home, away;

Stat RH, RA, RN, RT;

/* *************************************** */

char  db[MAX_SEASONS][DB_ROWS][DB_COLS][DB_CELL];
char edb[MAX_SEASONS][DB_ROWS][EV_COLS][DB_CELL];
int  ngm[MAX_SEASONS];

int NP, NEP;
int *psez, *pfy, *ply, *pmeci, *ptit, *pint, *prez, *pban, *pmin, *pgol, *ppen, *pown, *prec, *pred, *prnk;
int *pesez, *pefy, *pely, *pemeci, *petit, *peint, *perez, *peban, *pemin, *pegol, *pepen, *peown, *perec, *pered, *pernk;

Catalog *Pl;
Locations *Loc;
Catalog *EPl;
Locations *ELoc;
Ranking *C;
int nnp, npid[MAX_NPL], rank[MAX_NPL];

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

FILE *of;
char sf[128];

Aliases **CL;

//--------------------------------------

#define MAX_COMP 100

struct Compet {
  char mnem[6];
  char name[32];
};

Compet Comp[MAX_COMP];

//--------------------------------------
int GetYear(char *sd) {
  if (!sd) return -1;
  if (strlen(sd)<10) return -1;
  char sy[16];
  strncpy(sy, sd+6, 4);
  sy[4] = 0;
  int y = strtol(sy, NULL, 10);
  return y;
}

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;

  f = fopen("country.dat", "rt");
  if (!f) {
    fprintf(stderr, "ERROR: file 'country.dat' not found.\n");
    return 0;
  }
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
      k++;
    }
    ctty[i] = strdup(name);
    s[0] = 0;
  }
  fclose(f);

  f = fopen("competitions.dat", "rt");
  if (!f) {
    fprintf(stderr, "ERROR: file 'competitions.dat' not found.\n");
    return 0;
  }
  fscanf(f, "%d\n", &NC);
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, ",");
    tok[1] = strtok(NULL, ",\n");
    if (tok[0]) strcpy(Comp[i].mnem, tok[0]); else strcpy(Comp[i].mnem, " ");
    if (tok[1]) strcpy(Comp[i].name, tok[1]); else strcpy(Comp[i].mnem, "?");
    s[0] = 0;
  }
  fclose(f);

  int nd;
  f = fopen("dative.dat", "rt");
  if (!f) {
    fprintf(stderr, "ERROR: file 'dative.dat' not found.\n");
    return 0;
  }
  fscanf(f, "%d\n", &nd);
  dativ = new char*[nd];
  for (int i=0; i<nd; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    s[strlen(s)-1] = 0;
    dativ[i] = strdup(s);
    s[0] = 0;
  }
  fclose(f);

  return 1;
}

void ResetPlayerStats() {
  for (int i=0; i<NP; i++) {
     psez[i] = pfy[i] = ply[i] = pmeci[i] = ptit[i] = prez[i] = pban[i] = pmin[i] = pgol[i] = 0;
     ppen[i] = pown[i] = prec[i] = pred[i] = 0;
     pesez[i] = pefy[i] = pely[i] = pemeci[i] = petit[i] = perez[i] = peban[i] = pemin[i] = pegol[i] = 0;
     pepen[i] = peown[i] = perec[i] = pered[i] = 0;
     pernk[i] = i;
  }
  total = 0;
  nnpl = 0;
  RH.reset(); RA.reset(); RN.reset(); RT.reset();
}

void InitStats() {
  psez  = new int[MAX_NAMES];
  pmeci = new int[MAX_NAMES];
  pfy   = new int[MAX_NAMES];
  ply   = new int[MAX_NAMES];
  ptit  = new int[MAX_NAMES];
  prez  = new int[MAX_NAMES];
  pban  = new int[MAX_NAMES];
  pmin  = new int[MAX_NAMES];
  pgol  = new int[MAX_NAMES];
  ppen  = new int[MAX_NAMES];
  pown  = new int[MAX_NAMES];
  prec  = new int[MAX_NAMES];
  pred  = new int[MAX_NAMES];
  prnk  = new int[MAX_NAMES];

  pesez  = new int[MAX_NAMES];
  pemeci = new int[MAX_NAMES];
  pefy   = new int[MAX_NAMES];
  pely   = new int[MAX_NAMES];
  petit  = new int[MAX_NAMES];
  perez  = new int[MAX_NAMES];
  peban  = new int[MAX_NAMES];
  pemin  = new int[MAX_NAMES];
  pegol  = new int[MAX_NAMES];
  pepen  = new int[MAX_NAMES];
  peown  = new int[MAX_NAMES];
  perec  = new int[MAX_NAMES];
  pered  = new int[MAX_NAMES];
  pernk  = new int[MAX_NAMES];
}

void AddStats(int px, int k, int m) {
  if (px<0 || px>=NP) return;
  if (m<0) return;
  pmin[px] += m;
  if (m>0) pmeci[px]++;
  if (k>=1 && k<=11) {
    ptit[px]++;
  }
  else {
    if (m>0)
      { prez[px]++; }
    else
      { pban[px]++; }
  }
}

void AddEuroStats(int px, int k, int m) {
  if (px<0 || px>=NEP) return;
  if (m<0) return;
  pemin[px] += m;
  if (m>0) { pemeci[px]++; }
  if (k>=1 && k<=11) {
    petit[px]++;
  }
  else {
    if (m>0)
      { perez[px]++; }
    else
     { peban[px]++; }
  }
}

int FindComp(char *mn) {
  for (int i=0; i<NC; ++i) {
    if (strcmp(Comp[i].mnem, mn)==0) return i;
  }
  return 0;
}

void tolower(char *s) {
  if (!s) return;
  int len = strlen(s);
  for (int i=0; i<len; ++i)
    if (s[i]>='A' && s[i]<='Z') s[i] += 32;
}

int FindCtty(char *s) {
  char sct[12];
  strcpy(sct, s);
  tolower(sct);
  for (int i=0; i<NN; ++i) {
    if (strcmp(dir[i], sct)==0) return i;
  }
  return -1;
}

void LoadDB(int year) {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  int y = year - NFY;
  ngm[y] = 0;

  int i = 0;
  sprintf(filename, "nat-lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  while (!feof(f)) {
    fgets(s, 5000, f);
    if (strlen(s)<100) continue;
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(db[y][i][j], tk[j]);
      else strcpy(db[y][i][j], " ");
    }
    i++;
    s[0] = 0;
  }
  ngm[y] = i;
  fclose(f);
}

void LoadEvents(int year) {
  char filename[64], s[5000], *tk[EV_COLS];
  FILE *f;
  int y = year - NFY;
  int i = 0;
  sprintf(filename, "nat-events-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  while (!feof(f)) {
    fgets(s, 5000, f);
    if (strlen(s)<6) continue;
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<EV_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<EV_COLS; j++) {
      if (tk[j]!=NULL) strcpy(edb[y][i][j], tk[j]);
      else strcpy(edb[y][i][j], " ");
    }
    i++;
    s[0] = 0;
  }
  fclose(f);
}

void FlagOf(int t) {
  int ct = t/1000;
  strcpy(flag, dir[ct]);
  for (int i=0; i<strlen(flag); ++i) {
    if (flag[i]>='a' && flag[i]<='z') flag[i] -= 32;
  }
}

char *NameOf(Aliases **L, int t, int y) {
  return L[t/EURO]->GetName(y);
}

char *NickOf(Aliases **L, int t, int y) {
  return L[t/EURO]->GetNick(y);
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

/*
int Gkid(int t, int m) {
  return rmin[ROSTER_SIZE*t]>=m || m>=999 ? roster[t*ROSTER_SIZE] : roster[t*ROSTER_SIZE+11];
}
*/

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
  }
}

void AddPlayerStats(int y, int r) {
  char spi[64], *sp, *sm;
  int min, pid;
  int h = atoi(db[y][r][DB_HOME]);
  int g = atoi(db[y][r][DB_AWAY]);
  int year  = GetYear(db[y][r][DB_DATE]);
  int j1 = DB_ROSTER1;
  int j2 = DB_ROSTER1+ROSTER_SIZE;
  if (g==CTTY_ROM) { j1 = DB_ROSTER2; j2 = DB_ROSTER2+ROSTER_SIZE; }
  for (int j=j1; j<j2; j++) {
    strcpy(spi, db[y][r][j]);
    sp = strtok(spi, ":");
    sm = strtok(NULL, ":");
    min = sm!=NULL ? atoi(sm) : 0;
    pid = Pl->binFindMnem(sp);
    if (pid>=0) {
      if (min>0) {
        pmin[pid] += min;
        pmeci[pid]++;
        if (j<j1+11) { ptit[pid]++; } else { prez[pid]++; }
        if (pfy[pid] == 0) {
          psez[pid] = 1;
          pfy[pid] = ply[pid] = year;
        }
        else if (year > ply[pid]) {
          psez[pid]++;
          ply[pid] = year;
        }
      } else {
        pban[pid]++;
      }
    }
  }
  for (int j=0; j<EV_COLS; j++) {
    if (edb[y][r][j]==NULL || edb[y][r][j][0]==0x0 || edb[y][r][j][0]==' ' || edb[y][r][j][0]=='~') continue;
    strcpy(spi, edb[y][r][j]);
    char evt = edb[y][r][j][6];
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

void DatatablesHeader() {
    fprintf(of, "<link rel=\"stylesheet\" type=\"text/css\" href=\"//cdn.datatables.net/1.10.10/css/jquery.dataTables.css\">");
    fprintf(of, "<script type=\"text/javascript\" language=\"javascript\" src=\"//code.jquery.com/jquery-1.11.3.min.js\"></script>");
    fprintf(of, "<script type=\"text/javascript\" charset=\"utf8\" src=\"//cdn.datatables.net/1.10.10/js/jquery.dataTables.js\"></script>");
    fprintf(of, "<script type=\"text/javascript\" class=\"init\">");
    fprintf(of, "$(document).ready( function () {");
    fprintf(of, "    $('#all').DataTable( {");
    fprintf(of, "      pageLength : 20,");
    fprintf(of, "    } );");
    fprintf(of, "} );");
    fprintf(of, "</script>");
}

void HTMLLineupsHeader() {
  fprintf(of, "<script src=\"../../sorttable.js\"></script>\n");
//  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<TABLE id=\"all\" class=\"display\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>Data naºterii</TH>");
  fprintf(of, "<TH>Naþ.</TH>");
  fprintf(of, "<TH>#Sez.</TH>");
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
    fprintf(of, "<TD align=\"right\">%d</TD>", prec[x]);
    fprintf(of, "<TD align=\"right\">%d</TD>", pred[x]);
    fprintf(of, "</TR>\n");
  }
  fprintf(of, "</TABLE>");
}

void HTMLPlayerLink(int px, int full) {
  char pini[12];
  if (px<0) {
    fprintf(of, "?");
  }
  else {
    Pl->GetInitial(px, pini);
    makeHexlink(Pl->P[px].mnem);
    fprintf(of, "<a href=\"../../jucatori/%s.html\">%s %s</a>\n",
      hexlink, (full?Pl->P[px].pren:pini), Pl->P[px].name);
  }
}

void HTMLEuroPlayerLink(int px, int full) {
  char pini[12];
  if (px<0) {
    fprintf(of, "?");
  }
  else {
    EPl->GetInitial(px, pini);
    makeHexlink(EPl->P[px].mnem);
    fprintf(of, "<a href=\"../../eurojucatori/%s.html\">%s %s</a>\n",
      hexlink, (full?EPl->P[px].pren:pini), EPl->P[px].name);
  }
}

void CompName(int y, int r) {
  char s[32], *tok[5];
  char p, prel[32];
  strcpy(s, db[y][r][DB_COMP]);
  tok[0] = strtok(s, "'");
  tok[1] = strtok(NULL, "\n");
  p=tok[0][0];
  if (tok[0][0]=='T' || tok[0][0]=='P') tok[0]++;
  prel[0] = 0;
  if (p=='T') strcpy(prel, "Turneul Final<BR>");
  if (p=='P') strcpy(prel, "Preliminarii<BR>");
  int c = FindComp(tok[0]);
  if (tok[1]) {
    sprintf(compname, "%s %s %s", prel, Comp[c].name, tok[1]);
  }
  else {
    sprintf(compname, "%s %s", prel, Comp[c].name);
  }
}

void CompMnem(int y, int r) {
  if (db[y][r][DB_COMP][0]=='A') {
    sprintf(compname, "A");
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
    sprintf(roundname, "Calificãri grupe");
  else if (sk[0]=='G')
    sprintf (roundname, "Grupa %c", sk[1]);
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

int FindHost(char *stad, int year) {
  char *ecit = new char[DB_CELL];
  strncpy(ecit, stad, 4);
  ecit[4] = 0;
  int ci = ELoc->FindCity(ecit);
  delete ecit;
  int ct = (ci>=0) ? FindCtty(ELoc->C[ci].jud) : -1;
  if ((year < 1993) && (ct==10 || ct==44 || ct==8 || ct==32 || ct==218)) ct = 27;
  if ((year < 1994) && (ct==43)) ct = 12;
  if ((year < 1945) && (ct==49)) ct = 37;
  if ((year > 1945 && year < 1993) && (ct==49 || ct==7 || ct==19 || ct==28 || ct==30 || ct==34)) ct = 40;
  if ((year > 1989) && (ct==51)) ct = 20;
  return ct;
}

void VsPage(int t) {
  char sfilename[128];
  sprintf(sfilename, "html/vs-39000/vs-39000-%d000.html", t);
  of = fopen(sfilename, "wt");
  if (!of) {
    fprintf(stderr, "ERROR: could not open file %s.\n", sfilename);
    return;
  }

  fprintf(of, "<HTML>\n");
  fprintf(of, "<HEAD>\n<link href=\"../css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  DatatablesHeader();
  fprintf(of, "</HEAD>\n");
  fprintf(of, "<BODY>\n");

  fprintf(of, "<H3>Palmaresul echipei României împotriva %s<H3>\n", dativ[t]);
  fprintf(of, "<TABLE WIDTH=\"70%%\" cellpadding=\"1\" frame=\"box\">\n");
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"3%%\"> Num </TH>");
  fprintf(of, "<TH WIDTH=\"17%%\"> Data@Ora </TH>");
  fprintf(of, "<TH WIDTH=\"15%%\"> Loc </TH>");
  fprintf(of, "<TH WIDTH=\"17%%\"> Gazde </TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">  Scor </TH>");
  fprintf(of, "<TH WIDTH=\"17%%\"> Oaspeþi </TH>");
  fprintf(of, "<TH WIDTH=\"14%%\"> Competiþia </TH>");
  fprintf(of, "<TH WIDTH=\"12%%\">  Runda </TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "</THEAD>\n");

  fprintf(of, "<TBODY>\n");
  for (int y = 0; y<=NLY-NFY; y++) {
    for (int i=0; i<ngm[y]; ++i) {
      int home  = atoi(db[y][i][DB_HOME]);
      int away  = atoi(db[y][i][DB_AWAY]);
      if (home != t*1000 && away != t*1000) continue;
      int year  = GetYear(db[y][i][DB_DATE]);
      int score = atoi(db[y][i][DB_SCORE]);
      int zi    = CompactDate(db[y][i][DB_DATE]);
      int ecp   = atoi(db[y][i][DB_COMP]);
      int clen  = strlen(db[y][i][DB_ROUND]);
      int repl  = (db[y][i][DB_ROUND][clen-1] == 'r' || db[y][i][DB_ROUND][clen-1] == 'R');
      int x1 = score/100;
      int x2 = score%100;
      int wxl=0;
      int host = FindHost(db[y][i][DB_VENUE], y+NFY);
      int opp = home + away - CTTY_ROM;
      int ven = 1;
      if (host == -1) ven = 0;
      if (host == opp/EURO) ven = 2;

      if (score>=0) {
        if (ven==0) {
          if (x1>x2) wxl = 1; else if (x1<x2) wxl = 2;
          RH.addRes(x1, x2);
          RT.addRes(x1, x2);
          C->S[opp/EURO].addRes(x1, x2);
        } else {
          RT.addRes(x2, x1);
          C->S[opp/EURO].addRes(x2, x1);
          if (x1<x2) wxl = 1;
          if (x1>x2) wxl = 2;
          if (ven == 2)
            { RA.addRes(x2, x1); }
          else
           { RN.addRes(x2, x1); }
        }
      }
      fprintf(of, "<TR ");
      if (ven==0) fprintf(of, "BGCOLOR=\"DDFFFF\" ");
      if (ven==1) fprintf(of, "BGCOLOR=\"EEEEEE\" ");
      fprintf(of, ">\n");
      fprintf(of, "<TD ALIGN=\"center\">%d.</TD>", ++total);
      fprintf(of, "<TD ALIGN=\"left\">%s</TD>", db[y][i][DB_DATE]);
        char sci[7];
        strncpy(sci,db[y][i][DB_VENUE],4); sci[4] = 0;
        int ci;
      if (host==-1) {
        ci = Loc->FindCity(sci);
        fprintf(of, "<TD>%s</TD>", (ci>=0?Loc->C[ci].name:" "));
      }
      else {
        ci = ELoc->FindCity(sci);
        fprintf(of, "<TD>%s</TD>", (ci>=0?ELoc->C[ci].name:" "));
      }
      FlagOf(home);
      fprintf(of, "<TD><IMG SRC=\"../../../thumbs/22/3/%s.png\"/>", flag);
          char sscore[6]; strcpy(sscore, "-");
          if (score>=0) sprintf(sscore, "%d-%d", x1, x2);
      fprintf(of, "%s</TD>", NameOf(CL, home, y+NFY));
      fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../reports/%d/n%d-%d-%d.html\">%s</A></TD>",
        fxcol[wxl], year, home, away, zi, sscore);
      FlagOf(away);
      fprintf(of, "<TD><IMG SRC=\"../../../thumbs/22/3/%s.png\"/>", flag);
      fprintf(of, "%s</TD>", NameOf(CL, away, y+NFY));
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", db[y][i][DB_COMP]);
      RoundName(db[y][i][DB_ROUND]);
      fprintf(of, "<TD ALIGN=\"center\">%s</TD>", roundname);
      fprintf(of, "</TR>");
      AddPlayerStats(y, i);
    }
  }

  fprintf(of, "</TBODY></TABLE>\n");

  fprintf(of, "<H3>Bilanþ</H3>\n");
  fprintf(of, "<TABLE width=\"30%%\" cellpadding=\"0\" frame=\"box\">\n");
  fprintf(of, "<THEAD>");
  fprintf(of, "<TR>");
  fprintf(of, "<TH width=\"20%%\"></TH>");
  fprintf(of, "<TH width=\"10%%\"></TH>");
  fprintf(of, "<TH width=\"15%%\"></TH>");
  fprintf(of, "<TH width=\"10%%\"></TH>");
  fprintf(of, "<TH width=\"10%%\"></TH>");
  fprintf(of, "<TH width=\"20%%\"></TH>");
  fprintf(of, "<TH width=\" 5%%\"></TH>");
  fprintf(of, "<TH width=\"10%%\"></TH>");
  fprintf(of, "</TR>");
  fprintf(of, "<COLGROUP><COL SPAN=\"1\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"4\"></COLGROUP>");
  fprintf(of, "<COLGROUP><COL SPAN=\"3\"></COLGROUP>");
  fprintf(of, "</THEAD>\n");
  fprintf(of, "<TBODY>");
  if (RH.numg() > 0) {
    fprintf(of, "<TR ALIGN=\"right\"><TD>Acasã</TD>");
    fprintf(of, "<TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>-</TD><TD>%d</TD>",
      RH.numg(), RH.win, RH.drw, RH.los, RH.gsc, RH.gre);
    fprintf(of, "</TR>\n");
  }
  if (RA.numg() > 0) {
    fprintf(of, "<TR ALIGN=\"right\"><TD>Deplasare</TD>");
    fprintf(of, "<TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>-</TD><TD>%d</TD>",
      RA.numg(), RA.win, RA.drw, RA.los, RA.gsc, RA.gre);
    fprintf(of, "</TR>\n");
  }
  if (RN.numg()>0) {
  fprintf(of, "<TR ALIGN=\"right\"><TD>Neutru</TD>");
  fprintf(of, "<TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>-</TD><TD>%d</TD>",
    RN.numg(), RN.win, RN.drw, RN.los, RN.gsc, RN.gre);
  fprintf(of, "</TR>\n");
  }
  fprintf(of, "</TBODY>");
  fprintf(of, "<TBODY>");
  fprintf(of, "<TR  BGCOLOR=\"DDDDDD\" ALIGN=\"right\"><TD>Total</TD>");
  fprintf(of, "<TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>%d</TD><TD>-</TD><TD>%d</TD>",
    RT.numg(), RT.win, RT.drw, RT.los, RT.gsc, RT.gre);
  fprintf(of, "</TR>\n");
  fprintf(of, "</TBODY>");
  fprintf(of, "</TABLE>\n");

//  PlayerRanking(1);
  SortPlayerStats();
  HTMLPlayerStatsTable();

  fprintf(of, "</BODY>\n</HTML>\n");
  fclose(of);
}

void HTMLTeamRankingTable(Ranking *tr) {
  tr->bubbleSort(RULE_PTS);
  of = fopen("html/nat-vs.html", "wt");
  if (!of) {
    fprintf(stderr, "Error writing to file html/nat-vs.html...\n");
    return;
  }
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" width=\"50%%\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR BGCOLOR=\"CCCCCC\">\n");
  fprintf(of, "<TH WIDTH=\"5%%\">#</TH>");
  fprintf(of, "<TH WIDTH=\"35%%\">Împotriva</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Meciuri</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Victorii</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Egaluri</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\">Infrâng.</TH>");
  fprintf(of, "<TH WIDTH=\"15%%\" COLSPAN=\"3\">Golaveraj</TH>");
//  fprintf(of, "<TH WIDTH=\"1%%\"></TH>");
//  fprintf(of, "<TH WIDTH=\"4%%\">Gol-</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\">Puncte</TH>");
  fprintf(of, "<TH WIDTH=\"10%%\">Procentaj%%</TH>");
  fprintf(of, "</TR></THEAD>\n");
  for (int i=0; i<NN; ++i) {
    int t = tr->rank[i];
    Stat s = tr->S[t];
    int ng = s.numg();
    if (ng > 0) {
      fprintf(of, "\n<TR");
      if (i%2==1) fprintf(of, " BGCOLOR=\"EEEEEE\"");
      fprintf(of, ">");
      fprintf(of, "<TD>%d</TD>", i+1);
      fprintf(of, "<TD><A HREF=\"vs-39000/vs-39000-%d000.html\">%s</A></TD>", t, NameOf(CL, t*1000, 3000));
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.win);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.drw);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.los);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gsc);
      fprintf(of, "<TD>-</TD>");
      fprintf(of, "<TD ALIGN=\"left\">%d</TD>", s.gre);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*s.win+s.drw);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", (int)(100*s.pct()));
      fprintf(of, "</TR>\n");
    }
  }
  fprintf(of, "</TABLE>\n");
  fclose(of);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  char filename[256];
  NFY = 1922;
  NLY = 2016;

  if (!Load()) {
    printf("ERROR: called from invalid drectory.\n");
    return -1;
  }

  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-ly")==0 && k<argc-1)  {
      NLY  = atoi(argv[++k]);
    }
  }

  Pl = new Catalog();
  Pl->Load("players.dat");
  EPl = new Catalog();
  EPl->Load("europlayers.dat");
  NP  = Pl->Size();
  NEP = EPl->Size();
  Loc  = new Locations();
  ELoc = new Locations();
  Loc->Load("city.dat", "stadium.dat");
  ELoc->Load("eurocity.dat", "eurostadium.dat");
  C = new Ranking(NN);

  InitStats();

  for (int y = NFY; y<=NLY; y++) {
    LoadDB(y);
    LoadEvents(y);
  }

  for (int t=0; t<NN; t++) if (t!=CTTY_ROM/EURO) {
    ResetPlayerStats();
    fprintf(stderr, "Vs [%d] %s...\n", t+1, ctty[t]);
    VsPage(t);
  }

  HTMLTeamRankingTable(C);

  return 0;
}
