#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS 12

int max;
int  NC, ND;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;

int id[50];
int res[50][50];
int round[50][50];
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
char *filename;
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
  if (head->guest == tail->home || head->guest == tail->guest) 
   x = head->guest;
  int num = len - 2;
  int a1, a2, b1, b2;
  if (len == 1) num++;
  else {
    a1 = head->score/100; b1 = head->score%100;
    a2 = tail->score/100; b2 = tail->score%100;
    if (tail->home==x) {
      switch (type) {
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
      switch (type) {
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
  printf("%2d.%-30s %3d  [%2d %s %d - %2d %s %d]\n", 
          i+1, NameOf(L, x, tail->year), num, 
          head->date%50, month[head->date/50], head->year,
          tail->date%50, month[tail->date/50], tail->year
         );
}

void streak::print() {
  entry *e = head;
  for (int i=0; i<len; i++) {
    printf("  %c.%2d %s %d %-30s - %-30s  %d-%d\n", (char) 'A'+e->level, 
           e->date%50, month[e->date/50], e->year,
           NameOf(L, e->home, e->year), NameOf(L, e->guest, e->year), e->score/100, e->score%100);
    e = e->next;
  }
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


int LoadFile(char *filename) {
  FILE *f;
  int n, i, j, x, y, r, z, ppv, tbr, pr1, pr2, rel1, rel2;
  f = fopen(filename, "rt");
  if (NULL == f) { return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);  
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      res[i][j] = z;
    }
    fscanf(f, "\n");
  }
  fclose(f);
  // determine start date
  int fd = 0;
  int seas = year;
  for (int i=1; i<n; i++) {
    if (round[0][i]/1000 == 1 && round[0][i]%1000 > 200) fd = round[0][i]%1000 - 10;
    if (round[i][0]/1000 == 1 && round[i][0]%1000 > 200) fd = round[i][0]%1000 - 10;
  }
  if (fd == 0) fd = 365;
  if (fd > 300) seas = seas-1;
  // scan all dates;
  for (int d = fd; d<fd+649; d++) {
    if (d==651) seas++;
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (round[i][j]%1000 == d%650) {
          int a = res[i][j]/100;
          int b = res[i][j]%100;
          int t = id[i];
          int u = id[j];

          if (a>b) {
            if (tm<0 || t==tm) {
            if (tie[t]>0) { // TW
              currtie[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currwls[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currtie[t], ttie); 
              insert(currwls[t], twinles);
              currtie[t] = new streak;
              currwls[t] = new streak;
              currlos[t]->reset();
            } 
            else if (los[t]>0) { // LW
              currlos[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currwls[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              hcurrtie[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              hcurrwls[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(hcurrtie[t], thtie); 
              insert(hcurrwls[t], thwinles);
              hcurrtie[t] = new streak;
              hcurrlos[t]->reset();
              hcurrwls[t] = new streak;
            } 
            else if (hlos[t]>0) { // LW
              hcurrlos[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              hcurrwls[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              currtie[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currund[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currtie[u], ttie); 
              insert(currund[u], tundef);
              currwin[u]->reset();
              currtie[u] = new streak;
              currund[u] = new streak;
            } 
            else if (win[u]>0) { // WL
              currwin[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currund[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              acurrtie[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              acurrund[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(acurrtie[u], tatie); 
              insert(acurrund[u], taundef);
              acurrwin[u]->reset();
              acurrtie[u] = new streak;
              acurrund[u] = new streak;
            } 
            else if (awin[u]>0) { // WL
              acurrwin[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              acurrund[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              currwin[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currwin[t], twin); 
              currwin[t] = new streak;
              currlos[t]->reset(); 
            } 
            else if (los[t]>0) { // LT
              currlos[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currlos[t], tlos); 
              currwin[t]->reset();
              currlos[t] = new streak; 
            }
            else { // TT
              currwin[t]->reset();
              currlos[t]->reset(); 
            }

            if (hwin[t]>0) { // WT
              hcurrwin[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(hcurrwin[t], thwin); 
              hcurrwin[t] = new streak;
              hcurrlos[t]->reset(); 
            } 
            else if (hlos[t]>0) { // LT
              hcurrlos[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              currwin[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currwin[u], twin); 
              currwin[u] = new streak;
              currlos[u]->reset(); 
            } 
            else if (los[u]>0) { // LT
              currlos[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currlos[u], tlos); 
              currwin[u]->reset();
              currlos[u] = new streak; 
            }
            else { // TT
              currwin[u]->reset();
              currlos[u]->reset(); 
            }

            if (awin[u]>0) { // WT
              acurrwin[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(acurrwin[u], tawin); 
              acurrwin[u] = new streak;
              acurrlos[u]->reset(); 
            } 
            else if (alos[u]>0) { // LT
              acurrlos[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              currtie[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currund[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currtie[t], ttie); 
              insert(currund[t], tundef);
              currwin[t]->reset();
              currtie[t] = new streak;
              currund[t] = new streak;
            } 
            else if (win[t]>0) { // WL
              currwin[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currund[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              hcurrtie[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              hcurrund[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(hcurrtie[t], thtie); 
              insert(hcurrund[t], thundef);
              hcurrwin[t]->reset();
              hcurrtie[t] = new streak;
              hcurrund[t] = new streak;
            } 
            else if (hwin[t]>0) { // WL
              hcurrwin[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              hcurrund[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              currtie[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currwls[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(currtie[u], ttie);
              insert(currwls[u], twinles); 
              currtie[u] = new streak;
              currlos[u]->reset();
              currwls[u] = new streak;
            } 
            else if (los[u]>0) { // LW
              currlos[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              currwls[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
              acurrtie[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              acurrwls[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              insert(acurrtie[u], tatie);
              insert(acurrwls[u], tawinles); 
              acurrtie[u] = new streak;
              acurrlos[u]->reset();
              acurrwls[u] = new streak;
            } 
            else if (alos[u]>0) { // LW
              acurrlos[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
              acurrwls[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
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
          currwin[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currtie[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currlos[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currund[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currwls[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));

          hcurrwin[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          hcurrtie[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          hcurrlos[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          hcurrund[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          hcurrwls[t]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          }

          if (tm<0 || u==tm) {
          currwin[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currtie[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currlos[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currund[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          currwls[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));

          acurrwin[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          acurrtie[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          acurrlos[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          acurrund[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          acurrwls[u]->append(new entry(t, u, res[i][j], lvl, seas, round[i][j]%1000));
          }
        }
      }
    }
  }
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
  printf("\n Longest winning away streaks:\n");
  for (int i=0; i<max; i++) {
    tawin[i]->header(i,1);
    tawin[i]->print();
  }

  printf("\n Longest undefeated away streaks:\n");
  for (int i=0; i<max; i++) {
    taundef[i]->header(i,2);
    taundef[i]->print();
  }

  printf("\n Longest streaks of consecutive away ties:\n");
  for (int i=0; i<max; i++) {
    tatie[i]->header(i,3);
    tatie[i]->print();
  }

  printf("\n Longest winless away streaks:\n");
  for (int i=0; i<max; i++) {
    tawinles[i]->header(i,4);
    tawinles[i]->print();
  }

  printf("\n Longest losing away streaks:\n");
  for (int i=0; i<max; i++) {
/*
    x = talos[i]->head->home;
    if (talos[i]->head->guest == talos[i]->head->next->home || talos[i]->head->guest == talos[i]->head->next->guest) 
      x = talos[i]->head->guest;
//    printf("%2d.%-30s %3d [%d]\n", i+1, NameOf(L, x, maxlos[x]%10000), maxlos[x]/10000, maxlos[x]%10000);
    printf("%2d.%-30s %3d  [%2d %s %d - %2d %s %d]\n", i+1, NameOf(L, x, talos[i]->tail->year), talos[i]->len-1, 
            talos[i]->head->date%50, month[talos[i]->head->date/50], talos[i]->head->year,
            talos[i]->tail->date%50, month[talos[i]->tail->date/50], talos[i]->tail->year
           );
*/
    talos[i]->header(i,5);
    talos[i]->print();
  }

//-----------------------------------------------------------------
  printf("\n Longest home winning streaks:\n");
  for (int i=0; i<max; i++) {
    thwin[i]->header(i,1);
    thwin[i]->print();
  }

  printf("\n Longest undefeated at home streaks:\n");
  for (int i=0; i<max; i++) {
    thundef[i]->header(i,2);
    thundef[i]->print();
  }

  printf("\n Longest streaks of consecutive home ties:\n");
  for (int i=0; i<max; i++) {
    thtie[i]->header(i,3);
    thtie[i]->print();
  }

  printf("\n Longest winless at home streaks:\n");
  for (int i=0; i<max; i++) {
    thwinles[i]->header(i,4);
    thwinles[i]->print();
  }

  printf("\n Longest losing streaks at home:\n");
  for (int i=0; i<max; i++) {
    thlos[i]->header(i,5);
    thlos[i]->print();
  }

//-----------------------------------------------------------------
  printf("\n Longest winning streaks:\n");
  for (int i=0; i<max; i++) {
    twin[i]->header(i,1);
    twin[i]->print();
  }

  printf("\n Longest undefeated streaks:\n");
  for (int i=0; i<max; i++) {
    tundef[i]->header(i,2);
    tundef[i]->print();
  }

  printf("\n Longest streaks of consecutive ties:\n");
  for (int i=0; i<max; i++) {
    ttie[i]->header(i,3);
    ttie[i]->print();
  }

  printf("\n Longest winless streaks:\n");
  for (int i=0; i<max; i++) {
    twinles[i]->header(i,4);
    twinles[i]->print();
  }

  printf("\n Longest losing streaks:\n");
  for (int i=0; i<max; i++) {
    tlos[i]->header(i,5);
    tlos[i]->print();
  }

  return 0;
}


//-----------------------------------------------------------------
int Streaks(int old) {
  char filename[64];
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
      printf("Usage: streak [-n maxentry] [-team id] [-current] [-div D] [-fy year]\n");
      return 1;
    }
    if (strcmp(argv[k], "-n")==0) 
      max = atoi(argv[k+1]);
    else if (strcmp(argv[k], "--old")==0) 
     old = 1;
    else if (strcmp(argv[k], "--team")==0 || strcmp(argv[k], "-t")==0) {
      strcpy(stm, argv[k+1]);
      if (stm[0]>='a' && stm[0]<='z') stm[0] -= 32;
      if (k+2<argc && argv[k+2][0]!='-') {
        strcat(stm, " ");
        char stm2[33];
        strcpy(stm2, argv[k+2]);
        if (stm2[0]>='a' && stm2[0]<='z') stm2[0] -= 32; 
        strcat(stm, stm2);
      }
      tm = Find(stm);
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
  Streaks(old);
  return 0;
}
