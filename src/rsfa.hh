#ifndef RSFA_UTIL_HH
#define RSFA_UTIL_HH

//Constants
#define NP         -1
#define UNKNOWN    -1
#define NOT_PLAYED -1
#define MAXLSIZE  100

// Ranking criteria
#define DEFAULT      0
#define HEAD_TO_HEAD 1
#define RATIO        2
#define NUMWINS      4
#define DRAW8        5
#define DRAW10       6
#define GDIFF2       7
#define GDIFF3       8
#define AWAY3        9
#define NUMORD      10

// Load syntax
#define FIXED  0
#define VAR    1

// Season parameters
#define ROUNDS     10

// Scores
#define AWD_HOME_00	90
#define AWD_HOME_10	91
#define AWD_HOME_20	92
#define AWD_HOME_30	93
#define AWD_HOME_40	94
#define AWD_HOME_50	95

#define AWD_GUEST_00	80
#define AWD_GUEST_01	81
#define AWD_GUEST_02	82
#define AWD_GUEST_03	83
#define AWD_GUEST_04	84
#define AWD_GUEST_05	85

#define AWD_BOTH_00	70
#define AWD_BOTH_10	71
#define AWD_BOTH_20	72
#define AWD_BOTH_30	73
#define AWD_BOTH_40	74
#define AWD_BOTH_50	75

#define CANCELED	99
#define SPECIAL         50

// Season listing
#define DEFAULT_FIRST_DATE -1
#define DEFAULT_LAST_DATE  -1
#define BRIEF      0
#define DETAILED   1

// Archive parameters
#define MAX_LEVELS 10
#define MEDALS     10

// Season format
#define REGULAR		 0
#define PLAYOFF		 1
#define APERTURA	10
#define CLAUSURA	20

//-----------------------------------
//--- Aliases
//-----------------------------------

struct alias_data {
  int   year;
  char *name;
  char *nick;
  alias_data(int, char*, char*);
  ~alias_data();
};

struct alias_node {
  alias_data *data;
  alias_node  *next;
  alias_node(alias_data*, alias_node*);
  ~alias_node();
};
  
struct Aliases {
  alias_node *list;
  Aliases();   
  ~Aliases();
  void Append(alias_data *a);
  char* GetName(int y);
  char* GetNick(int y);
};

struct Fed {
  int NC;
  char **club;
  char **mnem;
  Aliases **L;
  int Load(char* filename, int style = FIXED);
  char *NameOf(int, int);
  char *NickOf(int, int);
  int Find(char*);
  int GetUnique(char*);
};

struct History {
  Fed *F;
  int ND;	// number of levels
  int *MAX;	// maximum number of partiticipants (per level)
  int *FY;	// first year
  int *LY;	// last year
  int ****part;	// list of participants part[d][p][0] = number of teams
 private:
  int **med;

 public:
  History(Fed*);
  void Collect();
  void Build();
  void Load();
  int  MedSup(int, int, int);
  void CountMedals(int, int);
  void ShowMedals(int);
  int  In(int, int, int, int);
  void Head2Head(int, int);
  ~Archive();

};

struct League {
  Fed *F;
  char  *infile;
  int year;
  int format;
  int winter;

  int n;
  int ngr;
  int ppv;
  int tbr;
  int promo1;
  int promo2;
  int releg1;
  int releg2;

  int *id;
  int *win;
  int *drw;
  int *los;
  int *gsc;
  int *gre;
  int *pts;
  int *pen;
  int *pdt;
  int **ev;
  int *rank;

  int numr;
  int ***res;
  int ***rnd;

 private:
  int hla, hlb;
  int fd, ld;
  int lastr;
  int *tbwin, *tbdrw, *tblos, *tbgsc, *tbgre, *tbrk;

 public:
  League(Fed *F);
  int  Alloc(char *);
  void Clear();
  void Reset();
  int  Load(char *);
  int  Save(char *);
  int  sup(int, int, int);
  void Tiebreak(int, int);
  void Ranking();
  void Listing();
  void Synoptical();
  void Season(int det=DETAILED, int ifd=DEFAULT_FIRST_DATE, int ild=DEFAULT_LAST_DATE);
  int  InteractiveAdd(int, int, int, int, int);
  int  AddResult(int, int, int, int);
  int  DelResult(int, int, int, int);
  int  Find(char*);
  int  GetUnique(char*);
  void ListMostRecent();
};

//--------------------------------------

int after(int, int);
int consecutive(int, int);

#endif
