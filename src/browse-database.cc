// completely messed up program to perform queries on 
// an artisanal database

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CLUBS    1000
#define NAME_LENGTH    40

#define TOTAL 0
#define PCT   1

#define NORMAL 0
#define YELLOW 1
#define CYAN   2

const char *color[] = {"\033[0m", "\033[33;1m", "\033[36;1m"};

const char *normal = "\033[0m";
const char *yellow = "\033[33;1m";
const char *cyan   = "\033[36;1m";

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
  delete name;
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

typedef int (Comp)(int*, int*, int*);

class Literal {
 public:
  Comp* fn;
  int*  x;
  int*  a;
  int*  b;
  Literal();
  int isValid();
  void Print();
};

class Filter {
 public:
  Literal L[12];
  Filter();
  int  Get();
  void Reset(int,int);
  void Print();
  int  Check();
};

class Entry {
 public:
  int  year;
  int  month;
  int  day;
  int  division;
  int  pool;
  int  round;
  int  home;
  int  guest;
  int  score[8];
  int  gdiff;
  Entry();
  void Copy(Entry*);
  void Decode(unsigned char*);
};

struct Board {
  int  win;
  int  drw;
  int  def;
  int  gsc;
  int  gre;
  void Print(int);
};

// global variables

const char* Month[] ={"???", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char **club;
char **mnem;
Entry  E, X, Y;
int    NT, NC;
Filter F;
int    RT;
int    all;

int    nst;
int    who[MAX_CLUBS];
int    rank[MAX_CLUBS];
Board  B[MAX_CLUBS];
const  char*  outputf = "output.html";
FILE*  outf;
char   **data;
int    M;
bool   hit, rec;

int GetUniqueCity(const char*);
char* NameOf(Aliases**,int,int);
char* NickOf(Aliases**,int,int);
void Load();

int GetUniqueClub(const char* prompt) {
  int x = -1;
  char buf[20];
  printf("%s: ", prompt);
  getc(stdin); gets(buf);
  // to upper case
  if (buf[0] >= 'a' && buf[0] <= 'z') buf[0] -= 32;
  for (int i=0; i<NT; i++)
    if (strcmp(mnem[i], buf) == 0) return i;  
  for (int i=0; i<NT; i++)
   if (strstr(mnem[i], buf) != NULL) return i;
  return -1;
}


//-----------------------------------
// Comparison functions
//-----------------------------------
int Any(int* x, int* a, int* b) {
 return 1;
};
int In(int* x, int* a, int* b) {
 return (*x >= *a && *x <= *b);
};
int Eq(int* x, int* a, int* b) {
 return (*x == *a);
};
int Dif(int* x, int* a, int* b) {
 return (*x != *a);
};
int Gt(int* x, int* a, int* b) {
 return (*x > *a);
};
int Lt(int* x, int* a, int* b) {
 return (*x < *a);
};

//-----------------------------------
// class Literal
//-----------------------------------

Literal::Literal() {
  fn = &Any;
  a  = new int;
  b  = new int;
};

int Literal::isValid() {
  return (*fn)(x, a, b);
};

void Literal::Print() {
  if (fn == &Any) printf("Any\n");
  else if (fn == &Eq) printf (" == %d\n", *a);
}

//-----------------------------------
//  class Board
//-----------------------------------

void ResetBoard() {
  int i;
  for (i=0; i<NT; i++) {
    B[i].win = 0;
    B[i].drw = 0;
    B[i].def = 0;
    B[i].gsc = 0;
    B[i].gre = 0;
  }
  nst = 0;
}

int PPV = 2;

bool Sup(int x, int y, Board* Bd) {
  int p1 = PPV*Bd[x].win + Bd[x].drw;
  int p2 = PPV*Bd[y].win + Bd[y].drw;
  int m1 = Bd[x].win + Bd[x].drw + Bd[x].def;
  int m2 = Bd[y].win + Bd[y].drw + Bd[y].def;
  if (RT==PCT) {
    double q1 = (m1==0? 0.0 : ((double)p1)/((double)m1));
    double q2 = (m2==0? 0.0 : ((double)p2)/((double)m2));
    if (q1 > q2) return 1;
    if (q1 < q2) return 0;
  }
  if (p1 > p2) return true;
  if (p1 < p2) return false;
//  if (p1 == 0) {
    int gm1 = Bd[x].win+Bd[x].drw+Bd[x].def;
    int gm2 = Bd[y].win+Bd[y].drw+Bd[y].def;
    if (gm1==0 || gm2==0) return (x<y);
    double dd1 = ((double) (Bd[x].gsc-Bd[x].gre))/((double) gm1);
    double dd2 = ((double) (Bd[y].gsc-Bd[y].gre))/((double) gm2);
    if (dd1>dd2) return true;
    if (dd1<dd2) return false;
//  }
  int d1 = Bd[x].gsc - Bd[x].gre;
  int d2 = Bd[y].gsc - Bd[y].gre;
  if (d1 > d2) return true;
  if (d1 < d2) return false;
  if (Bd[x].win > Bd[y].win) return true;
  if (Bd[x].win < Bd[y].win) return false;
  if (Bd[x].gsc > Bd[y].gsc) return true;
  if (Bd[x].gsc < Bd[y].gsc) return false;
  if (x < y) return true;
  return false;
}

void RankBoard(Board *Bd, int max) {
  int i, sorted, aux;
  for (i=0; i<max; i++) rank[i] = i;
  do {
    sorted = 1;
    for (i = 0; i < max-1; i++)
      if (Sup(rank[i+1], rank[i], Bd)) {
        sorted = 0;
        aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
  } while (!sorted);
}

void ListBoard(Board *Bd, int max, int *w) {
  if (max > 0) {
    printf("\n\n");
//    fprintf(outf, "<PRE>\n");
    RankBoard(Bd, max);
    for (int i=0; i<max; i++) {
      int x = rank[i];
      printf("%3d.%-30s ", i+1, NameOf(L, w[x], E.year));
//      fprintf(outf, "%3d.%-30s ", i+1, NameOf(L, w[x], E.year));
      Bd[x].Print(x);
    }
//    fprintf(outf, "</PRE>\n");
  }
  else printf("\nNo matching data found.\n");
}

void Board::Print(int x) {
  printf("%4d  %4d %4d %4d  %4d-%4d  %4dp [%4.3f]\n",
   win+drw+def, win, drw, def, gsc, gre, PPV*win+drw, 
     (win+drw+def==0?0.0:((double)2*win+drw)/((double)2*(win+drw+def))));
//  fprintf(outf, "%4d  %4d %4d %4d  %4d-%4d  %4dp\n", 
//          win+drw+def, win, drw, def, gsc, gre, PPV*win+drw);
}

//-----------------------------------
//  class Filter 
//-----------------------------------

Filter::Filter() { };

void Filter::Print() {
  printf("\n[1]-Home: ");
   if (L[0].fn == &Eq) printf("%s\n", club[*L[0].a]);
   else printf("*\n");
  printf("[2]-Guest: ");   
   if (L[1].fn == &Eq) printf("%s\n", club[*L[1].a]);
   else printf("*\n");
  printf("[3]-Edition ");
   if (L[2].fn == &Any) printf("\n");
   else if (L[2].fn == &In) printf("%4d-%-7d\n", *L[2].a, *L[2].b);
   else if (L[2].fn == &Lt) printf("<%-11d\n", *L[2].a);
   else if (L[2].fn == &Gt) printf(">%-11d\n", *L[2].a);
   else if (L[2].fn == &Eq) printf("%-12d\n", *L[2].a);
  printf("[4]-Tier:  ");
   if (L[3].fn == &Any) printf("\n");
   else if (L[3].fn == &In) printf("%c-%c\n", *L[3].a, *L[3].b);
   else if (L[3].fn == &Lt) printf("<%c\n", *L[3].a);
   else if (L[3].fn == &Gt) printf(">%c\n", *L[3].a);
   else if (L[3].fn == &Eq) printf("%c\n", *L[3].a);
  printf("[5]-Pool ");
   if (L[4].fn == &Any) printf("\n");
   else printf("%d\n", *L[4].a);
  printf("[6]-Round ");
   if (L[5].fn == &Any) printf("\n");
   else if (L[5].fn == &In) printf(" %d-%d\n", *L[5].a, *L[5].b);
   else if (L[5].fn == &Lt) printf("< %d\n", *L[5].a);
   else if (L[5].fn == &Gt) printf("> %d\n", *L[5].a);
   else if (L[5].fn == &Eq) printf("= %d\n", *L[5].a);
  printf("[7]-Goal differece ");
   if (L[6].fn == &Any) printf("\n");
   else if (L[6].fn == &In) printf(" %d-%d\n", *L[6].a, *L[6].b);
   else if (L[6].fn == &Lt) printf("< %d\n", *L[6].a);
   else if (L[6].fn == &Gt) printf("> %d\n", *L[6].a);
   else if (L[6].fn == &Eq) printf("= %d\n", *L[6].a);
  printf("[8]-Criteria (%s) [9]-Reset [10]-Exit\n\n", (RT==0?"total":"pct%"));
}

void Filter::Reset(int l1, int l2) {
 for (int i=l1; i<=l2; i++)
    L[i].fn = &Any;
   *(L[0].a) = -1;
}

int Filter::Get() { 
  int x, p;
  int option = 1;
  int res = 1;
  char s[5];

//  Reset(0,2);
  while (option) {
    Print();
    scanf("%d", &option);
    switch(option) {
      case 1:
        x = GetUniqueClub("Home");
        *(L[0].a) = x;
        if (x>=0) L[0].fn = &Eq; else L[0].fn = &Any;
        break;
      case 2:
        x = GetUniqueClub("Guest");
        *(L[1].a) = x;
        if (x>=0) L[1].fn = &Eq; else L[1].fn = &Any;
        break;
      case 3:
        printf("[<]-Before [=]-Exact [>]-After [;]-Between "); scanf("%s", s);
        if (s[0] == '=') L[2].fn = &Eq;
        else if (s[0] == '<') L[2].fn = &Lt;
        else if (s[0] == '>') L[2].fn = &Gt;
        else if (s[0] == ';') L[2].fn = &In;
        else {L[2].fn = &Any;}
        printf("\n Edition:"); scanf("%d", L[2].a);
        if (L[2].fn == &In) {
          printf("\n Until Edition:"); scanf("%d", L[2].b);
        }
        break;
      case 4:
        printf("\n Division [<=>][A-L]:");
        scanf("%s", s);
        s[1] = toupper(s[1]);
        s[2] = toupper(s[2]);
        if (s[0] == '=') L[3].fn = &Eq;
        else if (s[0] == '<') L[3].fn = &Lt;
        else if (s[0] == '>') L[3].fn = &Gt;
        else if (s[0] == ';') L[3].fn = &In;
        else {L[3].fn = &Any;}
        if(s[1]>='A' && s[1]<='L') *(L[3].a) = s[1];
        else L[3].fn = &Any;
        if(L[3].fn==&In) {
          if (s[2]>='A' && s[2]<='L') *(L[3].b) = s[2];
          else L[3].fn = &Any;
        }
        break;
      case 5:
        printf("\n Pool:");
        scanf("%d", &p);
        L[4].fn = &Eq;
        *(L[4].a) = p;
        if (p<=0) L[4].fn = &Any;
        break;
      case 6:
        printf("[<]-Further [=]-Exact [>]-Before [;]-Between "); scanf("%s", s);
        if (s[0] == '=') L[5].fn = &Eq;
        else if (s[0] == '<') L[5].fn = &Lt;
        else if (s[0] == '>') L[5].fn = &Gt;
        else if (s[0] == ';') L[5].fn = &In;
        else {L[5].fn = &Any;}
        scanf("%d", L[5].a);
        if (L[5].fn == &In) {
          scanf("%d", L[5].b);
        }
        break;
      case 7:
        printf("[<=>;] "); scanf("%s", s);
             if (s[0] == '=') L[6].fn = &Eq;
        else if (s[0] == '<') L[6].fn = &Lt;
        else if (s[0] == '>') L[6].fn = &Gt;
        else if (s[0] == ';') L[6].fn = &In;
        else {L[6].fn = &Any;}
        scanf("%d", L[6].a);
        if (L[6].fn == &In) {
          scanf("%d", L[6].b);
        }
        break;
      case 8:
        printf("\n Ranking Criteria [0=total, 1=pct]:");
        scanf("%d", &RT);
        if (RT!=0) RT = 1;
        break;
      case 9:
        Reset(0,10);
        break;
      case 11:
        for (int j=0; j<M; j++)
          if (data[j]!=NULL) delete data[j];
        if (data) delete data;
        Load();
        break;
      case 10:
        res = 0;
      default: option = 0;
    }
  }
  Print();
  return res;
};

int  Filter::Check() { 
  Y.Copy(&X);
  int aux, res;
  Comp *f;
  L[0].x = &(X.home);
  L[1].x = &(X.guest);
  L[2].x = &(X.year);
  L[3].x = &(X.division);
  L[4].x = &(X.pool);
  L[5].x = &(X.round);
  L[6].x = &(X.gdiff);
  res = L[0].isValid() && L[1].isValid();
  if (!res) {
    aux = *L[0].a; *L[0].a = *L[1].a; *L[1].a = aux;
    f = L[0].fn; L[0].fn = L[1].fn; L[1].fn = f;
    res = res || (L[0].isValid() && L[1].isValid());
  }
  if (res) for (int l=2; l<7; l++) res = res && L[l].isValid();
  return res;
};


//-------------------------------
// class Entry
//-------------------------------

Entry::Entry() {
 // nothing to do !
};

void Entry::Copy(Entry *e) {
  year     = e->year;
  month    = e->month;
  day      = e->day;
  division = e->division;
  pool     = e->pool;
  round    = e->round;
  home     = e->home;
  guest    = e->guest;
  for (int i=0; i<8; i++)
    score[i] = e->score[i];
}

void Entry::Decode(unsigned char* s) {
  int i;
  year     = 75*(s[0]-48) + (s[1]-48) + 1870;
  month    = s[2] - 48;
  day      = s[3] - 48;
  if (day<0) day = 0;
  division = s[4];
  pool     = s[5] - 48;
  round    = (int) (s[6] - 48);
  home     = 75*((int)(s[7]-48)) + s[8] - 48;
  guest    = 75*((int)(s[9]-48)) + s[10] - 48;
  int len  = strlen((char *)s);
  for (i=0; i<8; i++)
    if (i+11 < len) score[i] = s[i+11] - 48;
    else score[i] = -1;
  gdiff    = score[0] - score[1];
};

void AddGlobal() {
  int i = 0;
  bool found = false;
  while (i<nst && !found) {
    if (who[i] == X.home) found = true;
    else i++;
  }
  if (!found) who[nst++]=X.home; 
  int id1 = i;
  i = 0;
  found = false;
  while (i<nst && !found) {
    if (who[i] == X.guest) found = true;
    else i++;
  }
  if (!found) who[nst++]=X.guest;
  int id2 = i;
  B[id1].gsc += X.score[0];
  B[id1].gre += X.score[1];
  B[id2].gsc += X.score[1];
  B[id2].gre += X.score[0];
  if (X.score[0] > X.score[1]) {
    B[id1].win++;
    B[id2].def++;
  }
  else if (X.score[0] == X.score[1]) {
    B[id1].drw++;
    B[id2].drw++;
  }
  else {
    B[id1].def++;
    B[id2].win++;
  }
}

void PadBoard() {
  int bb[MAX_CLUBS];
  for (int i=0; i<MAX_CLUBS; i++)  bb[i] = 0;
  for (int i=0; i<nst; i++) bb[who[i]] = 1;
  int j = nst;
  for (int i=0; i<NT; i++) {
    if (bb[i]==0) who[j++] = i;
  }
}

//-----------------------------------
//  Global procedures
//-----------------------------------


char *NameOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetName(y);
  if (!s) return club[t];
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetNick(y);
  if (s==NULL) return mnem[t];
  return s;
}

void Load() {
 FILE *f;
 int k;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NT);
  club = new char*[NT];
  mnem = new char*[NT];
  L = new Aliases*[NT];
  for (int i=0; i<NT; i++) L[i] = new Aliases;
  for (int i=0; i<NT; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[32];
    memmove(mnem[i], s, 15); 
      k=14; while (mnem[i][k] == ' ') { mnem[i][k--]=0; }
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
  }
  fclose(f);

  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NT; i++) {
    fgets(s, 2000, f);
    if (!s) continue;
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
  }
  fclose(f);

  f = fopen("archive.dat", "rt");
  if (!f) exit(0);
  M = 0;
  while(!feof(f)) {
    fgets(s, 100, f);
    if (strlen(s) > 2) M++;
    s[0] = 0;
  }
  fclose(f);

  data = new char*[M];

  f = fopen("archive.dat", "rt");
  for (int j=0; j<M; j++) {
    fgets(s, 100, f);
    int len = strlen(s);
    s[len-1] = 0;
    data[j] = new char[len];
    strcpy(data[j], s);
  }
  fclose(f);
  printf("Loaded %d official games.\n", M);
};


void Extract() {
  int r;
  if (X.year != E.year) {
    printf("\n__________________________________________________________________________________________________________\n");
//    fprintf(outf, "<HR><HR>\n");
  }
//  switch(X.round) {
//    case 'P': r = 0; break;
//    case 'O': r = 1; break;
//    case 'Q': r = 2; break;
//    case 'S': r = 3; break;
//    case 'F': r = 4; break;
//  }
//  printf(", %s", Round[r]); 
//  fprintf(outf, ", %s", Round[r]);
  printf("\n%2d.%s.%d", X.day, Month[(X.month-1)%12+1], X.year - 1 + (X.month/13));
//  fprintf(outf, "<BR>%2d.%s.%d", X.day, Month[X.month], X.year);
  printf(" %c%c [%2d]", X.division, X.pool>0?((char)(X.pool+48)):' ', X.round);
//  fprintf(outf, " %c%c", X.division, X.pool);

  int ch = NORMAL;
  int cg = NORMAL;

  if (X.score[0]>X.score[1]) {
    if ((F.L[0].fn==&Eq && F.L[1].fn!=&Eq && *(F.L[0].a)==X.home) ||
        (F.L[1].fn==&Eq && F.L[0].fn!=&Eq && *(F.L[1].a)==X.home))
      ch = YELLOW;
    if ((F.L[0].fn==&Eq && F.L[1].fn!=&Eq && *(F.L[0].a)!=X.home) ||
        (F.L[1].fn==&Eq && F.L[0].fn!=&Eq && *(F.L[1].a)!=X.home))
      ch = CYAN;
    if (F.L[0].fn==F.L[1].fn) {
      if (X.home>X.guest) ch=YELLOW; else ch=CYAN;
    }
  }

  if (X.score[0]<X.score[1]) {
    if ((F.L[0].fn==&Eq && F.L[1].fn!=&Eq && *(F.L[0].a)==X.guest) ||
        (F.L[1].fn==&Eq && F.L[0].fn!=&Eq && *(F.L[1].a)==X.guest))
      cg = YELLOW;
    if ((F.L[0].fn==&Eq && F.L[1].fn!=&Eq && *(F.L[0].a)!=X.guest) ||
        (F.L[1].fn==&Eq && F.L[0].fn!=&Eq && *(F.L[1].a)!=X.guest))
      cg = CYAN;
    if (F.L[0].fn==F.L[1].fn) {
      if (X.home<X.guest) cg=YELLOW; else cg=CYAN;
    }
  }


  printf(" %s%-16s\033[0m - %s%-16s\033[0m ", color[ch], NickOf(L, X.home, X.year), color[cg], NickOf(L, X.guest, X.year));
//  fprintf(outf, " <FONT COLOR=\"#0000FF\"> <STRONG> %s - %s  ",
//         NameOf(L, X.home, X.year), NameOf(L, X.guest, X.year));
  printf("%d-%d", X.score[0], X.score[1]);
//  fprintf(outf, "%d-%d</STRONG>", X.score[0], X.score[1]);
//  fprintf(outf, "</COLOR></FONT>\n");
  E.Copy(&X);
}

void HTMLHeader() {
  outf = fopen("output.html", "wt");
  if (!outf) {
    printf("Output error: could not open output file \"output.html\"\n");
    exit(1);
  }
  fprintf(outf, "<HTML>\n<TITLE>Results</TITLE><HEAD>\nResults</HEAD>\n<BODY>\n<pre>\n");
}

void HTMLFooter() {
  fprintf(outf, "</pre>\n</BODY>\n</HTML>");
  fclose(outf);
}

//--------------------------------------------

int main(int argc, char** argv) {
 unsigned char *s;
 RT = 0;
 all = 0;
 for (int i=1; i<argc; i++) {
   if (strcmp(argv[i], "-a")==0) all = 1;
 }
 Load(); 
 F.Reset(0,10);
 while (F.Get()) {
   ResetBoard();
//   HTMLHeader();
   for (int j=0; j<M; j++) {
     if (strlen(data[j]) < 10) continue;
     s = (unsigned char*) data[j];
     X.Decode(s);
     if (F.Check()) {
       Extract();
       AddGlobal();
     }
   }
   if (all) PadBoard();
   ListBoard(B, (all>0?NT:nst), who);
//   HTMLFooter();
 };
 return 0;
}
