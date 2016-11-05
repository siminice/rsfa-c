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

char  db[MAX_SEASONS][DB_ROWS][DB_COLS][DB_CELL];
int   qord[DB_ROWS], qd[DB_ROWS];
int   ord[MAX_SEASONS][DB_ROWS];
Catalog *Ant;
Ranking *R[MAX_NAMES];
Ranking *RR;
Ranking *cR;

int FY, LY;

int id[MAX_TEAMS], res[MAX_RR][MAX_TEAMS][MAX_TEAMS], rnd[MAX_RR][MAX_TEAMS][MAX_TEAMS];
int NC, NT, NK, ppv, tbr, pr1, pr2, rel1, rel2, r, z;
int hc[MAX_NAMES][MAX_SEASONS];
int ns[MAX_NAMES], fs[MAX_NAMES], ls[MAX_NAMES], nch[MAX_NAMES], nret[MAX_NAMES];
int ntm[MAX_SEASONS];
int champ[MAX_SEASONS], champ_mid[MAX_SEASONS], champ_coach[MAX_SEASONS];
int num_winter;
int *start_winter, *end_winter;
char **club;
char **mnem;

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
  f = fopen("part.a", "rt");
  if (f==NULL) return 0;
	while (!feof(f)) {
    fgets(s, 512, f);
		if (feof(f)) continue;
		char *tky = strtok(s, " \n");
		int year = atoi(tky);
		if (year>=FY && year<=LY) {
			char *tkn = strtok(NULL, " \n");
			char *tkc = strtok(NULL, " \n");
			if (tkc!=NULL)	{
				champ[year-FY] = atoi(tkc);
			}
		}
		s[0] = 0;
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

  for (k=0; k<MAX_RR; k++)
    for (i=0; i<NT; i++)
      for (j=0; j<NT; j++) { rnd[k][i][j] = res[k][i][j] = -1; }

	int cmid = -1;
	int lr = -1;
	int ld = -1;
  for (k=0; k<numr; k++) {
    for (i=0; i<NT; i++) {
      for (j=0; j<NT; j++) {
        fscanf(f, "%d %d", &r, &z);
        rnd[k][i][j] = r;
        res[k][i][j] = z;
				int rij = r/1000;
				int dij = r%1000;
				if (z>=0 && (id[i]==champ[y] || id[j]==champ[y])) {
					if (rij>lr) {
						cmid = 100*i+j;
						lr = rij;
						if (dij<400) ld = dij;
					}
					else if (rij==lr) {
						if (dij<400 && dij>ld) {
							ld = dij;
							cmid = 100*i+j;
						}
					}
				}
      }
      fscanf(f, "\n");
    }
  }
	int cmh = cmid/100;
	int cma = cmid%100;
	champ_mid[y] = (NT-1)*cmh+cma-(cma>cmh?1:0);
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
	int cmh = champ_mid[y]/(ntm[y]-1);
	int cma = champ_mid[y]%(ntm[y]-1);
	if (cma>=cmh) cma++;
	int chh = 1;
	if (champ[y]==id[cma]) chh=0;
	char *chc = db[y][champ_mid[y]][DB_COACH1];
	if (!chh) chc = db[y][champ_mid[y]][DB_COACH2];
	int cid = Ant->FindMnem(chc);
	if (cid>=0) {
	fprintf(stderr, "Sezon %d: campioanã %s, antrenor %s %s [cmid=%d].\n", year, NickOf(L, champ[y], year),
		Ant->P[cid].pren, Ant->P[cid].name, champ_mid[y]);
	}
	else {
		fprintf(stderr, "Sezon %d: campioanã %s, antrenor ??? [cmid=%d].\n", year, NickOf(L, champ[y], year), champ_mid[y]);
	}
	if (cid>=0) {
		champ_coach[y] = cid;
		nch[cid]++;
	}
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
		nch[i] = 0;
		for (int y=0; y<MAX_SEASONS; ++y) {
			hc[i][y] = 0;
		}
	}
}

void Add(int c, int year) {
	if (hc[c][year-FY]==0) {
		ns[c]++;
		if (fs[c]==0) fs[c] = year;
	}
	hc[c][year-FY] = 1;
	ls[c] = year;
}

void CollectData(int year) {
	for (int i=0; i<NT*(NT-1); ++i) {
		int co1 = Ant->FindMnem(db[year-FY][i][DB_COACH1]);
		int co2 = Ant->FindMnem(db[year-FY][i][DB_COACH2]);
//		if (co1 < 0) fprintf(stderr, "ERROR %d #%d: %s not found.\n", year, i, db[year-FY][i][DB_COACH1]);
//		if (co2 < 0) fprintf(stderr, "ERROR %d #%d: %s not found.\n", year, i, db[year-FY][i][DB_COACH2]);
		int sc = atoi(db[year-FY][i][DB_SCORE]);
		int hm = atoi(db[year-FY][i][DB_HOME]);
		int aw = atoi(db[year-FY][i][DB_GUEST]);
		int x = sc/100;
		int y = sc%100;
		if (co1>=0) {
			RR->S[co1].addRes(x, y);
			R[co1]->S[aw].addRes(x,y);
			Add(co1, year);
		}
		if (co2>=0) {
			RR->S[co2].addRes(y, x);
			R[co2]->S[hm].addRes(y,x);
			Add(co2, year);
		}
	}
}

void ListData() {
	char nume[256];
	RR->bubbleSort(RULE_NUMG);
	for (int i=0; i<NC; ++i) {
		int co = RR->rank[i];
		int ng = RR->S[co].numg();
		if (ng > 0) {
			sprintf(nume, "%s %s", Ant->P[co].pren, Ant->P[co].name);
			printf("%3d.%-32s %4d %3d %3d %3d %4d-%3d %4d\n", i+1,
				nume, ng,
				RR->S[co].win, RR->S[co].drw, RR->S[co].los, RR->S[co].gsc, RR->S[co].gre, (int)(100*RR->S[co].pct()));
		}
	}
}

void HTMLTable() {
	FILE *of = fopen("html/antrenori.html", "wt");
	if (!of) { perror(""); return; }
  fprintf(of, "<HTML>\n<TITLE>Antrenori în prima divizie (%d-%d)</TITLE>\n", FY, LY);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<H2>Antrenori în prima divizie (%d-%d)</H2>\n", FY, LY);
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
  fprintf(of, "<TH>Titluri</TH>");
  fprintf(of, "<TH>Retrogr.</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Vict.</TH>");
  fprintf(of, "<TH>Egal.</TH>");
  fprintf(of, "<TH>Înfr.</TH>");
  fprintf(of, "<TH>G+</TH>");
  fprintf(of, "<TH></TH>");
  fprintf(of, "<TH>G-</TH>");
  fprintf(of, "<TH>Pct</TH>");
  fprintf(of, "</TR></THEAD>\n");

	for (int i=0; i<NC; ++i) {
		int co = RR->rank[i];
		int ng = RR->S[co].numg();
		Person p = Ant->P[co];
		Stat   s = RR->S[co];
		if (ng > 0) {
    	fprintf(of, "\n<TR");
    	if (i%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    	fprintf(of, ">");
			fprintf(of, "<TD>%d</TD>", i+1);
			fprintf(of, "<TD>%s</TD>", p.pren);
        makeHexlink(p.mnem);
    	fprintf(of, "<TD ALIGN=\"left\" sorttable_customkey=\"%s,%s\"><A HREF=\"antrenori/%s.html\">%s</A></TD>",
          p.name, p.pren, hexlink, p.name);
			fprintf(of, "<TD ALIGN=\"right\">%s</TD>", p.dob);
			fprintf(of, "<TD>%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", p.cty, p.cty);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ns[co]);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", fs[co]);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ls[co]);
			if (nch[co]>0) {
  			  fprintf(of, "<TD ALIGN=\"right\"><B>%d</B></TD>", nch[co]);
			} else { fprintf(of, "<TD></TD>"); }
			if (nret[co]>0) {
  			  fprintf(of, "<TD ALIGN=\"right\">%d</TD>", nret[co]);
			} else { fprintf(of, "<TD></TD>"); }
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.win);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.drw);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.los);
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gsc);
			fprintf(of, "<TD>-</TD>");
			fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s.gre);
			fprintf(of, "<TD ALIGN=\"right\">%d%%</TD>", (int)(100*s.pct()));
			fprintf(of, "</TR>\n");
		}
	}

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
	fclose(of);
}

void HTMLStatLine(FILE *of, int nl, int year, int tm, Stat *s, int bold) {
		char ssn[32];
		SeasonName(year, ssn);
    fprintf(of, "\n<TR");
    if (nl%2==1) fprintf(of, " BGCOLOR=\"BBFFFF\"");
    fprintf(of, ">");
		fprintf(of, "<TD><A HREF=\"../a.%d-r1.html\">%s</A></TD>", year, (tm>=0?ssn:""));
		if (bold>0) {
		fprintf(of, "<TD><B>%s</B></TD>", (tm>=0?NickOf(L, tm, year):"Total"));
		} else { fprintf(of, "<TD>%s</TD>", (tm>=0?NickOf(L, tm, year):"Total")); };
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->numg());
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->win);
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->drw);
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->los);
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->gsc);
		fprintf(of, "<TD>-</TD>");
		fprintf(of, "<TD ALIGN=\"right\">%d</TD>", s->gre);
		fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int)(100*s->pct()));
		fprintf(of, "</TR>\n");
}

void HTMLCoach(int c) {
	char filename[256];
	Person p = Ant->P[c];
    makeHexlink(p.mnem);
	sprintf(filename, "html/antrenori/%s.html", hexlink);
	FILE *of = fopen(filename, "wt");
	if (!of) { perror(filename); return; }

	Stat *total = new Stat();
	Stat **curr  = new Stat*[5];
	for (int i=0; i<5; i++) curr[i] = new Stat();
	int nctm = 0;
	int ctm[5];
	int nl = 0;

	fprintf(stderr, "%d.%s %s\n", c+1, p.pren, p.name);
  fprintf(of, "<HTML>\n<TITLE>%s %s</TITLE>\n", p.pren, p.name);
  fprintf(of, "<HEAD>\n<link href=\"../css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<TABLE CELLSPACING=\"10\" CELLPADDING=\"5\">\n<TR>\n<TD>");
  fprintf(of, "<H3>Antrenor <IMG SRC=\"../../../thumbs/22/3/%s.png\"></IMG> %s %s</H3>\n", p.cty, p.pren, p.name);
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
  fprintf(of, "<TH>Echipã</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Vict.</TH>");
  fprintf(of, "<TH>Egal.</TH>");
  fprintf(of, "<TH>Înfr.</TH>");
  fprintf(of, "<TH>G+</TH>");
  fprintf(of, "<TH></TH>");
  fprintf(of, "<TH>G-</TH>");
  fprintf(of, "<TH>Pct</TH>");
  fprintf(of, "</TR></THEAD>\n");

	int ischamp = 0;

	for (int year=fs[c]; year<=ls[c]; ++year) {
		int y = year-FY;
		int ngm = ntm[y]*(ntm[y]-1);
		nctm  = 0;
		for (int i=0; i<ngm; ++i) {
			int j = ord[y][i];
			int co1 = Ant->FindMnem(db[y][j][DB_COACH1]);
			int co2 = Ant->FindMnem(db[y][j][DB_COACH2]);
			int sc = atoi(db[y][j][DB_SCORE]);
			int x = sc/100;
			int z = sc%100;
			int hm = atoi(db[y][j][DB_HOME]);
			int aw = atoi(db[y][j][DB_GUEST]);
			if (co1==c) {
				int k = 0; while (k<nctm && ctm[k]!=hm) k++;
				if (k>=nctm) ctm[nctm++] = hm;
				curr[k]->addRes(x, z);
			}
			if (co2==c) {
				int k = 0; while (k<nctm && ctm[k]!=aw) k++;
				if (k>=nctm) ctm[nctm++] = aw;
				curr[k]->addRes(z, x);
			}
		}
		if (nctm>0) nl++;
		for (int k=0; k<nctm; k++) {
			ischamp = 0;
			if (champ[y]==ctm[k] && champ_coach[y]==c) ischamp = 1;
			HTMLStatLine(of, nl, year, ctm[k], curr[k], ischamp);
			total->add(curr[k]);
			curr[k]->reset();
		}
	}
	HTMLStatLine(of, ++nl, ls[c], -1, total, 0);

  fprintf(of, "</TABLE>\n");

  fprintf(of, "\n<HR>\n");
  fprintf(of, "<H3>Palmaresul antrenorului %s %s împotriva altor echipe</H3>\n", p.pren, p.name);

  R[c]->bubbleSort(RULE_PTS);

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
  fprintf(of, "<TH>Pts</TH>");
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
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*s.win+s.drw);
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
	Ant = new Catalog();
	Ant->Load("coaches.dat");
	NC = Ant->Size();
  for (int i=0; i<NC; i++)  R[i] = new Ranking(NK);
	RR = new Ranking(NC);
	cR = new Ranking(NC);
	InitStats();
  for (int year = FY; year<=LY; ++year) {
		LoadSeason(year);
		LoadDB(year);
		qSortDB(year);
		CollectData(year);
  }
//	ListData();
	RR->bubbleSort(RULE_NUMG);
	HTMLTable();
	for (int i=0; i<NC; ++i) HTMLCoach(i);
	return 0;
}
