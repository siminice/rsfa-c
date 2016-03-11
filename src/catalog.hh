#ifndef _CATALOG_HH
#define _CATALOG_HH

#define MAX_CATALOG 20000
#define	RULE_NUMG	0
#define	RULE_PCT	1
#define RULE_PTS	2

class City {
	public:
	char *mnem;
	char *name;
	char *jud;
};

class Alias {
	public:
		char *name;
		int fy;
		int ly;

	Alias *next;
	Alias(char*, int, int);
};

class Venue {
	public:
		char *mnem;
		char *name;
		int capacity;
		int built;
		int city;
	Alias *alias;
	char *getName(int);
};

class Person {
  public:
  char *mnem;
  char *name;
  char *pren;
  char *nick;
  char *dob;
  char *cty;
  char *jud;
  char *pob;
};

class Stat {
	public:
	int sez;
	int champ;
	int promo;
	int releg;
	int win;
	int drw;
	int los;
	int gsc;
	int gre;

	Stat();
	void reset();
	int numg();
	double pct();
	void add(Stat*);
	void addRes(int x, int y);
	int sup(Stat *x, int rule=0);
};

class PlayerStat {
  public:
    int sez;
    int fy;
    int ly;
    int champ;
    int promo;
    int releg;

    int win;
    int drw;
    int los;
    int gsc;
    int gre;

    int cap;
    int min;
    int tit;
    int rez;
    int ban;
    int itg;
    int cpt;

    int gol;
    int pen;
    int per;
    int pea;
    int aut;
    int gop;

    int gal;
    int ros;

    PlayerStat();
    void reset();
    int numg();
    double pct();
    void addStat(PlayerStat *x);
    void addStats(char **ldb, char **edb, Catalog *p, int n);
    void addRes(int x, int y);
    int sup(PlayerStat *x, int rule=0);
};

class Ranking {
	public:
	int n;
	Stat *S;
	int *rank;

	Ranking(): n(0) {};
	Ranking(int an);
	int Size() { return n; };
    void reset();
	void bubbleSort(int rule=0);
	int compare0(const void*, const void*);
	int compare1(const void*, const void*);
	void rqsort(int rule=0);
};

class Catalog {
  public:
  int n;
  int borna[256];
  Person *P;
  Catalog(): n(0) {};

	int Size() { return n; };
	int Load(const char *filename);
	int Save(const char *filename);
	int FindMnem(char *s);
	int binFindMnem(char *s);
	int FindNameUnique(char*, char*);
	int GetByLastname(char *);
	int BestMatch(char *sn, char *sp, int delta=2);
	void GetInitial(int i, char *pini);
	int Add(char*, char*);
	void ForceAdd(Person p);
};

class Locations {
	public:
	int nc, nv;
	City  *C;
	Venue *V;
	Locations(): nc(0), nv(0) {};

	int Load(const char *cfilename, const char *vfilename);
	int FindCity(char *cmnem);
	int FindVenue(char *vmnem);
};

#endif
