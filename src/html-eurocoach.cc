#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"
#include "alias.hh"

#define MAX_NAMES      2000
#define MAX_SEASONS     200

#define DB_ROWS         400
#define DB_COLS          60
#define DB_CELL          20

#define DB_HOME    0
#define DB_AWAY   1
#define DB_SCORE   2
#define DB_DATE    3
#define DB_COMP    4
#define DB_ROUND   5
#define DB_VENUE   6
#define DB_ATTEND  7
#define DB_WEATHER   8
#define DB_ROSTER1   9
#define DB_COACH1 31
#define DB_ROSTER2  32
#define DB_COACH2 54
#define DB_REF    55
#define DB_ASIST1 56
#define DB_ASIST2 57
#define DB_OBSERV 58
#define DB_T1    8
#define DB_T2   31

#define EURO 1000
#define MAX_LIST 30

char  db[MAX_SEASONS][DB_ROWS][DB_COLS][DB_CELL];
Catalog *Ant, *EAnt;
Ranking *R[MAX_NAMES];
Ranking *RR;
Ranking *ER;
int   qord[DB_ROWS], qd[DB_ROWS];
int   ord[MAX_SEASONS][DB_ROWS];
int mlist[MAX_NAMES][MAX_LIST];

const char* fxcol[] = {"F0F0B0", "AAFFAA", "FF8888"};
const char* ecln = "SCKU";
const char *cupmnem[] = {"SC", "CCE", "CC", "UEFA"};
const char *cupround[] = {"Câºtigãtoare", "Finala", "Semifinale", "Sferturi", "Optimi", "ªaisprezecimi", "1/32", "1/64"};
char ecupname[64], roundname[64];

int FY, LY;
int NK, NEC, NC, NT;
char **ctty;
char **dir;
int ns[MAX_NAMES], fs[MAX_NAMES], ls[MAX_NAMES];
int ens[MAX_NAMES], efs[MAX_NAMES], els[MAX_NAMES];
int ngm[MAX_SEASONS];

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

char *EuroName(int t, int nick, int year) {
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
      if (!tm) tm = tn;
    }
    j++;
  } while (ay<year && tok[j]!=NULL);
  if (nick) strcpy(ename, tm);
  else strcpy(ename, tn);
  return ename;
}


char *NameOf(Aliases **L, int t, int y) {
  if (t>=EURO) return EuroName(t,0,y);
  char *s = L[t]->GetName(y);
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
  if (t>=EURO) return EuroName(t,1,y);
  char *s = L[t]->GetNick(y);
  return s;
}

Aliases **L;
Aliases **CL;

//--------------------------------------

void SeasonName(int y, char *ss) {
  sprintf(ss, "%d/%02d", y-1, y%100);
}

void EcupMnem(char *sc, int year) {
  if (sc[0]=='I') {
    sprintf(ecupname, "CI");
  }
  else if (sc[0]=='0') {
    sprintf(ecupname, "SE");
  }
  else if (sc[0]=='1') {
    if (year<1992) sprintf(ecupname, "CCE");
    else sprintf(ecupname, "LC");
  }
  else if (sc[0]=='2') {
    sprintf(ecupname, "CC");
  }
  else if (sc[0]=='3') {
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
    sprintf(roundname, "Calificãri grupe");
  else if (sk[0]=='G')
    sprintf (roundname, "Grupa %c", sk[1]);
}

void toupper(char *s) {
  if (!s) return;
  int len = strlen(s);
  for (int i=0; i<len; ++i)
    if (s[i]>='a' && s[i]<='z') s[i] -= 32;
}

void tolower(char *s) {
  if (!s) return;
  int len = strlen(s);
  for (int i=0; i<len; ++i)
    if (s[i]>='A' && s[i]<='Z') s[i] += 32;
}

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;

  f = fopen("country.dat", "rt");
  if (!f) return 0;
  fscanf(f, "%d\n", &NK);
  CL = new Aliases*[NK];
  for (int i=0; i<NK; i++) CL[i] = new Aliases;
  ctty = new char*[NK];
  dir  = new char*[NK];
  for (int i=0; i<NK; i++) {
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

  f = fopen("webteams.dat", "rt");
  if (f==NULL) return 0;
  fscanf(f, "%d\n", &NT);
  L = new Aliases*[NT];
  for (int i=0; i<NT; i++) L[i] = new Aliases;
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return 0;
  for (int i=0; i<NT; i++) {
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

void LoadDB(int year) {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "euro-lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
	int y = year-FY;
	int n=0;
	while (!feof(f)) {
    fgets(s, 5000, f);
		if (strlen(s)<100) continue;
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(db[y][n][j], tk[j]);
      else strcpy(db[y][n][j], " ");
    }
		n++;
		s[0] = 0;
  }
	ngm[y] = n;
  fclose(f);
}

int NumericDate(char *s) {
	int dig[] = {1, 0, 4, 3, 9, 8};
	int x = 0;
	int expo = 1;
	for (int i=0; i<6; ++i) {
		x += expo*((int)s[dig[i]]-48);
		expo = expo*10;
	}
	return x;
}

void SortDB(int year) {
	int sorted;
	int y = year-FY;
	for (int i=0; i<ngm[y]; ++i) ord[y][i]=i;
	do {
		sorted = 1;
		for (int i=0; i<ngm[y]-1; ++i) {
			int j1 = ord[y][i];
			int j2 = ord[y][i+1];
			int d1 = NumericDate(db[y][j1][DB_DATE]);
			int d2 = NumericDate(db[y][j2][DB_DATE]);
			if (d2<d1) {
				sorted = 0;
				int aux = j1;
				ord[y][i] = j2;
				ord[y][i+1] = aux;
			}
		}
	} while (!sorted);
}

int compareDate(const void* x1, const void* x2) {
  int i1 = *(int*)x1;
  int i2 = *(int*)x2;
  return (qd[i1]-qd[i2]);
}

void qSortDB(int year) {
	int y = year-FY;
	for (int i=0; i<ngm[y]; ++i) {
		qd[i] = NumericDate(db[y][i][DB_DATE]);
		qord[i]=i;
	}
	qsort(qord, ngm[y], sizeof(int), compareDate);
	for (int i=0; i<ngm[y]; i++) ord[y][i] = qord[i];
}

void InitStats() {
	for (int i=0; i<MAX_NAMES; ++i) {
		ns[i]  = fs[i] = 0;
		ls[i]  = -1;
		ens[i] = efs[i] = 0;
		els[i] = -1;
    mlist[i][0] = 0;
	}
}

void CollectData(int year) {
	int y = year-FY;
	int co1, co2;
	int hm, aw, sc, x1, x2;
	for (int i=0; i<ngm[y]; ++i) {
		hm = atoi(db[year-FY][i][DB_HOME]);
		aw = atoi(db[year-FY][i][DB_AWAY]);
		sc = atoi(db[year-FY][i][DB_SCORE]);
		x1 = sc/100;
		x2 = sc%100;
		if (hm<EURO) {
			co1 = Ant->FindMnem(db[year-FY][i][DB_COACH1]);
			if (co1>=0) RR->S[co1].addRes(x1, x2);
			else fprintf(stderr, "ERROR [%d:%d]: %s not found.\n", year, i+1, db[year-FY][i][DB_COACH1]);
			if (ns[co1]==0) { fs[co1] = year; }
			if (year>ls[co1]) { ls[co1] = year; ns[co1]++; }
		}
		else {
			co1 = EAnt->FindMnem(db[year-FY][i][DB_COACH1]);
			if (co1>=0) ER->S[co1].addRes(x1, x2);
			else fprintf(stderr, "ERROR [%d:%d]: %s not found.\n", year, i+1, db[year-FY][i][DB_COACH1]);
			mlist[co1][0]++;
			mlist[co1][mlist[co1][0]] = 1000*y+i;
			if (ens[co1]==0) { efs[co1] = year; }
			if (year>els[co1]) { els[co1] = year; ens[co1]++; }
		}
		if (aw<EURO) {
			co2 = Ant->FindMnem(db[year-FY][i][DB_COACH2]);
			if (co2>=0) RR->S[co2].addRes(x2, x1);
			else fprintf(stderr, "ERROR [%d:%d]: %s not found.\n", year, i+1, db[year-FY][i][DB_COACH2]);
			if (ns[co2]==0) { fs[co2] = year; }
			if (year>ls[co2]) { ls[co2] = year; ns[co2]++; }
		}
		else {
			co2 = EAnt->FindMnem(db[year-FY][i][DB_COACH2]);
			if (co2>=0) ER->S[co2].addRes(x2, x1);
			else fprintf(stderr, "ERROR [%d:%d]: %s not found.\n", year, i+1, db[year-FY][i][DB_COACH2]);
			mlist[co2][0]++;
			mlist[co2][mlist[co2][0]] = 1000*y+i;
			if (ens[co2]==0) { efs[co2] = year; }
			if (year>els[co2]) { els[co2] = year; ens[co2]++; }
		}
	}
}

void HTMLTable(Ranking *R, Catalog *C, const char *filename, int eur) {
	FILE *of = fopen(filename, "wt");
	if (!of) { perror(""); return; }
	if (eur)
  fprintf(of, "<HTML>\n<TITLE>Antrenori adverºi în cupele europene (%d-%d)</TITLE>\n", FY, LY);
	else
  fprintf(of, "<HTML>\n<TITLE>Antrenori în cupele europene (%d-%d)</TITLE>\n", FY, LY);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

	if (eur)
  fprintf(of, "<H2>Antrenori adverºi în cupele europene (%d-%d)</H2>\n", FY, LY);
	else
  fprintf(of, "<H2>Antrenori în cupele europene (%d-%d)</H2>\n", FY, LY);
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
//  fprintf(of, "<TH>D.n.</TH>");
  fprintf(of, "<TH>Naþ.</TH>");
  fprintf(of, "<TH>Sez.</TH>");
  fprintf(of, "<TH>Primul</TH>");
  fprintf(of, "<TH>Ultimul</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Vict.</TH>");
  fprintf(of, "<TH>Egal.</TH>");
  fprintf(of, "<TH>Înfr.</TH>");
  fprintf(of, "<TH>G+</TH>");
  fprintf(of, "<TH></TH>");
  fprintf(of, "<TH>G-</TH>");
  fprintf(of, "<TH>P</TH>");
  fprintf(of, "<TH>%%</TH>");
  fprintf(of, "</TR></THEAD>\n");

	int cs = C->Size();
	for (int i=0; i<cs; ++i) {
		int co = R->rank[i];
		int ng = R->S[co].numg();
		Person p = C->P[co];
        makeHexlink(p.mnem);
		Stat   s = R->S[co];
		if (ng > 0) {
    	fprintf(of, "\n<TR");
    	if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    	fprintf(of, ">");
			fprintf(of, "<TD>%d</TD>", i+1);
			fprintf(of, "<TD>%s</TD>", p.pren);
			if (eur) {
  	    	  fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"euroantrenori/%s.html\">%s</A></TD>",
                p.name, p.pren, hexlink, p.name);
			}
			else {
	    	  fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"antrenori/%s.html\">%s</A></TD>",
                p.name, p.pren, hexlink, p.name);
			}
//			fprintf(of, "<TD ALIGN=\"right\">%s</TD>", p.dob);
			fprintf(of, "<TD>%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", p.cty, p.cty);
			if (eur) {
				fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ens[co]);
				fprintf(of, "<TD ALIGN=\"right\">%d</TD>", efs[co]);
				fprintf(of, "<TD ALIGN=\"right\">%d</TD>", els[co]);
			}
			else {
				fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ns[co]);
				fprintf(of, "<TD ALIGN=\"right\">%d</TD>", fs[co]);
				fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ls[co]);
			}
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.win);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.drw);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.los);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gsc);
			fprintf(of, "<TD>-</TD>");
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gre);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*s.win+s.drw);
			fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int)(100*s.pct()));
			fprintf(of, "</TR>\n");
		}
	}

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
	fclose(of);
}

void HTMLEuroCoach(int c) {
	char filename[256];
	Person p = EAnt->P[c];
    makeHexlink(p.mnem);
	sprintf(filename, "html/euroantrenori/%s.html", hexlink);
	FILE *of = fopen(filename, "wt");
	if (!of) { perror(filename); return; }

  fprintf(stderr, "%d.%s %s\n", c+1, p.pren, p.name);
  fprintf(of, "<HTML>\n<TITLE>%s %s</TITLE>\n", p.pren, p.name);
  fprintf(of, "<HEAD>\n<link href=\"../css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<TABLE CELLSPACING=\"10\" CELLPADDING=\"5\">\n<TR>\n<TD>");
  fprintf(of, "<H3>Antrenor <IMG SRC=\"../../../thumbs/22/3/%s.png\"></IMG> %s %s</H3>\n", p.cty, p.pren, p.name);
//  fprintf(of, "<UL><LI>Data naºterii: %s </LI>\n", p.dob);
//  fprintf(of, "<LI>Locul naºterii: %s\n", p.pob);
//  if (p.jud!=NULL && p.jud[0]!=0) { fprintf(of, " (%s)", p.jud); }
//  fprintf(of, "</LI>\n");
//  fprintf(of, "<LI>Debut: </LI>\n");
//  fprintf(of, "</UL>\n");
  fprintf(of, "</TR></TABLE>\n");

  fprintf(of, "<H3>Lista meciurilor</H3>\n");
  fprintf(of, "<TABLE cellpadding=\"1\" WIDTH=\"80%%\" RULES=\"groups\" frame=\"box\">\n");  
  fprintf(of, "<THEAD>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH WIDTH=\"20%%\"> Data@Ora </TH>");
  fprintf(of, "<TH WIDTH=\"25%%\"> Gazde </TH>");
  fprintf(of, "<TH WIDTH=\"5%%\"> Scor </TH>");
  fprintf(of, "<TH WIDTH=\"25%%\"> Oaspeþi </TH>");
  fprintf(of, "<TH WIDTH=\"10%%\"> Cupa </TH>");
  fprintf(of, "<TH WIDTH=\"15%%\"> Runda </TH>");
  fprintf(of, "</TR>");
  fprintf(of, "<TBODY>\n");
  int nm = mlist[c][0];
  for (int i=1; i<=nm; ++i) {
    int mid = mlist[c][i];
    int y = mid/1000;
    int year = FY+y;
    int k = mid%1000;
    int wxl = 0;
    fprintf(of, "<TR");
    if (i%2==1) fprintf(of, " BGCOLOR=\"FFFFFF\">"); else fprintf(of, " BGCOLOR=\"DDFFFF\">");
    int hid = atoi(db[y][k][DB_HOME]);
    int aid = atoi(db[y][k][DB_AWAY]);
    int scr = atoi(db[y][k][DB_SCORE]);
    int x1 = scr/100;
    int x2 = scr%100;
    if (hid<EURO) {
      if (x1>x2) wxl = 1;
      if (x1<x2) wxl = 2;
    }
    else {
      if (x1>x2) wxl = 2;
      if (x1<x2) wxl = 1;
    }
    fprintf(of, "<TD ALIGN=\"left\">%s</TD>", db[y][k][DB_DATE]);
    fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NameOf(L, hid, year));
    fprintf(of, "<TD BGCOLOR=\"%s\" ALIGN=\"center\"><A HREF=\"../reports/%d/e%d-%d.html\">%d-%d</A></TD>",
        fxcol[wxl], year, hid, aid, scr/100, scr%100);
    fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NameOf(L, aid, year));
    EcupMnem(db[y][k][DB_COMP], year);
    RoundName(db[y][k][DB_ROUND]);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", ecupname);
    fprintf(of, "<TD ALIGN=\"center\">%s</TD>", roundname);
    fprintf(of, "</TR>\n");
  }

  fprintf(of, "</TBODY>\n");
  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

int main(int argc, char **argv) {
  FY = 1957;
  LY = 2016;
  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-ly")==0 && k<argc-1)  {
      LY = atoi(argv[++k]);
    }
  }

	Load();

	Ant = new Catalog();
	Ant->Load("coaches.dat");
	NC = Ant->Size();

	EAnt = new Catalog();
	EAnt->Load("eurocoaches.dat");
	NEC = EAnt->Size();

	RR = new Ranking(NC);
	ER = new Ranking(NEC);

	InitStats();
  for (int year = FY; year<=LY; ++year) {
		LoadDB(year);
		SortDB(year);
		CollectData(year);
  }

	RR->bubbleSort(RULE_PTS);
	ER->bubbleSort(RULE_PTS);
	HTMLTable(RR, Ant, "html/antrenori-euro.html", 0);
	HTMLTable(ER, EAnt, "html/antrenori-euro-advers.html", 1);
//	for (int i=0; i<NC; ++i) HTMLCoach(i);
	for (int i=0; i<NEC; ++i) HTMLEuroCoach(i);
	return 0;
}
