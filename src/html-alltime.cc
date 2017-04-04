#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS 12
#define MAX_TEAMS 2000
#define MAX_RR      4
#define MAX_N      64
#define NUMORD     10

#define SPECIAL         50
#define LOSS_BOTH_0     50
#define LOSS_BOTH_9     59

int  NC, ND;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld, afd, ald;

int id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pts[MAX_N], pen[MAX_N], pdt[MAX_N];
int round[MAX_RR][MAX_N][MAX_N], res[MAX_RR][MAX_N][MAX_N];
int rank[MAX_N];
int aed[3][MAX_TEAMS], awin[3][MAX_TEAMS], adrw[3][MAX_TEAMS], alos[3][MAX_TEAMS], agsc[3][MAX_TEAMS], agre[3][MAX_TEAMS];
int pld[3][MAX_TEAMS][200];
int last[3][MAX_TEAMS];
int laste[MAX_LEVELS];
int arnk[MAX_TEAMS];

char inputfile[64], outputfile[64], filename[64];
FILE *of;

int n, ppv, tbr, rr;
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

int LoadFile(char *filename) {
  FILE *f;
  int h, i, j, x, y, r, z;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }

  // Loading file
  char s[100], *tok[10], t[100];
  int d = (int) (filename[0]-97);
  strcpy(t, filename);
  char *dv = strtok(t, ".");
  char *yr = strtok(NULL, ".");
  int year = atoi(yr);
  if (year>laste[d]) for (i=0; i<MAX_TEAMS; i++) last[d][i] = 0;

  fgets(s, 100, f);
  sscanf(s, "%d %d %d", &n, &ppv, &tbr); 
  rr = tbr/NUMORD + 1;

  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    if (pld[d][id[i]][year-1900]==0) {
      aed[d][id[i]]++;
      pld[d][id[i]][year-1900]=1;
    }
    if (year>=laste[d]) last[d][id[i]] = 1;
  }

  for (h=0; h<rr; h++) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      if (i!=j && z>=0) {
        y = z % 100;
        x = (int) (z/100);
        if (y>=SPECIAL) {
          if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
            int yy = y%10;
            alos[d][id[i]]++; alos[d][id[j]]++;
            agre[d][id[i]]+=yy; agre[d][id[i]]+=yy;
          }
        }
        else {
          agsc[d][id[i]] += x; agre[d][id[i]] += y;
          agsc[d][id[j]] += y; agre[d][id[j]] += x;
          if (x>y) { awin[d][id[i]]++; alos[d][id[j]]++; }
          else if (x==y) { adrw[d][id[i]]++; adrw[d][id[j]]++; }
          else { alos[d][id[i]]++; awin[d][id[j]]++; }
        }
      }
    }
    fscanf(f, "\n");
  }
  }
  fclose(f);
  if (year>laste[d]) laste[d] = year;
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
          if (d<0 || d>=3) continue;
          if (d==0 && l>1) continue;
          y = atoi(yr);
          if (y>=fd && y<=ld) {
            LoadFile(ep->d_name);
            if (y<afd) afd = y;
            if (y>ald) ald = y;
          }
        }
      }
      closedir(dp);  
      for (int d=0; d<ND; d++)
        printf("%c: %d - %d (max: %d)\n", (char) (d+65), FY[d], LY[d], MAX[d]);
  }
  else
   printf("ERROR: Couldn't open the directory.\n");  

  return 1;
}

void AddStats(int y, int d) {
  for (int h=0; h<rr; h++) {
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      int a = id[i];
      int b = id[j];      
      if (i!=j && res[h][i][j]>=0) {
        int x = res[h][i][j]/100;
        int y = res[h][i][j]%100;
        if (y>=SPECIAL) {
          if (y>=LOSS_BOTH_0 && y<=LOSS_BOTH_9) {
            int yy = y%10;
            alos[d][id[i]]++; alos[d][id[j]]++;
            agre[d][id[i]]+=yy; agre[d][id[i]]+=yy;
          }
        }
        else {
          agre[d][a] += y; agsc[d][a] += x;
          agre[d][b] += x; agsc[d][b] += y;
          if (x>y)  { awin[d][a]++; alos[d][b]++; }
          if (x==y) { adrw[d][a]++; adrw[d][b]++; }
          if (x<y)  { alos[d][a]++; awin[d][b]++; }
        }
      }
    } 
  }
  }
}

int sup(int d, int i, int j) {
  int wi = awin[d][i];
  int wj = awin[d][j];
  int di = adrw[d][i];
  int dj = adrw[d][j];
  int li = alos[d][i];
  int lj = alos[d][j];
  if (wi+di+li==0) return 0;
  if (wj+dj+lj==0) return 1;
  int pi = 2*wi + adrw[d][i];
  int pj = 2*wj + adrw[d][j];
  if (pi > pj) return 1;
  if (pi < pj) return 0;
  int gsi = agsc[d][i];
  int gsj = agsc[d][j];
  int gdi = gsi - agre[d][i];
  int gdj = gsj - agre[d][j];
  if (gdi > gdj) return 1;
  if (gdi < gdj) return 0;
  if (wi > wj) return 1;
  if (wi < wj) return 0;
  if (gsi > gsj) return 1;
  if (gsi < gsj) return 0;
  return 0;
}

void Ranking(int d) {
  for (int i=0; i<NC; i++) arnk[i] = i;
  int i, aux, sorted;
  int last = NC-1;
  do {
      sorted = 1;
      for (i=0; i<=last-1; i++) {
         if (sup(d, arnk[i+1], arnk[i])) {
           sorted = 0;
           aux = arnk[i];
           arnk[i] = arnk[i+1];
           arnk[i+1] = aux;
         }
      }
      last--;
  } while (!sorted);
}

void Alltime(int d) {
  char s[200];

  sprintf(outputfile, "html/alltime-%c.html", (char)(d+97));
  of = fopen(outputfile, "wt");
  if (!of) return;

  Ranking(d);

  fprintf(of, "<HTML>\n<TITLE>Clasament alltime %c (%d-%d)</TITLE>\n", (char)(d+65), afd, ald);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<H2>Clasament <I>all-time</I> %c (%d-%d)</H2>\n", (char)(d+65), afd, ald);
  fprintf(of, "<TABLE BORDER=\"1\" WIDTH=\"80%%\" CELLPADDING=\"2\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">");
  fprintf(of, "<TH WIDTH=\"2%%\" ALIGN=\"right\">#</TH>");
  fprintf(of, "<TH WIDTH=\"40%%\" ALIGN=\"left\">Echipa</TH>");
  fprintf(of, "<TH WIDTH=\"3%%\" ALIGN=\"center\">S</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">J</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">V</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">E</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">Î</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">gm</TH>");
  fprintf(of, "<TH WIDTH=\"1%%\" ALIGN=\"center\">-</TH>");
  fprintf(of, "<TH ALIGN=\"center\">gp</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">P</TH>");
  fprintf(of, "<TH WIDTH=\"7%%\" ALIGN=\"center\">%%</TH>");
  fprintf(of, "</TR>\n");

  for (int i=0; i<NC; i++) {
    int k = arnk[i];
    int ng = awin[d][k]+adrw[d][k]+alos[d][k];
    if (ng>0) {
      fprintf(of, "<TR BGCOLOR=\"%s\">", (i%2==1?"FFFFFF":"DDDDFF"));
      fprintf(of, "<TD ALIGN=\"right\">%d.</TD>", i+1);
      fprintf(of, "<TD ALIGN=\"left\"><A HREF=\"istoric-%d.html\">%s%s%s</A></TD>", k, (last[d][k]>0?"<B>":""), NameOf(L, k, 3000), (last[d][k]>0?"</B>":""));
      fprintf(of, "<TD ALIGN=\"right\">(%d)</TD>", aed[d][k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", awin[d][k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", adrw[d][k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", alos[d][k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", agsc[d][k]);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", agre[d][k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*awin[d][k]+adrw[d][k]);
      fprintf(of, "<TD ALIGN=\"right\">[%3d]</TD>", (int) ((100.0*awin[d][k]+50*adrw[d][k]) / (ng)));
      fprintf(of, "</TR>\n");
    }
  }

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

//---------------------------------------------

int main(int argc, char* argv[]) {

  ald = fd = 0;
  afd = ld = 3000;

  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-fd")==0) {
      if (k+1<argc) fd = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-ld")==0) {
      if (k+1<argc) ld = atoi(argv[k+1]);
    }
  }

  for (int d=0; d<3; d++) {
    laste[d] = 0;
    for (int i=0; i<MAX_TEAMS; i++) 
      for (int y=0; y<200; y++) pld[d][i][y] = 0;
  }

  for (int d=0; d<3; d++) { 
    for (int i=0; i<MAX_TEAMS; i++) {
      last[d][i] = aed[d][i] = awin[d][i] = adrw[d][i] = alos[d][i] = agsc[d][i] = agre[d][i] = 0;
    }
  }

  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }

  Alltime(0);
  Alltime(1);
  Alltime(2);

  return 0;
}
