#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

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

#define MAX_LEVELS 12
#define MAX_RR      4
#define MAX_N      64
#define MAX_TEAMS 2000
#define MAX_CUPG        20000

#define SPECIAL         50
#define LOSS_BOTH_0     50
#define LOSS_BOTH_9     59

int  NC, ND;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;
int  ****part;

int  id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pts[MAX_N], pen[MAX_N], pdt[MAX_N];
int  round[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N];
int  rank[MAX_N];
char desc[MAX_N][32];
int  tbwin[MAX_N], tbdrw[MAX_N], tblos[MAX_N], tbgsc[MAX_N], tbgre[MAX_N], tbrk[MAX_N];

char inputfile[64], outputfile[64], filename[64];
int awin[MAX_LEVELS][MAX_TEAMS], adrw[MAX_LEVELS][MAX_TEAMS], alos[MAX_LEVELS][MAX_TEAMS], agsc[MAX_LEVELS][MAX_TEAMS], agre[MAX_LEVELS][MAX_TEAMS];
int arnk[MAX_TEAMS];
FILE *of;
int details;

const char *month[] = {"???", "ian", "feb", "mar", "apr", "mai", "iun", 
                              "iul", "aug", "sep", "oct", "noi", "dec"};
const int rlo[] = {  0, 255, 255, 0, 0, 0, 0, 0} ;
const int rhi[] = {191, 255, 240, 0, 0, 0, 0, 0} ;
const int glo[] = {255, 255,   0, 0, 0, 0, 0, 0} ;
const int ghi[] = {255,  63, 240, 0, 0, 0, 0, 0} ;
const int blo[] = {0,   0,     0, 0, 0, 0, 0, 0} ;
const int bhi[] = {0,   0,   240, 0, 0, 0, 0, 0} ;

int n, rr, ppv, tbr;
int d1, rka;

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


int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  int dummy, n, p, t, d, y;
  char *dv, *pl, *yr, *suf;

  f = fopen("webteams.dat", "rt");
  if (!f) return 0;
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;

  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[48];
    club[i] = new char[48];
    strncpy(mnem[i], strtok(s, ","), 32);
    strncpy(club[i], strtok(NULL, ",\n"), 32);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return 0;
  for (int i=0; i<NC; i++) {
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

  part = new int***[MAX_LEVELS];
  FY = new int[MAX_LEVELS];
  LY = new int[MAX_LEVELS];
  MAX = new int [MAX_LEVELS];

  for (int d=0; d<MAX_LEVELS; d++) {
    FY[d] = 2100;
    LY[d] = 1800;
    MAX[d] = 0;
  }

  ND = 0;
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
      for (int d=0; d<ND; d++)
        printf("%c: %d - %d (max: %d)\n", (char) (d+65), FY[d], LY[d], MAX[d]);
  }
  else
   printf("ERROR: Couldn't open the directory.\n");  

  for (int d=0; d<ND; d++) {
    if (MAX[d]>0) {
      part[d] = new int**[LY[d]-FY[d]+1];
      for (int i=FY[d]; i<=LY[d]; i++) {
        part[d][i-FY[d]] = new int*[MAX[d]+1];
        for (int j=0; j<=MAX[d]; j++) {
          part[d][i-FY[d]][j] = new int[64];
        }
      }
    }
  }

// quick data
  for (int d=0; d<ND; d++) {
    sprintf(filename, "part.%c", (char) (d+97));
    f = fopen(filename, "rt");
    for (int y=0; y<=LY[d]-FY[d]; y++) {
      if (f==NULL) part[d][y][0][1] = 0;
      else {
        fscanf(f, "%d %d", &dummy, &n);
        part[d][y][0][0] = dummy;
        part[d][y][0][1] = n;
        for (int i=0; i<n; i++) {
          fscanf(f, "%d", &t); part[d][y][0][i+2] = t;
        }
        fgets(s, 200, f);
      }
    }
    if (f) fclose(f);
  }

  for (int d=0; d<ND; d++) {
    for (int i=1; i<=MAX[d]; i++) {
      sprintf(filename, "part.%c%d", (char) (d+97), i);
      f = fopen(filename, "rt");
      for (int y=0; y<=LY[d]-FY[d]; y++) {
        if (f==NULL) part[d][y][i][1] = 0;
        else {
          fscanf(f, "%d %d", &dummy, &n);
          part[d][y][i][0] = dummy;
          part[d][y][i][1] = n;
          for (int j=0; j<n; j++) {
            fscanf(f, "%d", &t); 
            part[d][y][i][j+2] = t;
          }
          fgets(s, 200, f);
        }
      }
      if (f) fclose(f);
    }
  }

  return 1;
}

int Find(char* s) {
  int found = 0;
  int multi = 0;
  int j;
  int l = strlen(s);

  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  for (int i=0; i<l-1; i++)
    if ((s[i]==32 || s[i]=='.') && s[i+1]>96) s[i+1] -= 32;

  int i = 0;
  while (i < NC) {
    if (strcmp(mnem[i], s)==0) return i;
    i++;
  }

  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
    else i++;
  }
  if (found) return i;

  // try uppercase
  for (int j=0; j<l; j++)
    if (s[j]>='a' && s[j]<='z') s[j] -= 32;
  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
    else i++;
  }
  if (found) return i;
  return -1;
}

int GetUnique(const char *prompt) {
  char name[32];
  int res;
  do {
   printf("%s", prompt); 
   do { fgets(name, 30, stdin); } while (!strlen(name));
   name[strlen(name)-1] = 0;
   res = Find(name);
  } while (res < 0);
  return res;
}

int LoadFile(char *filename) {
  FILE *f;
  int h, i, j, x, y, r, z;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[512], *tok[10];
  fgets(s, 100, f);
  sscanf(s, "%d %d %d", &n, &ppv, &tbr); 
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
    fgets(s, 512, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);   
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);   
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
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
}

int FindId(int t) {
  for (int i=0; i<n; i++)
   if (id[i] == t) return i;
  return -1;
}

int in(int y, int d, int p, int t) {
  if (MAX[d]==0) return 0;
  if (y<FY[d] || y>LY[d]) return 0;
  int m = part[d][y-FY[d]][p][1];
  for (int j=2; j<m+2; j++)
    if (part[d][y-FY[d]][p][j] == t) return j-1;
  return -1;
}

void CupData(int a) {
  char s[128];
  int score[8];
  FILE *f = fopen("cuparchive.dat", "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: cannot find file 'cuparchive.dat'\n");
    return;
  }

  while (!feof(f)) {
    fgets(s, 128, f);
    if (strlen(s) > 10) {
      int home     = 75*((int)(s[7]-48)) + s[8] - 48;
      int guest    = 75*((int)(s[9]-48)) + s[10] - 48;
      if (a!=home && a!=guest) continue;

      int len  = strlen((char *)s);

      for (int i=0; i<8; i++)
        if (i+11 < len) score[i] = s[i+11] - 48;
        else score[i] = -1;
      int gdiff = score[0] - score[1];
           if (gdiff>0) { awin[10][home]++; alos[10][guest]++; }
      else if (gdiff==0) { adrw[10][home]++; adrw[10][guest]++; }
      else               { alos[10][home]++; awin[10][guest]++; }
      agsc[10][home]  += score[0];
      agre[10][home]  += score[1];
      agsc[10][guest] += score[1];
      agre[10][guest] += score[0];
    }
    s[0] = 0;
  }
  fclose(f);
}

void AddStats(int a, int y, int d1, int *pl, int rk) {
  int t;
  char filename[15];


  if (part[0][y-FY[0]][0][1]==0) return;

  int d = d1/100;
  int p = d1%100; 
  if (p==0) sprintf(filename, "%c.%d", d+97, y);
    else sprintf(filename, "%c%d.%d", d+97, p, y);
  LoadFile(filename);
  t = FindId(a);
  if (t>=0) {
    for (int h=0; h<rr; h++) {
    for (int j=0; j<n; j++) {
    if (t!=j) {
      if (res[h][t][j]>=0) {
        int x = res[h][t][j]/100;
        int y = res[h][t][j]%100;

        if (y>=SPECIAL) {
          if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {        
            int yy = y%10;
            alos[d][a]++; alos[d][id[j]]++;
            agre[d][a]+=yy; 
            agre[d][id[j]]+=yy;
          }
        }
        else {
          agre[d][a] += y; agsc[d][a] += x;
          agre[d][id[j]] += x; agsc[d][id[j]] += y;
          if (x>y)  { awin[d][a]++; alos[d][id[j]]++; }
          if (x==y) { adrw[d][a]++; adrw[d][id[j]]++; }
          if (x<y)  { alos[d][a]++; awin[d][id[j]]++; }
        }
      }
      if (res[h][j][t]>=0) {
        int x = res[h][j][t]/100;
        int y = res[h][j][t]%100;
        if (y>=SPECIAL) {
          if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {        
            int yy = y%10;
            alos[d][a]++; alos[d][id[j]]++;
            agre[d][a]+=yy; 
            agre[d][id[j]]+=yy;
          }
        }
        else {
          agsc[d][a] += y; agre[d][a] += x;
          agsc[d][id[j]] += x; agre[d][id[j]] += y;
          if (x>y)  { alos[d][a]++; awin[d][id[j]]++; }
          if (x==y) { adrw[d][a]++; adrw[d][id[j]]++; }
          if (x<y)  { awin[d][a]++; alos[d][id[j]]++; }
        }
      }
    } 
    }
    }
  }
}

int sup(int i, int j) {
  int wi = 0; for (int d=0; d<MAX_LEVELS; d++) wi = wi + awin[d][i];
  int wj = 0; for (int d=0; d<MAX_LEVELS; d++) wj = wj + awin[d][j];
  int di = 0; for (int d=0; d<MAX_LEVELS; d++) di = di + adrw[d][i];
  int dj = 0; for (int d=0; d<MAX_LEVELS; d++) dj = dj + adrw[d][j];
  int li = 0; for (int d=0; d<MAX_LEVELS; d++) li = li + alos[d][i];
  int lj = 0; for (int d=0; d<MAX_LEVELS; d++) lj = lj + alos[d][j];
  if (wi+di+li==0) return 0;
  if (wj+dj+lj==0) return 1;
  int pi = 2*wi + di;
  int pj = 2*wj + dj;
  if (pi > pj) return 1;
  if (pi < pj) return 0;
  int gsi = 0; for (int d=0; d<MAX_LEVELS; d++) gsi = gsi + agsc[d][i];
  int gsj = 0; for (int d=0; d<MAX_LEVELS; d++) gsj = gsj + agsc[d][j];
  int gri = 0; for (int d=0; d<MAX_LEVELS; d++) gri = gri + agre[d][i];
  int grj = 0; for (int d=0; d<MAX_LEVELS; d++) grj = grj + agre[d][j];
  int gdi = gsi - gri;
  int gdj = gsj - grj;
  if (gdi > gdj) return 1;
  if (gdi < gdj) return 0;
  if (wi > wj) return 1;
  if (wi < wj) return 0;
  if (gsi > gsj) return 1;
  if (gsi < gsj) return 0;
  return 0;
}

void Ranking() {
  for (int i=0; i<NC; i++) arnk[i] = i;
  int i, aux, sorted;
  int last = NC-1;
  do {
      sorted = 1;
      for (i=0; i<=last-1; i++) {
         if (sup(arnk[i+1], arnk[i])) {
           sorted = 0;
           aux = arnk[i];
           arnk[i] = arnk[i+1];
           arnk[i+1] = aux;
         }
      }
      last--;
  } while (!sorted);
}

void Palmares(int a) {
  int d1, rk1, pl[20], found;
  char s[200];

  sprintf(outputfile, "html/palmares-%d.html", a);
  of = fopen(outputfile, "wt");
  if (!of) return;

  for (int i=0; i<NC; i++) {
    for (int j=0; j<MAX_LEVELS; j++) {
      awin[j][i] = adrw[j][i] = alos[j][i] = agsc[j][i] = agre[j][i] = 0;
    }
  }

  for (int y = fd; y<=ld; y++) {
    if (part[0][y-FY[0]][0][1] == 0 && part[0][y-FY[0]][1][1] == 0) continue;

    found = 0;
    rka = -1;
    for (int d=ND-1; d>=0; d--) {
      for (int p = 0; p < 20; p++) pl[p] = 0;
      for (int p = MAX[d]+1; p>=0; p--) {
        rk1 = in(y, d, p%(MAX[d]+1), a);
        if (rk1>0) { 
          rka = rk1; 
          d1 = 100*d+p%(MAX[d]+1); 
          pl[p] = 1;
          found = 1;
        }
      }
    }
    if (rka>0) {
       AddStats(a, y, d1, pl, rka);
    }
  }
  CupData(a);

  Ranking();

  int ng[MAX_LEVELS];

  fprintf(of, "<HTML>\n<TITLE>Palmares %s</TITLE>\n", NameOf(L, a, 3000));
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");
  fprintf(of, "<H2>Palmares %s</H2>\n", NameOf(L, a, 3000));

  fprintf(of, "<TABLE WIDTH=\"50%%\" BORDER=\"1\" CELLPADDING=\"5\" RULES=\"groups\" FRAME=\"box\">\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"1\"></COLGROUP>\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"9\"></COLGROUP>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH ALIGN=\"left\">Divizia</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"7%%\">J</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"9%%\">V</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"7%%\">E</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"7%%\">Î</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"12%%\">gm</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\">-</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"5%%\">gp</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"10%%\">P</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\" WIDTH=\"12%%\">%%</TH>\n");
  fprintf(of, "</TR>\n");
    for (int j=0; j<MAX_LEVELS; j++) ng[j] = 0;
    for (int j=0; j<MAX_LEVELS; j++) ng[j] += awin[j][a]+adrw[j][a]+alos[j][a];
   
    int ngk = 0;
    for (int j=0; j<MAX_LEVELS; j++) ngk = ngk+ng[j];
    if (ngk>0) {
      int wink = 0; for (int j=0; j<MAX_LEVELS; j++) wink = wink + awin[j][a];
      int drwk = 0; for (int j=0; j<MAX_LEVELS; j++) drwk = drwk + adrw[j][a];
      int losk = 0; for (int j=0; j<MAX_LEVELS; j++) losk = losk + alos[j][a];
      int gsck = 0; for (int j=0; j<MAX_LEVELS; j++) gsck = gsck + agsc[j][a];
      int grek = 0; for (int j=0; j<MAX_LEVELS; j++) grek = grek + agre[j][a];
      fprintf(of, "<TR BGCOLOR=\"EEEEEE\">");
      fprintf(of, "<TD ALIGN=\"left\">Total</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      fprintf(of, "</TR>\n");
      for (int j=0; j<MAX_LEVELS; j++) {
        if (ng[j]>0) {
          fprintf(of, "<TR BGCOLOR=\"FFFFFF\">");
          if (j<ND) fprintf(of, "<TD>Divizia %c</TD>", (char)(j+65));
          else if (j==10) fprintf(of, "<TD>Cupã</TD>");
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng[j]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", awin[j][a]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", adrw[j][a]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", alos[j][a]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", agsc[j][a]);
          fprintf(of, "<TD ALIGN=\"center\">-</TD>");
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", agre[j][a]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*awin[j][a]+adrw[j][a]);
          fprintf(of, "<TD ALIGN=\"right\">[%3d%%]</TD>", (int) ((100.0*awin[j][a]+50*adrw[j][a]) / (ng[j])));
          fprintf(of, "</TR>\n");
        }
      }
    }
  fprintf(of, "</TABLE><BR>\n\n");


  fprintf(of, "<TABLE  WIDTH=\"99%%\" BORDER=\"1\" CELLPADDING=\"1\" RULES=\"groups\" FRAME=\"box\">\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"2\"></COLGROUP>\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"10\"></COLGROUP>\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"10\"></COLGROUP>\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"10\"></COLGROUP>\n");
  fprintf(of, "<COLGROUP><COL SPAN=\"10\"></COLGROUP>\n");
  fprintf(of, "<TBODY>\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<TH ALIGN=\"right\">#</TH>");
  fprintf(of, "<TH ALIGN=\"left\">Echipa</TH>");
  fprintf(of, "<TH ALIGN=\"center\" COLSPAN=\"10\">Total</TH>");
  fprintf(of, "<TH ALIGN=\"center\" COLSPAN=\"10\">Divizia A</TH>");
  fprintf(of, "<TH ALIGN=\"center\" COLSPAN=\"10\">Divizia B</TH>");
  fprintf(of, "<TH ALIGN=\"center\" COLSPAN=\"10\">Divizia C</TH>");
  fprintf(of, "</TR>\n");
  fprintf(of, "<TR BGCOLOR=\"EEEEEE\">");
  fprintf(of, "<TD ALIGN=\"right\"></TD>");
  fprintf(of, "<TD ALIGN=\"left\"></TD>");
  for (int k=0; k<4; k++) {
  fprintf(of, "<TD ALIGN=\"right\"></TD>");
  fprintf(of, "<TD ALIGN=\"right\">J</TD>");
  fprintf(of, "<TD ALIGN=\"right\">V</TD>");
  fprintf(of, "<TD ALIGN=\"right\">E</TD>");
  fprintf(of, "<TD ALIGN=\"right\">Î</TD>");
  fprintf(of, "<TD ALIGN=\"right\">gm</TD>");
  fprintf(of, "<TD ALIGN=\"center\">-</TD>");
  fprintf(of, "<TD ALIGN=\"right\">gp</TD>");
  fprintf(of, "<TD ALIGN=\"right\">P</TD>");
  fprintf(of, "<TD ALIGN=\"right\">%%</TD>");
  }
  fprintf(of, "</TR>\n");
  fprintf(of, "</TBODY>\n");

  for (int i=0; i<NC; i++) {
    int k = arnk[i];
    for (int j=0; j<MAX_LEVELS; j++) ng[j] = 0;
    for (int j=0; j<MAX_LEVELS; j++) ng[j] += awin[j][k]+adrw[j][k]+alos[j][k];
    int ngk = 0 ; for (int j=0; j<MAX_LEVELS; j++) ngk = ngk + ng[j];
    if (a!=k && ngk>0) {
      int wink = 0; for (int j=0; j<MAX_LEVELS; j++) wink = wink + awin[j][k];
      int drwk = 0; for (int j=0; j<MAX_LEVELS; j++) drwk = drwk + adrw[j][k];
      int losk = 0; for (int j=0; j<MAX_LEVELS; j++) losk = losk + alos[j][k];
      int gsck = 0; for (int j=0; j<MAX_LEVELS; j++) gsck = gsck + agsc[j][k];
      int grek = 0; for (int j=0; j<MAX_LEVELS; j++) grek = grek + agre[j][k];
      fprintf(of, "<TBODY><TR BGCOLOR=\"%s\">", (i%2==1?"FFFFFF":"DDDDFF"));
      fprintf(of, "<TD ALIGN=\"right\">%d.</TD>", i);
      if (a!=k) fprintf(of, "<TD ALIGN=\"left\"><A HREF=\"vs-%04d/vs-%d-%d.html\">%s</A></TD>", a, a, k, NameOf(L, k, 3000));
      else fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NameOf(L, a, 3000));
      fprintf(of, "<TD></TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ngk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", wink);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drwk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", losk);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsck);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", grek);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*wink+drwk);
      fprintf(of, "<TD ALIGN=\"right\">[%3d]</TD>", (int) ((100.0*wink+50*drwk) / (ngk)));
      for (int j=0; j<3; j++) {
        if (ng[j]>0) {
          fprintf(of, "<TD></TD>");
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng[j]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", awin[j][k]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", adrw[j][k]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", alos[j][k]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", agsc[j][k]);
          fprintf(of, "<TD ALIGN=\"center\">-</TD>");
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", agre[j][k]);
          fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*awin[j][k]+adrw[j][k]);
          fprintf(of, "<TD ALIGN=\"right\">[%3d]</TD>", (int) ((100.0*awin[j][k]+50*adrw[j][k]) / (ng[j])));
        }
        else for (int h=0; h<10; h++) fprintf(of, "<TD></TD>");
      }
    fprintf(of, "</TR></TBODY>\n");
    }
  }

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);

}

//---------------------------------------------

int main(int argc, char* argv[]) {
  int a, b;
  details = 0;

  a = -1;
  for (int k=1; k<argc; k++) {
    if (strstr(argv[k], "-d")==argv[k]) details = 1;
    if (strcmp(argv[k], "-fd")==0) {
      if (k+1<argc) fd = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-ld")==0) {
      if (k+1<argc) ld = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-t")==0) {
      if (k+1<argc) a = atoi(argv[k+1]);
    }
  }

  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }

  if (fd < FY[0]) fd = FY[0];
  if (ld < fd || ld > LY[0]) ld = LY[0];

  if (a<0) {
//    a = GetUnique("Team : ");
    for (int i=0; i<NC; i++)  {
      printf("%3d.%s\n", i+1, NameOf(L, i, 3000));
      Palmares(i);
    }
  }
  else {
    Palmares(a);
  }
  return 0;
}
