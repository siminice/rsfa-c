#include "catalog.hh"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void copytk(char **pf, char *tk) {
	if (tk==NULL) {
		*pf = new char[2];
		strcpy(*pf, " ");
	}
	else {
		*pf = new char[strlen(tk)+1];
		strcpy(*pf, tk);
	}
}

char low(char c) {
  if (c=='ª') return 'º';
  if (c=='Þ') return 'þ';
  if (c=='Î') return 'î';
  if (c>='A' && c<='Z') return c+32;
  return c;
}

/************************************/
Stat::Stat() {
	sez = champ = promo = releg = 0;
	win = drw = los = gsc = gre = 0;
}

void Stat::reset() {
	sez = champ = promo = releg = 0;
	win = drw = los = gsc = gre = 0;
}

int Stat::numg() {
	return win+drw+los;
}

double Stat::pct() {
	int ng = win+drw+los;
	if (ng==0) return 0.0;
	return (win+0.5*drw)/ng;
}

void Stat::addRes(int x, int y) {
	gsc += x; gre += y;
	if (x>y) ++win;
	else if (x==y) ++drw;
	else ++los;
}

void Stat::add(Stat *x) {
	sez += x->sez;
	champ += x->champ;
	promo += x->promo;
	releg += x->releg;
	win	+= x->win;
	drw += x->drw;
	los	+= x->los;
	gsc += x->gsc;
	gre += x->gre;
}

int Stat::sup(Stat *x, int rule) {
	int ng1 = win+drw+los;
	int ng2 = x->win+x->drw+x->los;
	if (rule==RULE_NUMG) {
		if (ng1>ng2) return -1;
		else if (ng1==ng2) return 0;
		else return 1;
	}
	if (rule==RULE_PCT) {
		double pc1 = pct();
		double pc2 = x->pct();
		if (pc1>pc2) return -1;
		if (pc1<pc2) return 1;
		if (ng1>ng2) return -1;
		else return 1;
	}
	int pt1 = 2*win+drw;
	int pt2 = 2*x->win+x->drw;
	if (pt1>pt2) return -1;
	if (pt1<pt2) return 1;
	if (pt1==0) {
		if (ng1>0  && ng2==0) return -1;
		if (ng1==0 && ng2>0)  return 1;
	}
	int gd1 = gsc-gre;
	int gd2 = x->gsc-x->gre;
	if (gd1>gd2) return -1;
	if (gd1<gd2) return 1;
	if (gsc>x->gsc) return -1;
	return 1;
}
/************************************/
PlayerStat::PlayerStat() {
  reset();
}

void PlayerStat::reset() {
	sez = fy = ly = champ = promo = releg = 0;
	win = drw = los = gsc = gre = 0;
    cap = min = tit = rez = ban = itg = cpt = 0;
    gol = pen = per = pea = aut = gop = 0;
    gal = ros = 0;
}

int PlayerStat::numg() {
	return win+drw+los;
}

double PlayerStat::pct() {
	int ng = win+drw+los;
	if (ng==0) return 0.0;
	return (win+0.5*drw)/ng;
}

void PlayerStat::addRes(int x, int y) {
	gsc += x; gre += y;
	if (x>y) ++win; else { if (x==y) ++drw; else ++los; }
}

void PlayerStat::addStat(PlayerStat *x) {
    sez = x->sez;
    fy = x->fy;
    ly = x->ly;
    champ = x->champ;
    promo = x->promo;
    releg = x->releg;

    win = x->win;
    drw = x->drw;
    los = x->los;
    gsc = x->gsc;
    gre = x->gre;

    cap = x->cap;
    min = x->min;
    tit = x->tit;
    rez = x->rez;
    ban = x->ban;
    itg = x->itg;
    cpt = x->cpt;

    gol = x->gol;
    pen = x->pen;
    per = x->per;
    pea = x->pea;
    aut = x->aut;
    gop = x->gop;

    gal = x->gal;
    ros = x->ros;
}

void addStats(char **ldb, char **edb, Catalog *p, int n) {
}

int PlayerStat::sup(PlayerStat *x, int rule) {
	int ng1 = win+drw+los;
	int ng2 = x->win+x->drw+x->los;
	if (rule==RULE_NUMG) {
		if (ng1>ng2) return -1;
		else if (ng1==ng2) return 0;
		else return 1;
	}
	if (rule==RULE_PCT) {
		double pc1 = pct();
		double pc2 = x->pct();
		if (pc1>pc2) return -1;
		if (pc1<pc2) return 1;
		if (ng1>ng2) return -1;
		else return 1;
	}
	int pt1 = 2*win+drw;
	int pt2 = 2*x->win+x->drw;
	if (pt1>pt2) return -1;
	if (pt1<pt2) return 1;
	if (pt1==0) {
		if (ng1>0  && ng2==0) return -1;
		if (ng1==0 && ng2>0)  return 1;
	}
	int gd1 = gsc-gre;
	int gd2 = x->gsc-x->gre;
	if (gd1>gd2) return -1;
	if (gd1<gd2) return 1;
	if (gsc>x->gsc) return -1;
	return 1;
}

/************************************/
Ranking::Ranking(int an) {
	n = an;
	S = new Stat[n];
	rank = new int[n];
	for (int i=0; i<n; ++i) {
		S[i].reset();
		rank[i] = i;
	}
}

void Ranking::reset() {
	for (int i=0; i<n; ++i) {
		S[i].reset();
		rank[i] = i;
	}
}

int Ranking::compare0(const void *c1, const void *c2) {
	int* i1 = (int*)c1;
	int* i2 = (int*)c2;
	return(S[*i1].sup(&S[*i2], 0));
}

int Ranking::compare1(const void *c1, const void *c2) {
	int* i1 = (int*)c1;
	int* i2 = (int*)c2;
	return(S[*i1].sup(&S[*i2], 1));
}

void Ranking::rqsort(int rule) {
/*
	int (*comp0)(const void*, const void*);
	int (*comp1)(const void*, const void*);
	comp0 = &compare0;
	comp1 = &compare1;
	if (rule==0)
		qsort(rank, n, sizeof(int), *comp0);
	else if (rule==1)
    qsort(rank, n, sizeof(int), *comp1);
*/
}

void Ranking::bubbleSort(int rule) {
	int sorted;
	do {
		sorted = 1;
		for (int i=0; i<n-1; ++i) {
			if (S[rank[i+1]].sup(&S[rank[i]], rule) < 0)	{
				int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
				sorted = 0;
			}
		}
	} while (!sorted);
}

/************************************/
int Catalog::Load(const char *filename) {
  n = 0;
  int c;
  for (c=0; c<257; c++) borna[c] = -1;
  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: Catalog %s not found.\n", filename);
    return -1;
  }

  char s[100], *t[10];
  fgets(s, 100, f);
  sscanf(s, "%d", &n);
  P = new Person[MAX_CATALOG];
  for (int i=0; i<n; i++) {
    fgets(s, 100, f);
    t[0] = strtok(s, ",\t\n");
    for (int j=1; j<10; j++) t[j] = strtok(NULL, ",\t\n");
    P[i].mnem = new char[7];
      strcpy(P[i].mnem, t[0]);
    P[i].name = new char[strlen(t[1])+1];
      strcpy(P[i].name, t[1]);
    P[i].pren = new char[strlen(t[2])+1];
      strcpy(P[i].pren, t[2]);
    P[i].nick = new char[strlen(t[1])+3];
    if (P[i].pren[0]!=0 && P[i].pren[0]!=' ') {
      sprintf(P[i].nick, "%c.%s", P[i].pren[0], P[i].name);
    } else {
      strcpy(P[i].nick, P[i].name);
    }

 		copytk(&(P[i].dob), t[3]);
 		copytk(&(P[i].cty), t[4]);
 		copytk(&(P[i].pob), t[5]);
 		copytk(&(P[i].jud), t[6]);

		if (i==0 || (i>0 && P[i-1].mnem[0]!=P[i].mnem[0])) {
			c = ((unsigned char) P[i].mnem[0]);
			borna[c] = i;
		}
	}
  fclose(f);

  c = ((unsigned char)P[n-1].mnem[0]);
  borna[c+1] = n;
  for (c=255; c>0; c--)
    if (borna[c]>=0 && borna[c-1]<0) borna[c-1] = borna[c];
	return 0;
}


int Catalog::FindMnem(char *s) {
  int c = ((unsigned char)s[0]);
  for (int i=borna[c]; i<borna[c+1]; i++)
    if (strcmp(P[i].mnem, s)==0) return i;
  return -1;
}

int Catalog::binFindMnem(char *s) {
  int c = ((unsigned char)s[0]);
	int lo = borna[c];
	int hi = borna[c+1]-1;
	int mid = (hi+lo)/2;
	while (lo<hi) {
		int comp = strcmp(P[mid].mnem, s);
    if (comp==0) return mid;
		if (comp>0) { hi = mid; } else { lo = mid+1; }
		mid = (lo+hi)/2;
	}
	if (strcmp(P[mid].mnem, s)==0) return mid;
  return -1;
}

int Catalog::FindNameUnique(char *sp, char *sn) {
	int exact = 0;
	int sub = 0;
	int loc = -1;
	for (int i=0; i<n; i++) {
		if (strcmp(P[i].name, sn)==0) {
			if (strcmp(P[i].pren, sp)==0) { exact++; loc = i; }
			else if (strstr(P[i].pren, sp)!=NULL) sub++;
		}
	}
	if (exact==1) return loc;
	return -1;
}

int Catalog::Add(char *sp, char *sn) {
  char s[5000], *tk[100];
  int ch, opt[1000], nopt, pa;

  nopt = 0;
  for (int i=0; i<n; i++) {
    if (strcmp(P[i].name, sn)==0) {
      opt[nopt] = i;
      fprintf(stdout, "   [%2d] %s: %-12s, %-20s [%10s]\n", nopt+2, P[i].mnem, P[i].name, P[i].pren, P[i].dob);
      nopt++;
    }
    else if (strstr(P[i].name, sn)!=NULL) {
      opt[nopt] = i;
      fprintf(stdout, "   [%2d] %s: %-12s, %-20s [%10s]\n", nopt+2, P[i].mnem, P[i].name, P[i].pren, P[i].dob);
      nopt++;
    }
  }

      int add = 0;
			fprintf(stdout, "...Name %s %s not found. [0]=cancel [1]=add [2-%d]=use ", sp, sn, nopt+2);
      scanf("%d", &add);
      if (add==1) {

        P[n].name = new char[strlen(sn)+1];
        strcpy(P[n].name, sn);
        P[n].pren = new char[strlen(sp)+1];
        strcpy(P[n].pren, sp);
        P[n].dob = new char[12];
        strcpy(P[n].dob, "00/00/0000");
        P[n].cty = new char[4];
        strcpy(P[n].cty, "ROM");
        P[n].pob = new char[2];
        strcpy(P[n].pob, " ");
        P[n].jud = new char[2];
        strcpy(P[n].jud, " ");

        P[n].mnem = new char[7];
        P[n].mnem[0] = low(P[n].name[0]);
        int d = 0; int k=1;
        while (k<4) {
          if (k>=strlen(sn)) P[n].mnem[k]='x'; 
          else if (P[n].name[k+d]==' ') {d++; k--;}
          else P[n].mnem[k] = P[n].name[k+d];
          k++;
        }
        if (sp!=NULL) {
          if (sp[0]==' ') P[n].mnem[4] = '_';
          else P[n].mnem[4] = low(P[n].pren[0]); 
        }
        else P[n].mnem[4] = '_';
        if (sp!=NULL) {
          if (strlen(sp)<2) P[n].mnem[5] = '_';
          else if (sp[1]==' ') P[n].mnem[5] = '_';
          else P[n].mnem[5] = low(P[n].pren[1]);
        }
        P[n].mnem[6] = 0;
        if (FindMnem(P[n].mnem)>=0) {
          fprintf(stderr, "...Mnemonic %s already in use.\n", P[n].mnem);
          int k=0;
          do {
            k++;
            P[n].mnem[5] = (char) (k+48);
          } while (FindMnem(P[n].mnem)>=0);
        }
        fprintf(stdout, ".Added entry #%d, %s: %s %s [%s] (%s, %s %s)\n", n+1,
			P[n].mnem, P[n].pren, P[n].name, P[n].dob, P[n].cty, P[n].pob, P[n].jud);
        n++;

      }
      else if (add>=2 && add<=nopt+1) {
/*
        pa = opt[add-2];
        fprintf(stderr, " Use #%d %s: %s, %s.\n", pa, pmnem[pa], pname[pa], ppren[pa]);
*/
			}
	return 1;
}

void Catalog::ForceAdd(Person p) {
  P[n].mnem = strdup(p.mnem);
  P[n].name = strdup(p.name);
  P[n].pren = strdup(p.pren);
  P[n].dob = strdup(p.dob);
  P[n].cty = strdup(p.cty);
  P[n].pob = strdup(p.pob);
  P[n].jud = strdup(p.jud);
  n++;
}

int Catalog::GetByLastname(char *s) {
  int opt[1000];
  int nopt = 0;
  int ch;
  for (int i=0; i<n; i++) {
    if (strcmp(s, P[i].name)==0) {
       opt[nopt++] = i;
    }
  }
  if (nopt==0) return -1;
  if (nopt==1) return opt[0];
  do {
    for (int i=0; i<nopt; i++) {
      fprintf(stderr, "\n... [%d]. %s %s \t[%10s] ", i+1,
       P[opt[i]].pren, P[opt[i]].name, P[opt[i]].dob);
    }
    scanf("%d", &ch);
  } while (ch<=0 || ch>nopt);
  return opt[ch-1];
}

int cMIN(int x, int y) {
  if (x<y) return x;
  else return y;
}

int cdist(char*a, char*b) {
  int i, j, modif, ins, del;
  int m = strlen(a);
  int n = strlen(b);
  int **d = new int*[m+1];
  for (i=0; i<=m; i++) {
    d[i] = new int[n+1];
    d[i][0] = i;
  }
  for (i=0; i<=n; i++)
    d[0][i] = i;

  for (i=1; i<=m; i++)
    for (j=1; j<=n; j++) {
      if (a[i-1] == b[j-1]) modif = d[i-1][j-1];
        else modif = d[i-1][j-1] + 1;
      del = cMIN(d[i-1][j], d[i][j-1]) + 1;
      d[i][j] = cMIN(modif, del);
    }

  int res = d[m][n];
  // cleanup;
  for (int i=0; i<=m; i++)
    delete[] d[i];
  delete[] d;
  return res;
}

int Catalog::BestMatch(char *sn, char *sp, int delta) {
  int opt[1000];
  int nopt = 0;
  int ch;
  for (int i=0; i<n; i++) {
    if (strcmp(sn, P[i].name)==0) {
       opt[nopt++] = i;
    }
  }
  if (nopt==1) return opt[0];

  for (int i=0; i<n; i++) {
    int d = cdist(P[i].name, sn);
    if (d>0 && d<=delta) {
       opt[nopt++] = i;
    }
  }

  if (nopt<1) return -1;
  do {
    for (int i=0; i<nopt; i++) {
      fprintf(stderr, "\n... [%d]. %s %s \t[%10s] ", i+1,
       P[opt[i]].pren, P[opt[i]].name, P[opt[i]].dob);
    }
    scanf("%d", &ch);
  } while (ch<=0 || ch>nopt);
  return opt[ch-1];
}

void Catalog::GetInitial(int i, char *pini) {
    pini[1] = '.'; pini[2] = 0;
    if (i<0 || P[i].pren==NULL || P[i].pren[0]==' ') pini[0] = 0;
    else if (strlen(P[i].pren)>0) pini[0] = P[i].pren[0];
    else pini[0] = 0;
}


int Catalog::Save(const char *filename) {
  char nfilename[128];
  char ofilename[128];
	sprintf(nfilename, "%s.new", filename);
	sprintf(ofilename, "%s.old", filename);
  FILE *f = fopen(nfilename, "wt");
	if (f==NULL) {
		fprintf(stderr, "ERROR: cannot open file %s.\n", nfilename);
		return -1;
	}
  fprintf(f, "%d\n", n);
  for (int i=0; i<n; i++) {
    fprintf(f, "%s,%s,%s,%s,%s,%s,%s\n", P[i].mnem, P[i].name, P[i].pren, P[i].dob, P[i].cty, P[i].pob, P[i].jud);
  }
  fclose(f);
  rename(filename, ofilename);
  rename(nfilename, filename);
	return 0;
}

/* ******************************************************* */

Alias::Alias(char *an, int afy, int aly) {
	int len = strlen(an);
	name = new char[len+1];
	strcpy(name, an);
	fy = afy;
	ly = aly;
	next = NULL;
}

char * Venue::getName(int year) {
	Alias *a = alias;
	while (a!=NULL) {
		if (a->fy<=year && year<=a->ly) return a->name;
		a = a->next;
	}
	return name;
}

/************************************/
int Locations::Load(const char *cfilename, const char *vfilename) {
  char s[1024], *tk[100];
  nc = 0;
  FILE *f = fopen(cfilename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: file %s not found.\n", cfilename);
    return -1;
  }
  fgets(s, 100, f);
  sscanf(s, "%d", &nc);
  C = new City[nc];
  for (int i=0; i<nc; i++) {
    fgets(s, 100, f);
    tk[0] = strtok(s, ",\t\n");
    for (int j=1; j<4; j++) tk[j] = strtok(NULL, ",\t\n");
 		copytk(&(C[i].mnem), tk[0]);
 		copytk(&(C[i].name), tk[1]);
 		copytk(&(C[i].jud), tk[2]);
	}
  fclose(f);

	char sc[5];
  nv = 0;
  f = fopen(vfilename, "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: file %s not found.\n", vfilename);
    return -1;
  }
  fgets(s, 10, f);
  sscanf(s, "%d", &nv);
	int y;
	Alias *a, *b;
  V = new Venue[nv];
  for (int i=0; i<nv; i++) {
    fgets(s, 1000, f);
    tk[0] = strtok(s, ",\t\n");
    for (int j=1; j<16; j++) tk[j] = strtok(NULL, ",\t\n");
 		copytk(&(V[i].mnem), tk[0]);
 		copytk(&(V[i].name), tk[1]);
    strncpy(sc, V[i].mnem, 4); sc[4] = 0;
		int ci = FindCity(sc);
	  V[i].city = ci;
 		if (tk[2]) V[i].capacity = atoi(tk[2]);
		if (tk[3]) V[i].built = atoi(tk[3]);
		a = new Alias(tk[1], V[i].built, 9999);
		V[i].alias = a;
		for (int j=4; tk[j]!=NULL; j+=2) {
			if (tk[j+1]) y = atoi(tk[j+1]); else y = 0;
			b = new Alias(tk[j], 0, y);
			a->fy = y+1;
			b->next = a;
			V[i].alias = b;
			a = b;
		}
/*
		if (ci>=0) fprintf(stderr, "Located venue %s in %s.\n", V[i].name, C[ci].name);
		else fprintf(stderr, "Cannot locate venue %s.\n", V[i].mnem);
*/
	}
  fclose(f);

	return 0;
}

int Locations::FindCity(char *s) {
  for (int i=0; i<nc; i++)
    if (strcmp(C[i].mnem, s)==0) return i;
  return -1;
}

int Locations::FindVenue(char *s) {
  for (int i=0; i<nv; i++)
    if (strcmp(V[i].mnem, s)==0) return i;
  return -1;
}
