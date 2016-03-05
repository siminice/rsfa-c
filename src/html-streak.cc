#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS  12
#define MAX_RR       4
#define MAX_N       64
#define MAX_TEAMS 2000
#define NUMORD      10

int max;
int  NC, ND;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;
int num_winter;
int *start_winter, *end_winter;

int id[MAX_N];
int round[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N];
int year;
int  *win, *maxwin;
int  *los, *maxlos;
int  *tie, *maxtie;
int  *undef, *maxundef;
int  *winles, *maxwinles;
int  *hwin, *maxhwin;
int  *hlos, *maxhlos;
int  *htie, *maxhtie;
int  *hundef, *maxhundef;
int  *hwinles, *maxhwinles;
int  *awin, *maxawin;
int  *alos, *maxalos;
int  *atie, *maxatie;
int  *aundef, *maxaundef;
int  *awinles, *maxawinles;
FILE *of;
char *filename;

const char* fxcol[] = {"77FF77", "FFFF99", "FF5050", "7777FF", "FF77FF", "77FFFF", "CCCCFF", "FFCCCC"};
const char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char stm[33];
int  tm, act, dvs, lvl, fy;

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
 
Aliases **L;

//--------------------------------------------------
  
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

int Find(char *s) {
  for (int i=0; i<NC; i++) {
    if (strstr(mnem[i], s)==mnem[i]) return i;
  }
  return -1;
}

//--------------------------------------------------

struct entry {
  int home, guest;
  int score;
  int level;
  int year;
  int date;
  entry *next;
  entry(int h, int g, int s, int l, int y, int d): home(h), guest(g), score(s), level(l), year(y), date(d), next(NULL) {}
};

struct streak {
  int len;
  entry *head;
  entry *tail;
  streak(): len(0), head(NULL), tail(NULL) {}
  ~streak();
  void append(entry *e);
  void header(int i, int type);
  void print();
  void reset();
};

streak::~streak() {
  while (head!=NULL) {
    entry *temp = head->next;
    head->next = NULL;
    delete head;
    head = temp;
  }
  len = 0;
  head = tail = NULL;
}

void streak::append(entry *e) {
  len++;
  if (head==NULL) {
    len = 1;
    head = tail = e;
  }
  else {
     tail->next = e;
     tail = e;
  }
}

void streak::header(int i, int type) {
  if (head==NULL || len==0) return;
  int x = head->home;
  int y = head->guest;
  if (head->guest == tail->home || head->guest == tail->guest) {
     if (tail->home!=x && tail->guest!=x) x = head->guest;
     else if (head->next != NULL) {
       if (head->next->home == y || head->next->guest == y) x = y;
     }
  }
  int num = len - 2;
  int a1, a2, b1, b2;
  if (len == 1) num++;
  else {
    a1 = head->score/100; b1 = head->score%100;
    a2 = tail->score/100; b2 = tail->score%100;
    if (tail->home==x) {
      switch (type%6) {
        case 1:
          if (a2>b2) num++; break;
        case 2:
          if (a2>=b2) num++; break;
        case 3:
          if (a2==b2) num++; break;
        case 4:
          if (a2<=b2) num++; break;
        case 5:
          if (a2<b2) num++; break;
      }
    }
    if (tail->guest==x) {
      switch (type%6) {
        case 1:
          if (a2<b2) num++; break;
        case 2:
          if (a2<=b2) num++; break;
        case 3:
          if (a2==b2) num++; break;
        case 4:
          if (a2>=b2) num++; break;
        case 5:
          if (a2>b2) num++; break;
      }
    }
  }
  fprintf(of, "<A NAME=\"streak-%d-%d\" HREF=\"javascript:;\" onmousedown=\"toggleDiv('streak-%d-%d');\">", type, i, type, i);
  fprintf(of, "<H3>");
  fprintf(of, "%d: %-30s (%2d %s %d - %2d %s %d)<BR>\n", 
          num, NameOf(L, x, tail->year), 
          head->date%50, month[head->date/50], head->year,
          tail->date%50, month[tail->date/50], tail->year
         );
  fprintf(of, "</H3></A>\n");
  fprintf(of, "<DIV id=\"streak-%d-%d\" style=\"display:none\">\n", type, i);
}

void streak::print() {

    fprintf(of, "<TABLE WIDTH=\"70%%\" BGCOLOR=\"DDDDDD\" BORDER=\"0\" RULES=\"rows\" FRAME=\"box\">\n");
    fprintf(of, "<THEAD>\n");
    fprintf(of, "<COL WIDTH=\"4%%\" ALIGN=\"center\"></COL>\n");
    fprintf(of, "<COL WIDTH=\"12%%\" ALIGN=\"right\"></COL>\n");
    fprintf(of, "<COL WIDTH=\"1%%\"></COL>\n");
    fprintf(of, "<COL WIDTH=\"37%%\" ALIGN=\"left\"></COL>\n");
    fprintf(of, "<COL WIDTH=\"1%%\"></COL>\n");
    fprintf(of, "<COL WIDTH=\"37%%\" ALIGN=\"left\"></COL>\n");
    fprintf(of, "<COL WIDTH=\"8%%\" ALIGN=\"center\"></COL>\n");
    fprintf(of, "</THEAD><TBODY>\n");

  entry *e = head;
  for (int i=0; i<len; i++) { 
    fprintf(of, "<TR BGCOLOR=\"%s\">", (i==0 || i==len-1? "CCCCCC" : "EEEEEE"));
//    fprintf(of, "  %c.%2d %s %d %-30s - %-30s  %d-%d<BR>\n", (char) 'A'+e->level, 
//           e->date%50, month[e->date/50], e->year,
//           NameOf(L, e->home, e->year), NameOf(L, e->guest, e->year), e->score/100, e->score%100);
    fprintf(of, "<TD BGCOLOR=\"%s\">%c</TD>", fxcol[e->level], (char) 'A'+e->level);
    fprintf(of, "<TD ALIGN=\"center\">%02d-%02d-%d</TD>", e->date%50, e->date/50, e->year);
    fprintf(of, "<TD></TD>");
    fprintf(of, "<TD>%s</TD><TD>-</TD><TD>%s</TD>", NameOf(L, e->home, e->year), NameOf(L, e->guest, e->year));
    fprintf(of, "<TD>%d-%d</TD>", e->score/100, e->score%100);
    fprintf(of, "</TR>\n");
    e = e->next;
  }

  fprintf(of, "</TBODY></TABLE>\n");
  fprintf(of, "</DIV>\n");
}

void streak::reset() {
  len = 0;
  head = NULL;
  tail = NULL;
}

void insert(streak *s, streak **t) {
  if (s->len<=t[max-1]->len) {
//    delete s;
  }
  int i = 0;
  while (i<max && s->len<=t[i]->len) i++;
  if (i<max) {
//    delete t[max-1];
    for (int k=max-1; k>i; k--) t[k] = t[k-1];
    t[i] = s;
  }
}

streak **twin, **tundef, **ttie, **tlos, **twinles;
streak **currwin, **currtie, **currlos, **currund, **currwls;
streak **thwin, **thundef, **thtie, **thlos, **thwinles;
streak **hcurrwin, **hcurrtie, **hcurrlos, **hcurrund, **hcurrwls;
streak **tawin, **taundef, **tatie, **talos, **tawinles;
streak **acurrwin, **acurrtie, **acurrlos, **acurrund, **acurrwls;

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  int dummy, n, p, t, d, y;
  char *dv, *pl, *yr, *suf;

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
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;

  win = new int[NC];
  los = new int[NC];
  tie = new int[NC];
  undef = new int[NC];
  winles = new int[NC];
  maxwin = new int[NC];
  maxlos = new int[NC];
  maxtie = new int[NC];
  maxundef = new int[NC];
  maxwinles = new int[NC];
  hwin = new int[NC];
  hlos = new int[NC];
  htie = new int[NC];
  hundef = new int[NC];
  hwinles = new int[NC];
  maxhwin = new int[NC];
  maxhlos = new int[NC];
  maxhtie = new int[NC];
  maxhundef = new int[NC];
  maxhwinles = new int[NC];
  awin = new int[NC];
  alos = new int[NC];
  atie = new int[NC];
  aundef = new int[NC];
  awinles = new int[NC];
  maxawin = new int[NC];
  maxalos = new int[NC];
  maxatie = new int[NC];
  maxaundef = new int[NC];
  maxawinles = new int[NC];

  twin = new streak*[max];
  tundef = new streak*[max];
  ttie = new streak*[max];
  tlos = new streak*[max];
  twinles = new streak*[max];
  thwin = new streak*[max];
  thundef = new streak*[max];
  thtie = new streak*[max];
  thlos = new streak*[max];
  thwinles = new streak*[max];
  tawin = new streak*[max];
  taundef = new streak*[max];
  tatie = new streak*[max];
  talos = new streak*[max];
  tawinles = new streak*[max];

  currwin = new streak*[NC];
  currtie = new streak*[NC];
  currlos = new streak*[NC];
  currund = new streak*[NC];
  currwls = new streak*[NC];
  hcurrwin = new streak*[NC];
  hcurrtie = new streak*[NC];
  hcurrlos = new streak*[NC];
  hcurrund = new streak*[NC];
  hcurrwls = new streak*[NC];
  acurrwin = new streak*[NC];
  acurrtie = new streak*[NC];
  acurrlos = new streak*[NC];
  acurrund = new streak*[NC];
  acurrwls = new streak*[NC];

  for (int i=0; i<max; i++) {
    twin[i] = new streak;
    tundef[i] = new streak;
    ttie[i] = new streak;
    tlos[i] = new streak;
    twinles[i] = new streak;
    thwin[i] = new streak;
    thundef[i] = new streak;
    thtie[i] = new streak;
    thlos[i] = new streak;
    thwinles[i] = new streak;
    tawin[i] = new streak;
    taundef[i] = new streak;
    tatie[i] = new streak;
    talos[i] = new streak;
    tawinles[i] = new streak;
  }
  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[32];
    memmove(mnem[i], s, 15); mnem[i][15] = 0;
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
    win[i] = los[i] = tie[i] = undef[i] = winles[i] = 0;
    maxwin[i] = maxlos[i] = maxtie[i] = maxundef[i] = maxwinles[i] = 0;
    currwin[i] = new streak;
    currtie[i] = new streak;
    currlos[i] = new streak;
    currund[i] = new streak;
    currwls[i] = new streak;
    hcurrwin[i] = new streak;
    hcurrtie[i] = new streak;
    hcurrlos[i] = new streak;
    hcurrund[i] = new streak;
    hcurrwls[i] = new streak;
    acurrwin[i] = new streak;
    acurrtie[i] = new streak;
    acurrlos[i] = new streak;
    acurrund[i] = new streak;
    acurrwls[i] = new streak;
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
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
//      printf("Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
      k++;
    }
  }
  fclose(f);

  FY = new int[MAX_LEVELS];
  LY = new int[MAX_LEVELS];
  MAX = new int [MAX_LEVELS];

  for (int d=0; d<MAX_LEVELS; d++) {
    FY[d] = 2100;
    LY[d] = 1800;
    MAX[d] = 0;
  }

  DIR *dp;
  struct dirent *ep;
  dp = opendir("./");
  if (dp != NULL) {
      while (ep = readdir (dp)) {
        strcpy(s, ep->d_name);
        dv = strtok(s, ".");
        yr = strtok(NULL, ".");
        suf = strtok(NULL, ".");
        if (dv!=NULL && yr!=NULL && suf==NULL) {
          int l = strlen(dv);
          if (l==0 || l>3) continue;
          d = ((int) s[0]) - 97;
          if (d<0 || d>=MAX_LEVELS) continue;
          if (d+1>ND) ND = d+1;
          if (l>1) p = atoi(dv+1); else p = 1;
          if (p<0 && p>12) continue;
          y = atoi(yr);
          if (d>=0 && y>1888 && y<2100) {
            if (y<FY[d]) FY[d] = y;
            if (y>LY[d]) LY[d] = y;
            if (p>MAX[d]) MAX[d] = p;
          }
        }
      }
      closedir(dp);
  }
  else
   printf("ERROR: Couldn't open the directory.\n");  

}

int isWinter(int y) {
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) return 1;
  }
  return 0;
}

int LoadFile(char *filename) {
  FILE *f;
  int n, h, i, j, x, y, r, z, ppv, tbr, rr, pr1, pr2, rel1, rel2;
  f = fopen(filename, "rt");
  if (NULL == f) { return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);  
  rr = tbr/NUMORD + 1;


  /* clear all  data */
  for (h=0; h<rr; h++) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        round[h][i][j] = -1;
        res[h][i][j] = -1;
      }
    }
  }

  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
  }
  for (int h=0; h<rr; h++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[h][i][j] = r;
      res[h][i][j]   = z;
    }
    fscanf(f, "\n");
  }
  }
  fclose(f);

  // determine start date
  int fd = 0;
  int seas = year;
  for (int i=1; i<n; i++) {
    if (round[0][0][i]/1000 == 1 && round[0][0][i]%1000 > 200) fd = round[0][0][i]%1000 - 10;
    if (round[0][i][0]/1000 == 1 && round[0][i][0]%1000 > 200) fd = round[0][i][0]%1000 - 10;
  }
  if (fd == 0) fd = 365;
  if ((fd > 300) && !isWinter(year)) seas = seas-1;
  // scan all dates;
  for (int d = fd; d<fd+649; d++) {
    if (d==651) seas++;
    for (int h=0; h<rr; h++) {
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (round[h][i][j]%1000 == d%650) {
          int a = res[h][i][j]/100;
          int b = res[h][i][j]%100;
          int t = id[i];
          int u = id[j];

          if (a>b) {
            if (tm<0 || t==tm) {
            if (tie[t]>0) { // TW
              currtie[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currwls[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currtie[t], ttie); 
              insert(currwls[t], twinles);
              currtie[t] = new streak;
              currwls[t] = new streak;
              currlos[t]->reset();
            } 
            else if (los[t]>0) { // LW
              currlos[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currwls[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currlos[t], tlos);
              insert(currwls[t], twinles);
              currtie[t]->reset();
              currlos[t] = new streak;
              currwls[t] = new streak;
            }
            else { // WW
              currtie[t]->reset();
              currlos[t]->reset();
              currwls[t]->reset();
            }

            if (htie[t]>0) { // TW
              hcurrtie[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              hcurrwls[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(hcurrtie[t], thtie); 
              insert(hcurrwls[t], thwinles);
              hcurrtie[t] = new streak;
              hcurrlos[t]->reset();
              hcurrwls[t] = new streak;
            } 
            else if (hlos[t]>0) { // LW
              hcurrlos[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              hcurrwls[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(hcurrlos[t], thlos);
              insert(hcurrwls[t], thwinles);
              hcurrtie[t]->reset();
              hcurrlos[t] = new streak;
              hcurrwls[t] = new streak;
            }
            else { // WW
              hcurrtie[t]->reset();
              hcurrlos[t]->reset();
              hcurrwls[t]->reset();
            }
            }

            if (tm<0 || u==tm) {
            if (tie[u]>0) { // TL
              currtie[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currund[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currtie[u], ttie); 
              insert(currund[u], tundef);
              currwin[u]->reset();
              currtie[u] = new streak;
              currund[u] = new streak;
            } 
            else if (win[u]>0) { // WL
              currwin[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currund[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currwin[u], twin); 
              insert(currund[u], tundef);
              currwin[u] = new streak;
              currtie[u]->reset();
              currund[u] = new streak;
            }
            else { // LL
              currwin[u]->reset();
              currtie[u]->reset();
              currund[u]->reset();
            }

            if (atie[u]>0) { // TL
              acurrtie[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              acurrund[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(acurrtie[u], tatie); 
              insert(acurrund[u], taundef);
              acurrwin[u]->reset();
              acurrtie[u] = new streak;
              acurrund[u] = new streak;
            } 
            else if (awin[u]>0) { // WL
              acurrwin[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              acurrund[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(acurrwin[u], tawin); 
              insert(acurrund[u], taundef);
              acurrwin[u] = new streak;
              acurrtie[u]->reset();
              acurrund[u] = new streak;
            }
            else { // LL
              acurrwin[u]->reset();
              acurrtie[u]->reset();
              acurrund[u]->reset();
            }
            }

            los[t] = winles[t] = tie[t] = 0;
            hlos[t] = hwinles[t] = htie[t] = 0;
            win[u] = undef[u] = tie[u] = 0;
            awin[u] = aundef[u] = atie[u] = 0;
            win[t]++; if (win[t]>maxwin[t]/10000) maxwin[t] = 10000*win[t]+seas;
            hwin[t]++; if (hwin[t]>maxhwin[t]/10000) maxhwin[t] = 10000*hwin[t]+seas;
            undef[t]++; if (undef[t]>maxundef[t]/10000) maxundef[t] = 10000*undef[t]+seas;
            hundef[t]++; if (hundef[t]>maxhundef[t]/10000) maxhundef[t] = 10000*hundef[t]+seas;
            los[u]++; if (los[u]>maxlos[u]/10000) maxlos[u] = 10000*los[u]+seas;
            alos[u]++; if (alos[u]>maxalos[u]/10000) maxalos[u] = 10000*alos[u]+seas;
            winles[u]++; if (winles[u]>maxwinles[u]/10000) maxwinles[u] = 10000*winles[u]+seas;
            awinles[u]++; if (awinles[u]>maxawinles[u]/10000) maxawinles[u] = 10000*awinles[u]+seas;
          }

          else if (a==b) {
            if (tm<0 || t==tm) {
            if (win[t]>0) { // WT
              currwin[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currwin[t], twin); 
              currwin[t] = new streak;
              currlos[t]->reset(); 
            } 
            else if (los[t]>0) { // LT
              currlos[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currlos[t], tlos); 
              currwin[t]->reset();
              currlos[t] = new streak; 
            }
            else { // TT
              currwin[t]->reset();
              currlos[t]->reset(); 
            }

            if (hwin[t]>0) { // WT
              hcurrwin[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(hcurrwin[t], thwin); 
              hcurrwin[t] = new streak;
              hcurrlos[t]->reset(); 
            } 
            else if (hlos[t]>0) { // LT
              hcurrlos[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(hcurrlos[t], thlos); 
              hcurrwin[t]->reset();
              hcurrlos[t] = new streak; 
            }
            else { // TT
              hcurrwin[t]->reset();
              hcurrlos[t]->reset(); 
            }
            }

            if (tm<0 || u==tm) {
            if (win[u]>0) { // WT
              currwin[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currwin[u], twin); 
              currwin[u] = new streak;
              currlos[u]->reset(); 
            } 
            else if (los[u]>0) { // LT
              currlos[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currlos[u], tlos); 
              currwin[u]->reset();
              currlos[u] = new streak; 
            }
            else { // TT
              currwin[u]->reset();
              currlos[u]->reset(); 
            }

            if (awin[u]>0) { // WT
              acurrwin[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(acurrwin[u], tawin); 
              acurrwin[u] = new streak;
              acurrlos[u]->reset(); 
            } 
            else if (alos[u]>0) { // LT
              acurrlos[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(acurrlos[u], talos); 
              acurrwin[u]->reset();
              acurrlos[u] = new streak; 
            }
            else { // TT
              acurrwin[u]->reset();
              acurrlos[u]->reset(); 
            }
            }

            win[t] = los[t] = 0;
            hwin[t] = hlos[t] = 0;
            win[u] = los[u] = 0;
            awin[u] = alos[u] = 0;
            tie[t]++; if (tie[t]>maxtie[t]/10000) maxtie[t] = 10000*tie[t]+seas;
            htie[t]++; if (htie[t]>maxhtie[t]/10000) maxhtie[t] = 10000*htie[t]+seas;
            undef[t]++; if (undef[t]>maxundef[t]/10000) maxundef[t] = 10000*undef[t]+seas;
            hundef[t]++; if (hundef[t]>maxhundef[t]/10000) maxhundef[t] = 10000*hundef[t]+seas;
            tie[u]++; if (tie[u]>maxtie[u]/10000) maxtie[u] = 10000*tie[u]+seas;
            atie[u]++; if (atie[u]>maxatie[u]/10000) maxatie[u] = 10000*atie[u]+seas;
            undef[u]++; if (undef[u]>maxundef[u]/10000) maxundef[u] = 10000*undef[u]+seas;
            aundef[u]++; if (aundef[u]>maxaundef[u]/10000) maxaundef[u] = 10000*aundef[u]+seas;
          }

          else { // a<b
            if (tm<0 || t==tm) {
            if (tie[t]>0) { // TL
              currtie[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currund[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currtie[t], ttie); 
              insert(currund[t], tundef);
              currwin[t]->reset();
              currtie[t] = new streak;
              currund[t] = new streak;
            } 
            else if (win[t]>0) { // WL
              currwin[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currund[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currwin[t], twin); 
              insert(currund[t], tundef);
              currwin[t] = new streak;
              currtie[t]->reset();
              currund[t] = new streak;
            }
            else { // LL
              currwin[t]->reset();
              currtie[t]->reset();
              currund[t]->reset();
            }

            if (htie[t]>0) { // TL
              hcurrtie[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              hcurrund[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(hcurrtie[t], thtie); 
              insert(hcurrund[t], thundef);
              hcurrwin[t]->reset();
              hcurrtie[t] = new streak;
              hcurrund[t] = new streak;
            } 
            else if (hwin[t]>0) { // WL
              hcurrwin[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              hcurrund[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(hcurrwin[t], thwin); 
              insert(hcurrund[t], thundef);
              hcurrwin[t] = new streak;
              hcurrtie[t]->reset();
              hcurrund[t] = new streak;
            }
            else { // LL
              hcurrwin[t]->reset();
              hcurrtie[t]->reset();
              hcurrund[t]->reset();
            }
            }


            if (tm<0 || u==tm) {
            if (tie[u]>0) { // TW
              currtie[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currwls[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currtie[u], ttie);
              insert(currwls[u], twinles); 
              currtie[u] = new streak;
              currlos[u]->reset();
              currwls[u] = new streak;
            } 
            else if (los[u]>0) { // LW
              currlos[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              currwls[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(currlos[u], tlos);
              insert(currwls[u], twinles); 
              currtie[u]->reset();
              currlos[u] = new streak;
              currwls[u] = new streak;
            }
            else { // WW
              currtie[u]->reset();
              currlos[u]->reset();
              currwls[u]->reset();
            }

            if (atie[u]>0) { // TW
              acurrtie[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              acurrwls[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(acurrtie[u], tatie);
              insert(acurrwls[u], tawinles); 
              acurrtie[u] = new streak;
              acurrlos[u]->reset();
              acurrwls[u] = new streak;
            } 
            else if (alos[u]>0) { // LW
              acurrlos[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              acurrwls[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
              insert(acurrlos[u], talos);
              insert(acurrwls[u], tawinles); 
              acurrtie[u]->reset();
              acurrlos[u] = new streak;
              acurrwls[u] = new streak;
            }
            else { // WW
              acurrtie[u]->reset();
              acurrlos[u]->reset();
              acurrwls[u]->reset();
            }
            }

            win[t] = undef[t] = tie[t] = 0;
            hwin[t] = hundef[t] = htie[t] = 0;
            los[u] = winles[u] = tie[u] = 0;
            alos[u] = awinles[u] = atie[u] = 0;
            los[t]++; if (los[t]>maxlos[t]/10000) maxlos[t] = 10000*los[t]+seas;
            hlos[t]++; if (hlos[t]>maxhlos[t]/10000) maxhlos[t] = 10000*hlos[t]+seas;
            winles[t]++; if (winles[t]>maxwinles[t]/10000) maxwinles[t] = 10000*winles[t]+seas;
            hwinles[t]++; if (hwinles[t]>maxhwinles[t]/10000) maxhwinles[t] = 10000*hwinles[t]+seas;
            win[u]++; if (win[u]>maxwin[u]/10000) maxwin[u] = 10000*win[u]+seas;
            awin[u]++; if (awin[u]>maxawin[u]/10000) maxawin[u] = 10000*awin[u]+seas;
            undef[u]++; if (undef[u]>maxundef[u]/10000) maxundef[u] = 10000*undef[u]+seas;
            aundef[u]++; if (aundef[u]>maxaundef[u]/10000) maxaundef[u] = 10000*aundef[u]+seas;
          }

          if (tm<0 || t==tm) {
          currwin[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currtie[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currlos[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currund[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currwls[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));

          hcurrwin[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          hcurrtie[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          hcurrlos[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          hcurrund[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          hcurrwls[t]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          }

          if (tm<0 || u==tm) {
          currwin[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currtie[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currlos[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currund[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          currwls[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));

          acurrwin[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          acurrtie[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          acurrlos[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          acurrund[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          acurrwls[u]->append(new entry(t, u, res[h][i][j], lvl, seas, round[h][i][j]%1000));
          }
        }
      }	// for j
    }	// for i
    }	// for h
  }	//  for d
}



/*******************************************************************/


int Ranking() {
  int *rank = new int[NC];
  int sorted = 0;

//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxwin[rank[i+1]]>maxwin[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest winning streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxwin[rank[i]]%10000), maxwin[rank[i]]/10000, maxwin[rank[i]]%10000);

//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxundef[rank[i+1]]>maxundef[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest undefeated streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxundef[rank[i]]%10000), maxundef[rank[i]]/10000, maxundef[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxtie[rank[i+1]]>maxtie[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest streaks of consecutive ties:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxtie[rank[i]]%10000), maxtie[rank[i]]/10000, maxtie[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxwinles[rank[i+1]]>maxwinles[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest winless streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxwinles[rank[i]]%10000), maxwinles[rank[i]]/10000, maxwinles[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxlos[rank[i+1]]>maxlos[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest losing streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxlos[rank[i]]%10000), maxlos[rank[i]]/10000, maxlos[rank[i]]%10000);


//-----------------------------------------------------------------
//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxhwin[rank[i+1]]>maxhwin[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest home winning streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxhwin[rank[i]]%10000), maxhwin[rank[i]]/10000, maxhwin[rank[i]]%10000);

//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxhundef[rank[i+1]]>maxhundef[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest undefeated at home streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxhundef[rank[i]]%10000), maxhundef[rank[i]]/10000, maxhundef[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxhtie[rank[i+1]]>maxhtie[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest streaks of consecutive home ties:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxhtie[rank[i]]%10000), maxhtie[rank[i]]/10000, maxhtie[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxhwinles[rank[i+1]]>maxhwinles[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest winless streaks at home:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxhwinles[rank[i]]%10000), maxhwinles[rank[i]]/10000, maxhwinles[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxhlos[rank[i+1]]>maxhlos[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest losing at home streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxhlos[rank[i]]%10000), maxhlos[rank[i]]/10000, maxhlos[rank[i]]%10000);

//-----------------------------------------------------------------
//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxawin[rank[i+1]]>maxawin[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest winning away streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxawin[rank[i]]%10000), maxawin[rank[i]]/10000, maxawin[rank[i]]%10000);

//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxaundef[rank[i+1]]>maxaundef[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest undefeated away streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxaundef[rank[i]]%10000), maxaundef[rank[i]]/10000, maxaundef[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxatie[rank[i+1]]>maxatie[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest streaks of consecutive away ties:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxatie[rank[i]]%10000), maxatie[rank[i]]/10000, maxatie[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxawinles[rank[i+1]]>maxawinles[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest winless away streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxawinles[rank[i]]%10000), maxawinles[rank[i]]/10000, maxawinles[rank[i]]%10000);


//-----------------------------------------------------------------
  for (int i=0; i<NC; i++) rank[i] = i;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) {
      if (maxalos[rank[i+1]]>maxalos[rank[i]]) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
    }
  } while (sorted==0);
  printf("\n Longest losing away streaks:\n");
  for (int i=0; i<max; i++) 
    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, rank[i], maxalos[rank[i]]%10000), maxalos[rank[i]]/10000, maxalos[rank[i]]%10000);

  return 0;
}



//-----------------------------------------------------------------
//-----------------------------------------------------------------
int StreakRanking() {

  int x;

//-----------------------------------------------------------------
  fprintf(of, "<H3>Victorii consecutive</H3>\n");
  for (int i=0; i<max; i++) {
    twin[i]->header(i,1);
    twin[i]->print();
  }

  fprintf(of, "<H3>Meciuri consecutive fãrã înfrângere</H3>\n");
  for (int i=0; i<max; i++) {
    tundef[i]->header(i,2);
    tundef[i]->print();
  }

  fprintf(of, "<H3>Egaluri consecutive</H3>\n");
  for (int i=0; i<max; i++) {
    ttie[i]->header(i,3);
    ttie[i]->print();
  }

  fprintf(of, "<H3>Meciuri consecutive fãrã victorie</H3>\n");
  for (int i=0; i<max; i++) {
    twinles[i]->header(i,4);
    twinles[i]->print();
  }

  fprintf(of, "<H3>Înfrângeri consecutive</H3>\n");
  for (int i=0; i<max; i++) {
    tlos[i]->header(i,5);
    tlos[i]->print();
  }

  fprintf(of, "<HR>\n");

//-----------------------------------------------------------------
  fprintf(of, "<H3>Victorii consecutive acasã</H3>\n");
  for (int i=0; i<max; i++) {
    thwin[i]->header(i,7);
    thwin[i]->print();
  }

  fprintf(of, "<H3>Meciuri consecutive fãrã înfrângere acasã</H3>\n");
  for (int i=0; i<max; i++) {
    thundef[i]->header(i,8);
    thundef[i]->print();
  }

  fprintf(of, "<H3>Egaluri consecutive acasã</H3>\n");
  for (int i=0; i<max; i++) {
    thtie[i]->header(i,9);
    thtie[i]->print();
  }

  fprintf(of, "<H3>Meciuri consecutive fãrã victorie acasã</H3>\n");
  for (int i=0; i<max; i++) {
    thwinles[i]->header(i,10);
    thwinles[i]->print();
  }

  fprintf(of, "<H3>Înfrângeri consecutive acasã</H3>\n");
  for (int i=0; i<max; i++) {
    thlos[i]->header(i,11);
    thlos[i]->print();
  }

  fprintf(of, "<HR>\n");

//-----------------------------------------------------------------
  fprintf(of, "<H3>Victorii consecutive în deplasare</H3>\n");
  for (int i=0; i<max; i++) {
    tawin[i]->header(i,13);
    tawin[i]->print();
  }

  fprintf(of, "<H3>Meciuri consecutive fãrã înfrângere în deplasare</H3>\n");
  for (int i=0; i<max; i++) {
    taundef[i]->header(i,14);
    taundef[i]->print();
  }

  fprintf(of, "<H3>Egaluri consecutive în deplasare</H3>\n");
  for (int i=0; i<max; i++) {
    tatie[i]->header(i,15);
    tatie[i]->print();
  }

  fprintf(of, "<H3>Meciuri consecutive fãrã victorie în deplasare</H3>\n");
  for (int i=0; i<max; i++) {
    tawinles[i]->header(i,16);
    tawinles[i]->print();
  }

  fprintf(of, "<H3>Înfrângeri consecutive în deplasare</H3>\n");
  for (int i=0; i<max; i++) {
    talos[i]->header(i,17);
    talos[i]->print();
  }

  return 0;
}


void HTMLHeader(int t) {
  if (t>=0) 
    fprintf(of, "<HTML>\n<TITLE>%s - serii consecutive</TITLE>\n", NameOf(L, t, 3000));
  else 
    fprintf(of, "<HTML>\n<TITLE>Serii consecutive</TITLE>\n");
  fprintf(of, "<HEAD>\n<link href=\"css/results.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");
  fprintf(of, "<script language=\"javascript\">\n");
  fprintf(of, "  function toggleDiv(divid){\n");
  fprintf(of, "    if(document.getElementById(divid).style.display == 'none'){\n");
  fprintf(of, "      document.getElementById(divid).style.display = 'block';\n");
  fprintf(of, "    }else{\n");
  fprintf(of, "      document.getElementById(divid).style.display = 'none';\n");
  fprintf(of, "    }\n");
  fprintf(of, "  }\n");  
  fprintf(of, "</script>\n");
}

//-----------------------------------------------------------------
int Streaks(int t, int old) {
  char filename[64];
  
  sprintf(filename, "html/consecutive-%d.html", t);
  of = fopen(filename, "wt");
  if (of==NULL) {
    fprintf(stderr, "ERROR: cannot open %s.\n", filename);
    return -1;
  }
  
  HTMLHeader(t); 
  if (t>=0)
    fprintf(of, "<H2>%s - serii consecutive</H2>\n", NameOf(L, t, 3000));

  for (year=(fy>FY[0]?fy:FY[0]); year<=LY[0]; year++) {
    for (lvl=0; lvl<ND; lvl++) {
      if (dvs==0 || dvs==lvl+1) {
        for (int p=1; p<=MAX[lvl]; p++) {
          sprintf(filename, "%c%d.%d", (char)(lvl+97), p, year);
          LoadFile(filename);
        }
        sprintf(filename, "%c.%d", (char)(lvl+97), year);
        LoadFile(filename);
      }
    }
  }

  if (act) {
    for (int i=0; i<max; i++) {
      twin[i]->len = 0;
      tundef[i]->len = 0;
      ttie[i]->len = 0;
      twinles[i]->len = 0;
      tlos[i]->len = 0;
      thwin[i]->len = 0;
      thundef[i]->len = 0;
      thtie[i]->len = 0;
      thwinles[i]->len = 0;
      thlos[i]->len = 0;
      tawin[i]->len = 0;
      taundef[i]->len = 0;
      tatie[i]->len = 0;
      tawinles[i]->len = 0;
      talos[i]->len = 0;
    }
  }

  for (int i=0; i<NC; i++) {
    if (win[i]>0) { 
      insert(currwin[i], twin); 
      insert(currund[i], tundef);
    }
    else if (tie[i]>0) { 
      insert(currund[i], tundef); 
      insert(currtie[i], ttie);
      insert(currwls[i], twinles);
    }
    else if (los[i]>0) { 
      insert(currwls[i], twinles); 
      insert(currlos[i], tlos);
    }

    if (hwin[i]>0) { 
      insert(hcurrwin[i], thwin); 
      insert(hcurrund[i], thundef);
    }
    else if (htie[i]>0) { 
      insert(hcurrund[i], thundef); 
      insert(hcurrtie[i], thtie);
      insert(hcurrwls[i], thwinles);
    }
    else if (hlos[i]>0) { 
      insert(hcurrwls[i], thwinles); 
      insert(hcurrlos[i], thlos);
    }

    if (awin[i]>0) { 
      insert(acurrwin[i], tawin); 
      insert(acurrund[i], taundef);
    }
    else if (atie[i]>0) { 
      insert(acurrund[i], taundef); 
      insert(acurrtie[i], tatie);
      insert(acurrwls[i], tawinles);
    }
    else if (alos[i]>0) { 
      insert(acurrwls[i], tawinles); 
      insert(acurrlos[i], talos);
    }
  }
  if (old)
    Ranking();
  else
    StreakRanking();

  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);

  return 0;
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  int a, b;
  max = 10;
  int old = 0;
  tm = -1;
  act = 0;
  dvs = 0;
  fy = 0;
  Load();
  for (int k = 1; k<argc; k++) {
    if (strcmp(argv[k], "-h")==0 || strcmp(argv[k],"-help")==0) {
      printf("Usage: streak [-n maxentry] [-t, --team <n>] [-c, --current] [-d, --div D] [-f, --fy year]\n");
      return 1;
    }
    if (strcmp(argv[k], "-n")==0) 
      max = atoi(argv[k+1]);
    else if (strcmp(argv[k], "--old")==0) 
     old = 1;
    else if (strcmp(argv[k], "--team")==0 || strcmp(argv[k], "-t")==0) {
      if (k<argc-1) tm = atoi(argv[k+1]);
    }
    else if (strcmp(argv[k], "--current")==0 || strcmp(argv[k], "-c")==0) {
      if (tm>=0) max = 1;
      act = 1;
    }
    else if (strcmp(argv[k], "--div")==0 || strcmp(argv[k], "-d")==0) {
      dvs = atoi(argv[k+1]);
    }
    else if (strcmp(argv[k], "--fy")==0 ||  strcmp(argv[k], "-f")==0) {
      fy = atoi(argv[k+1]);
    }
  }
  Streaks(tm, old);
  return 0;
}
