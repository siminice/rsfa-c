#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"

#define MAX_NAMES      2000
#define MAX_SEASONS     200

#define DB_ROWS         400
#define DB_COLS          60
#define DB_CELL          20

#define DB_HOME    0
#define DB_GUEST   1
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

#define NUM_ORD	10
#define MAX_RR	 3
#define MAX_TEAMS 24
#define ROSTER_SIZE 22

#define EV_COLS 30

char   db[MAX_SEASONS][DB_ROWS][DB_COLS][DB_CELL];
char  edb[MAX_SEASONS][DB_ROWS][DB_COLS][DB_CELL];
int   qord[DB_ROWS], qd[DB_ROWS];
int   ord[MAX_SEASONS][DB_ROWS];
Catalog *Arb;
Ranking *R[MAX_NAMES];
Ranking *RR;

int FY, LY;

int id[MAX_TEAMS], res[MAX_RR][MAX_TEAMS][MAX_TEAMS], rnd[MAX_RR][MAX_TEAMS][MAX_TEAMS];
int NR, NT, NK, tbr, ppv, pr1, pr2, rel1, rel2;
int ntm[MAX_SEASONS];
int hr[MAX_NAMES][MAX_SEASONS];
int ns[MAX_NAMES], fs[MAX_NAMES], ls[MAX_NAMES];
int rcenter[MAX_NAMES], rline[MAX_NAMES];
int num_winter;
int *start_winter, *end_winter;
char **club;
char **mnem;
char roster[2*ROSTER_SIZE][DB_CELL+1];
const char *evsymb = "'`\"/#!";
int rhyel[MAX_NAMES], rayel[MAX_NAMES];
int rhred[MAX_NAMES], rared[MAX_NAMES];
int rhpen[MAX_NAMES], rapen[MAX_NAMES];

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

char *NameOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetName(y);
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetNick(y);
  return s;
}

Aliases **L;

//--------------------------------------

int isWinter(int y) {
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
  fscanf(f, "%d\n", &NK);
  club = new char*[NK];
  mnem = new char*[NK];
  L = new Aliases*[NK];
  for (int i=0; i<NK; i++) L[i] = new Aliases;
  for (int i=0; i<NK; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[32];
    memmove(mnem[i], s, 15); mnem[i][15] = 0;
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return 0;
  for (int i=0; i<NK; i++) {
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

void LoadSeason(int year) {
  char filename[64];
  sprintf(filename, "a.%d", year);
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: file %s not found.\n", filename);
    return;
  }
  char s[500], *tok[12];
  int i, j, k;
	int winter = isWinter(year);
	int y = year-FY;
  fscanf(f, "%d %d %d %d %d %d %d\n", &NT, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
	ntm[y] = NT;
  int numr = tbr/NUM_ORD + 1;
  for (i=0; i<NT; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    id[i] = atoi(tok[0]);
  }
  fclose(f);
}

void LoadDB(int year) {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "lineups-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
	int y = year-FY;
  for (int i=0; i<NT*(NT-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<DB_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<DB_COLS; j++) {
      if (tk[j]!=NULL) strcpy(db[y][i][j], tk[j]);
      else strcpy(db[y][i][j], " ");
    }
  }
  fclose(f);
}

void LoadEvents(int year) {
  char filename[64], s[5000], *tk[DB_COLS];
  FILE *f;
  sprintf(filename, "events-%d.db", year);
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stderr, "ERROR: database %s not found.\n", filename); return; }
  int y = year - FY;
  for (int i=0; i<NT*(NT-1); i++) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    for (int j=1; j<EV_COLS; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<EV_COLS; j++) {
      if (tk[j]!=NULL) strcpy(edb[y][i][j], tk[j]);
      else strcpy(edb[y][i][j], " ");
    }
  }
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
  int ngm = ntm[y]*(ntm[y]-1);
	for (int i=0; i<ngm; ++i) ord[y][i]=i;
	do {
		sorted = 1;
		for (int i=0; i<ngm-1; ++i) {
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
  int ngm = ntm[y]*(ntm[y]-1);
	for (int i=0; i<ngm; ++i) {
		qd[i] = NumericDate(db[y][i][DB_DATE]);
		qord[i]=i;
	}
	qsort(qord, ngm, sizeof(int), compareDate);
	for (int i=0; i<ngm; i++) ord[y][i] = qord[i];
}

void InitStats() {
	for (int i=0; i<MAX_NAMES; ++i) {
		ns[i]  = fs[i] = 0;
		ls[i]  = -1;
		for (int y=0; y<MAX_SEASONS; ++y) {
			hr[i][y] = 0;
		}
		rcenter[i] = rline[i] = 0;
        rhyel[i] = rayel[i] = 0;
        rhred[i] = rared[i] = 0;
        rhpen[i] = rapen[i] = 0;
	}
}

void Add(int c, int year) {
	if (hr[c][year-FY]==0) {
		ns[c]++;
		if (fs[c]==0) fs[c] = year;
	}
	hr[c][year-FY] = 1;
	ls[c] = year;
}

void GetRoster(int y, int r) {
  char hpl[DB_CELL], apl[DB_CELL];
  for (int i=0; i<ROSTER_SIZE; i++) {
    strcpy(hpl, db[y-FY][r][DB_ROSTER1+i]);
    strcpy(apl, db[y-FY][r][DB_ROSTER2+i]);
    strncpy(roster[i], strtok(hpl, ":"), DB_CELL);
    strncpy(roster[i+ROSTER_SIZE], strtok(apl, ":"), DB_CELL);
  }
}

int RosterId(char *s) {
  if (!s) return -1;
  for (int i=0; i<2*ROSTER_SIZE; i++) {
     if (strcmp(roster[i], s)==0) return i;
  }
  return -1;
}

int Tid(char *s) {
  int rid = RosterId(s);
  if (rid<0) return -1;
  if (rid<ROSTER_SIZE) return 0;
  if (rid<2*ROSTER_SIZE) return 1;
  return -1;
}

void CollectData(int year) {
    char spl[DB_CELL];
	for (int i=0; i<NT*(NT-1); ++i) {
		int arb = Arb->FindMnem(db[year-FY][i][DB_REF]);
		int len = strlen(db[year-FY][i][DB_REF]);
		if (arb < 0) {
			if (len>0 && db[year-FY][i][DB_REF][0]!=' ') {
				fprintf(stderr, "ERROR %d #%d: %s not found.\n", year, i, db[year-FY][i][DB_REF]);
			}
		}
		int sc = atoi(db[year-FY][i][DB_SCORE]);
		int hm = atoi(db[year-FY][i][DB_HOME]);
		int aw = atoi(db[year-FY][i][DB_GUEST]);
		int x = sc/100;
		int y = sc%100;
		if (arb>=0) {
			Add(arb, year);
			rcenter[arb]++;
			RR->S[arb].addRes(x,y);
			R[arb]->S[hm].addRes(x, y);
			R[arb]->S[aw].addRes(y, x);
		}
        GetRoster(year, i);
        for (int j=0; j<EV_COLS; j++) {
          if (edb[year-FY][i][j]!=NULL && edb[year-FY][i][j][0]!=0 && edb[year-FY][i][j][0]!=' ') {
             strncpy(spl, edb[year-FY][i][j], 6);
             int tid = Tid(spl);
             char evt = edb[year-FY][i][j][6];
             if (tid>=0) {
               if (evt == 34 || evt == 47) {
                 if (tid==0) rhpen[arb]++;
                 if (tid==1) rapen[arb]++;
               }
             }
          }
        }
	}
}

void ListData() {
	char nume[256];
	RR->bubbleSort(RULE_NUMG);
	for (int i=0; i<NR; ++i) {
		int co = RR->rank[i];
		int ng = RR->S[co].numg();
		if (ng > 0) {
			sprintf(nume, "%s %s", Arb->P[co].pren, Arb->P[co].name);
			printf("%3d.%-32s %3d %4d [%4d%%]\n", i+1,
				nume, ns[co], ng, (int)(100*RR->S[co].pct()));
		}
	}
}

void HTMLTable() {
	FILE *of = fopen("html/arbitri.html", "wt");
	if (!of) { perror(""); return; }
  fprintf(of, "<HTML>\n<TITLE>Arbitri în prima divizie (%d-%d)</TITLE>\n", FY, LY);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<H2>Arbitri în prima divizie (%d-%d)</H2>\n", FY, LY);
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>D.n.</TH>");
  fprintf(of, "<TH>Naþ.</TH>");
  fprintf(of, "<TH>Sez.</TH>");
  fprintf(of, "<TH>Primul</TH>");
  fprintf(of, "<TH>Ultimul</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Central</TH>");
  fprintf(of, "<TH>Linie</TH>");
  fprintf(of, "<TH>Penalty</TH>");
  fprintf(of, "<TH>Cartonaºe roºii</TH>");
  fprintf(of, "<TH>%%Gazde</TH>");
  fprintf(of, "</TR></THEAD>\n");

	for (int i=0; i<NR; ++i) {
		int co = RR->rank[i];
		int ng = RR->S[co].numg();
		Person p = Arb->P[co];
		Stat   s = RR->S[co];
		if (ng > 0) {
    	fprintf(of, "\n<TR");
    	if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    	fprintf(of, ">");
			fprintf(of, "<TD>%d</TD>", i+1);
			fprintf(of, "<TD>%s</TD>", p.pren);
        makeHexlink(p.mnem);
    	fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"arbitri/%s.html\">%s</A></TD>",
        p.name, p.pren, hexlink, p.name);
			fprintf(of, "<TD ALIGN=\"right\">%s</TD>", p.dob);
			fprintf(of, "<TD>%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", p.cty, p.cty);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ns[co]);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", fs[co]);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ls[co]);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
			fprintf(of, "<TD ALIGN=\"right\">0</TD>");
			fprintf(of, "<TD ALIGN=\"right\">%d - %d</TD>", rhpen[co], rapen[co]);
			fprintf(of, "<TD ALIGN=\"right\"></TD>");
			fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int)(100*s.pct()));
			fprintf(of, "</TR>\n");
		}
	}

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
	fclose(of);
}

void HTMLStatLine(FILE *of, int r, int nl, Stat *s, int year) {
		char ssn[32];
		SeasonName(year, ssn);
    fprintf(of, "\n<TR");
    if (nl%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    fprintf(of, ">");
    fprintf(of, "<TD>%s</TD>", (year>0?ssn:""));
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->numg());
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->numg());
		fprintf(of, "<TD ALIGN=\"right\">0</TD>");
        if (year==0) {
  		  fprintf(of, "<TD ALIGN=\"right\">%d-%d</TD>", rhpen[r], rapen[r]);
        } else {
  		  fprintf(of, "<TD ALIGN=\"right\"></TD>");
        }
		fprintf(of, "<TD ALIGN=\"right\"></TD>");
		fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int)(100*s->pct()));
		fprintf(of, "</TR>\n");
}

void HTMLReferee(int c) {
	char filename[256];
	Person p = Arb->P[c];
    makeHexlink(p.mnem);
	sprintf(filename, "html/arbitri/%s.html", hexlink);
	FILE *of = fopen(filename, "wt");
	if (!of) { perror(filename); return; }

	Stat *total = new Stat();
	Stat *curr  = new Stat();

	fprintf(stderr, "%d.%s %s\n", c+1, p.pren, p.name);
  fprintf(of, "<HTML>\n<TITLE>%s %s</TITLE>\n", p.pren, p.name);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<TABLE CELLSPACING=\"10\" CELLPADDING=\"5\">\n<TR>\n<TD>");
  fprintf(of, "<H3>Arbitru <IMG SRC=\"../../../thumbs/22/3/%s.png\"></IMG> %s %s</H3>\n", p.cty, p.pren, p.name);
  fprintf(of, "<UL><LI>Data naºterii: %s </LI>\n", p.dob);
  fprintf(of, "<LI>Locul naºterii: %s\n", p.pob);
  if (p.jud!=NULL && p.jud[0]!=0) { fprintf(of, " (%s)", p.jud); }
  fprintf(of, "</LI>\n");
  fprintf(of, "<LI>Debut: </LI>\n");
  fprintf(of, "</UL>\n");
  fprintf(of, "</TR></TABLE>\n");

  fprintf(of, "<TABLE cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>Sezon</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Central</TH>");
  fprintf(of, "<TH>Linie</TH>");
  fprintf(of, "<TH>Penalty</TH>");
  fprintf(of, "<TH>Cartonaºe roºii</TH>");
  fprintf(of, "<TH>%%Gazde</TH>");
  fprintf(of, "</TR></THEAD>\n");

	int nl = 0;

	for (int year=fs[c]; year<=ls[c]; ++year) {
		int y = year-FY;
		int ngm = ntm[y]*(ntm[y]-1);
		for (int i=0; i<ngm; ++i) {
			int j = ord[y][i];
			int arb = Arb->FindMnem(db[y][j][DB_REF]);
			if (arb==c)	{
				int sc = atoi(db[y][j][DB_SCORE]);
				int x = sc/100;
				int z = sc%100;
				int hm = atoi(db[y][j][DB_HOME]);
				int aw = atoi(db[y][j][DB_GUEST]);
				curr->addRes(x,z);
			}
		}
		HTMLStatLine(of, c, ++nl, curr, year);
    total->add(curr);
    curr->reset();
	}
	HTMLStatLine(of, c, ++nl, total, 0);

  fprintf(of, "</TABLE>\n");

	fprintf(of, "\n<HR>\n");
	fprintf(of, "<H3>Palmaresul cluburilor în meciurile arbitrate de %s %s</H3>\n", p.pren, p.name);

	R[c]->bubbleSort(RULE_PCT);

  fprintf(of, "<script src=\"../../sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\" frame=\"box\">\n");
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Club</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Vict.</TH>");
  fprintf(of, "<TH>Egal.</TH>");
  fprintf(of, "<TH>Înfr.</TH>");
  fprintf(of, "<TH>G+</TH>");
  fprintf(of, "<TH></TH>");
  fprintf(of, "<TH>G-</TH>");
  fprintf(of, "<TH>Pct</TH>");
  fprintf(of, "</TR></THEAD>\n");

  for (int i=0; i<NK; ++i) {
    int t = R[c]->rank[i];
    Stat s = R[c]->S[t];
    int ng = s.numg();
    if (ng > 0) {
      fprintf(of, "\n<TR");
      if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
      fprintf(of, ">");
      fprintf(of, "<TD>%d</TD>", i+1);
			fprintf(of, "<TD>%s</TD>", NameOf(L, t, 3000));
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.win);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.drw);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.los);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gsc);
      fprintf(of, "<TD>-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gre);
      fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int)(100*s.pct()));
      fprintf(of, "</TR>\n");
    }
  }
  fprintf(of, "</TABLE>\n");

  fprintf(of, "</BODY>\n</HTML>");
	fclose(of);
}

int main(int argc, char **argv) {
  FY = 1933;
  LY = 2016;
  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-ly")==0 && k<argc-1)  {
      LY = atoi(argv[++k]);
    }
  }
	Load();
	Arb = new Catalog();
	Arb->Load("referees.dat");
	NR = Arb->Size();
	for (int i=0; i<NR; i++)	R[i] = new Ranking(NK);
	RR = new Ranking(NR);
	InitStats();
  for (int year = FY; year<=LY; ++year) {
		LoadSeason(year);
		LoadDB(year);
		LoadEvents(year);
		qSortDB(year);
		CollectData(year);
  }
//	ListData();

	RR->bubbleSort(RULE_NUMG);
	HTMLTable();
	for (int i=0; i<NR; ++i) HTMLReferee(i);

	return 0;
}
