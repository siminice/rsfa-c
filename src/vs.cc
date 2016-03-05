#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS 12

int  NC, ND;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;
int  ****part;
int id[64], win[64], drw[64], los[64], gsc[64], gre[64];
int pts[64], pen[64], pdt[64], rank[64];
int round[64][64], round2[64][64], res[64][64], res2[64][64];
int  tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
char inputfile[64], *outputfile, *filename;
int details;

const char *normal  = "\033[0m";
const char *black   = "\033[30m";
const char *red     = "\033[31m";
const char *green   = "\033[32m";
const char *yellow  = "\033[33m";
const char *blue    = "\033[34m";
const char *magenta = "\033[35m";
const char *cyan    = "\033[36m";
const char *white   = "\033[37m";
const char *loblack   = "\033[30;0m";
const char *lored     = "\033[31;0m";
const char *logreen   = "\033[32;0m";
const char *loyellow  = "\033[33;0m";
const char *loblue    = "\033[34;0m";
const char *lomagenta = "\033[35;0m";
const char *locyan    = "\033[36;0m";
const char *lowhite   = "\033[37;0m";
const char *hiblack   = "\033[30;1m";
const char *hired     = "\033[31;1m";
const char *higreen   = "\033[32;1m";
const char *hiyellow  = "\033[33;1m";
const char *hiblue    = "\033[34;1m";
const char *himagenta = "\033[35;1m";
const char *hicyan    = "\033[36;1m";
const char *hiwhite   = "\033[37;1m";

const char *colors[] = {black, red, green, yellow, blue, magenta, cyan, white}; 
const char *locols[] = {loblack, lored, logreen, loyellow, loblue, lomagenta, locyan, lowhite}; 
const char *hicols[] = {hiblack, hired, higreen, hiyellow, hiblue, himagenta, hicyan, hiwhite}; 
const char *month[] = {"???", "jan", "feb", "mar", "apr", "may", "jun", 
                              "jul", "aug", "sep", "oct", "nov", "dec"};

int n, ppv, tbr;
int *wh, *dh, *lh, *sh, *rh;
int *wg, *dg, *lg, *sg, *rg;
int d1, d2, rka, rkb;
int start, last;

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

  f = fopen("teams.dat", "rt");
  if (!f) return 0;
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;

  for (int i=0; i<NC; i++) {
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
//      printf("Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
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
//            printf("%s: %d %d %d\n", ep->d_name, d, p, y);
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
  filename = new char[20];
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

  wh = new int[ND+1];
  dh = new int[ND+1];
  lh = new int[ND+1];
  sh = new int[ND+1];
  rh = new int[ND+1];
  wg = new int[ND+1];
  dg = new int[ND+1];
  lg = new int[ND+1];
  sg = new int[ND+1];
  rg = new int[ND+1];
  for (int d=0; d<ND+1; d++) {
    wh[d] = dh[d] = lh[d] = sh[d] = rh[d] = 0;
    wg[d] = dg[d] = lg[d] = sg[d] = rg[d] = 0;
  }

  return 1;
}

int sup(int i, int j, int tbr=0) {
  if (tbr) {
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    return sup(i, j, 0);
  }
  else {
    int p1 = pts[i] - pen[i];
    int p2 = pts[j] - pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (i > j) return 0;
    return 1; 
  }   
}
    
void Tiebreak(int h, int k) {
  for (int i=h; i<=k; i++) {
    int j = rank[i];
    tbwin[j] = tbdrw[j] = tblos[j] = tbgsc[j] = tbgre[j] = 0;
    tbrk[i] = rank[i];
  }
  for (int i=h; i<=k; i++) {
    for (int j=h; j<=k; j++) {
      int t1 = rank[i];   
      int t2 = rank[j];
      if (res[t1][t2] >= 0) {
        int x = res[t1][t2] / 100;
        int y = res[t1][t2] % 100;
        tbgsc[t1] += x; tbgre[t2] += x;
        tbgsc[t2] += y; tbgre[t1] += y;
        if (x>y) {tbwin[t1]++; tblos[t2]++;}
        else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
        else {tbwin[t2]++; tblos[t1]++;}
      }
      if (res2[t1][t2] >= 0) {
        int x = res2[t1][t2] / 100;
        int y = res2[t1][t2] % 100;
        tbgsc[t1] += x; tbgre[t2] += x;
        tbgsc[t2] += y; tbgre[t1] += y;
        if (x>y) {tbwin[t1]++; tblos[t2]++;}
        else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
        else {tbwin[t2]++; tblos[t1]++;}
      }
    }
  }
        
  int sorted;
  do {
    sorted = 1;
    for (int i=h; i<k; i++)
      if (sup(tbrk[i+1], tbrk[i], 1)) {
        sorted = 0;
        int aux = tbrk[i];
        tbrk[i] = tbrk[i+1];
        tbrk[i+1] = aux;
      }
  } while (!sorted);
  for (int i=h; i<=k; i++)
    rank[i] = tbrk[i];
}       

void Ranking() {
  // BubbleSort
  int i, j;
  for (i=0; i<n; i++) rank[i] = i;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<n-1; i++)
      if (sup(rank[i+1], rank[i])) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
  } while (!sorted);
  if (tbr) {
    i = 0;
    while (i<n-1) {
      j = i;
      while (j+1<n && (pts[rank[j+1]]-pen[rank[j+1]] == pts[rank[i]]-pen[rank[i]])) j++;
      if (j>i) Tiebreak(i,j);  
      i = j+1;
    }
  }
}

void Listing() {
  printf("\nStandings:\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    printf("%2d.%-30s%2d%3d%3d%3d%4d-%2d%3d\n", i+1,
     club[id[x]], win[x]+drw[x]+los[x], 
     win[x], drw[x], los[x], gsc[x], gre[x], pts[x]);
  }
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

int AddResult(int rd, int a, int b, int x, int y) {
  return 1;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[100], *tok[10];
  fgets(s, 100, f);
  sscanf(s, "%d %d %d", &n, &ppv, &tbr); 
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
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
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      res[i][j]   = z;
      round2[i][j] = res2[i][j] = -1;
      if (z >=0) {
        y = z % 100;
        x = (int) (z/100);
//        gsc[i] += x; gre[i] += y;
//        gsc[j] += y; gre[j] += x;
//        if (x>y) { win[i]++; los[j]++; pts[i] += ppv; }
//          else if (x==y) { drw[i]++; drw[j]++; pts[i]++; pts[j]++; }
//           else { los[i]++; win[j]++; pts[j] += ppv; }
      }
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        round2[i][j] = r;
        res2[i][j] = z;
        if (z >=0) {
          y = z % 100;
          x = (int) (z/100);
//          gsc[i] += x; gre[i] += y;
//          gsc[j] += y; gre[j] += x;
//          if (x>y) { win[i]++; los[j]++; pts[i] += ppv; }
//            else if (x==y) { drw[i]++; drw[j]++; pts[i]++; pts[j]++; }
//             else { los[i]++; win[j]++; pts[j] += ppv; }
        }
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

void strdiv(int d, char *s) {
  if (d>=0 && d<ND*100) {
    if (d%100>0) sprintf(s, "%s%c%2d", hicols[7-d/100], (char) (d/100+65), d%100);
    else sprintf(s, "%s%c  ", hicols[7-d/100], (char) (d/100+65));
  }
  else sprintf(s, " - ");
}

void StatLine(int a, int y, int d, int *pl) {
  if (part[0][y-FY[0]][0][1] == 0 && part[0][y-FY[0]][1][1] == 0) return;
  int t;
  int ywin, ydrw, ylos, ygsc, ygre;
	ywin = ydrw = ylos = ygsc = ygre = 0;
  char filename[15];
  for (int p=0; p<=MAX[d]; p++) if (pl[p]>0) {
    if (p==0) sprintf(filename, "%c.%d", d+97, y);
      else sprintf(filename, "%c%d.%d", d+97, p, y);
    LoadFile(filename);
    int t = FindId(a);
    if (t>=0) {
      ywin += win[t];
      ydrw += drw[t];
      ylos += los[t];
      ygsc += gsc[t];
      ygre += gre[t];
    }
  }

      if (ywin+ydrw+ylos>0) {
        printf("%2d%3d%3d%3d%4d-%2d%4dp [%3d %% ]",
        ywin+ydrw+ylos,
        ywin, ydrw, ylos, ygsc, ygre,
        2*ywin+ydrw,
        (int) (100.0*ywin+50*ydrw) / (ywin+ydrw+ylos));
        return;
      }
}

void Extract(int a, int b, int y, int d, int *pl) {
  char filename[15], str[500], s[500], sd[20], sdet[500], saux[500];

  int fp = 0;
  while (fp<20 && pl[fp]==0) fp++;
  strdiv(100*d+fp, sd);
  sprintf(s, "%d %s[%s|%s] ", y, hicols[7-d], sd, sd);

//  sprintf(str, "%s", (d==0?white:normal);
//  strcat(s, str);
  if (a==b) {
    sprintf(str, "%2d.%-32s", rka, NameOf(L, a, y));
    strcat(s, str);
    printf("%s", s);
    StatLine(a, y, d, pl);
    printf("%s\n", normal);
    return;
  }
  sprintf(str, "%2d.%-15s- %2d.%-15s%s", rka, NickOf(L, a, y), rkb, NickOf(L, b, y), normal);
  strcat(s, str);

  for (int p=0; p<=MAX[d]; p++) if (pl[p]>0) {
   if (p==0) sprintf(filename, "%c.%d", d+97, y);
     else sprintf(filename, "%c%d.%d", d+97, p, y);
   LoadFile(filename);
   int t = FindId(a);
   int u = FindId(b);
//   printf("%2d.%-15s- %2d.%-15s", rk1, mnem[a], rk2, mnem[b]);
   if (res[t][u]>=0 || res[u][t]>=0) {
    if (res[t][u] >= 0) {
      int s1 = res[t][u]/100;
      int s2 = res[t][u]%100;
      if (s1>s2) sprintf(str, " %s%d-%d%s", higreen, s1, s2, normal);
      else if (s1==s2) sprintf(str, " %d-%d", s1, s2);
      else sprintf(str, " %s%d-%d\%s", hired, s1, s2, normal);
      if (details) { 
        sprintf(saux, " [%2d/%02d.%02d]", round[t][u]/1000, round[t][u]%50, (round[t][u]%1000)/50);
        strcat(str, saux);
      }
      sh[d]+=s1; rh[d]+=s2;
      sh[ND]+=s1; rh[ND]+=s2;
      if (s1>s2) { wh[d]++; wh[ND]++; } 
      else if (s1==s2) { dh[d]++; dh[ND]++; }
      else { lh[d]++; lh[ND]++; }
    } 
    else sprintf(str, "    ");
    strcat(s, str);
    if (res[u][t] >= 0) {
      int s3 = res[u][t]/100;
      int s4 = res[u][t]%100;
      if (s4>s3) sprintf(str, " %s%d-%d%s", higreen, s4, s3, normal);
      else if (s4==s3) sprintf(str, " %d-%d", s4, s3);
      else sprintf(str, " %s%d-%d%s", hired, s4, s3, normal);
      if (details) {
        sprintf(saux, " [%2d/%02d.%02d]", round[u][t]/1000, round[u][t]%50, (round[u][t]%1000)/50);
        strcat(str, saux);
      }
      sg[d]+=s4; rg[d]+=s3;
      sg[ND]+=s4; rg[ND]+=s3;
//      if (s4>s3) wg++; else if (s4==s3) dg++; else lg++;
      if (s4>s3) { wg[d]++; wg[ND]++; } 
      else if (s4==s3) { dg[d]++; dg[ND]++; }
      else { lg[d]++; lg[ND]++; }
    } else sprintf(str, "    ");
    strcat(s, str);
   } // if res

   // double-rounds of results
   if (res2[t][u]>=0 || res2[u][t]>=0) {
    if (res2[t][u] >= 0) {
      int s1 = res2[t][u]/100;
      int s2 = res2[t][u]%100;
      if (s1>s2) sprintf(str, " %s%d-%d%s", higreen, s1, s2, normal);
      else if (s1==s2) sprintf(str, " %d-%d", s1, s2);
      else sprintf(str, " %s%d-%d\%s", hired, s1, s2, normal);
      if (details) {
        sprintf(saux, " [%2d/%02d.%02d]", round2[t][u]/1000, round2[t][u]%50, (round2[t][u]%1000)/50);
        strcat(str, saux);
      }
      sh[d]+=s1; rh[d]+=s2;
      sh[ND]+=s1; rh[ND]+=s2;
      if (s1>s2) { wh[d]++; wh[ND]++; } 
      else if (s1==s2) { dh[d]++; dh[ND]++; }
      else { lh[d]++; lh[ND]++; }
    } 
    else sprintf(str, "    ");
    strcat(s, str);
    if (res2[u][t] >= 0) {
      int s3 = res2[u][t]/100;
      int s4 = res2[u][t]%100;
      if (s4>s3) sprintf(str, " %s%d-%d%s", higreen, s4, s3, normal);
      else if (s4==s3) sprintf(str, " %d-%d", s4, s3);
      else sprintf(str, " %s%d-%d%s", hired, s4, s3, normal);
      if (details) {
        sprintf(saux, "[%2d/%02d.%02d]", round2[u][t]/1000, round2[u][t]%50, (round2[u][t]%1000)/50);
        strcat(str, saux);
      }
      sg[d]+=s4; rg[d]+=s3;
      sg[ND]+=s4; rg[ND]+=s3;
//      if (s4>s3) wg++; else if (s4==s3) dg++; else lg++;
      if (s4>s3) { wg[d]++; wg[ND]++; } 
      else if (s4==s3) { dg[d]++; dg[ND]++; }
      else { lg[d]++; lg[ND]++; }
    } else sprintf(str, "    ");
    strcat(s, str);
   } // if res
  } // for p
  printf("%s\n%s", s, normal);
}

void Head2Head(int a, int b) {
  int d1, d2, rk1, rk2, stork1, stork2, pl[20];
  char strd1[25], strd2[25], s[200];

  for (int d=0; d<ND+1; d++) {
    wh[d] = dh[d] = lh[d] = sh[d] = rh[d] = 0;
    wg[d] = dg[d] = lg[d] = sg[d] = rg[d] = 0;
  }

  start = 0; last = 3000;

  for (int y = fd; y<=ld; y++) {
    if (part[0][y-FY[0]][0][1] == 0 && part[0][y-FY[0]][1][1] == 0) continue;
    printf("%s", normal);
    d1 = d2 = rka = rkb = -1;
    strcpy(s, "");
    int ok = 0;

    for (int d=0; d<ND; d++) {
      int found = 0;
      for (int p = 0; p < 20; p++) pl[p] = 0;
      for (int p = 1; p <= MAX[d]+1; p++) {
        rk1 = in(y, d, p%(MAX[d]+1), a);
        rk2 = in(y, d, p%(MAX[d]+1), b);
        if (rk1>0 && (rka<0 || p==MAX[d]+1)) { rka = rk1; d1 = 100*d+p%(MAX[d]+1); }
        if (rk2>0 && (rkb<0 || p==MAX[d]+1)) { rkb = rk2; d2 = 100*d+p%(MAX[d]+1); }
        if (rk1>0 && rk2>0) { pl[p%(MAX[d]+1)] = 1; found = 1; }
      }
      if (d1>=0 || d2>=0) {
        if (start>0 && y>last+1) {
          for (int yy=last+1; yy<y; yy++) {
            if (part[0][yy-FY[0]][0][1]>0) printf("%d [ - | - ] \n", yy);
          }
        }
        if (found) {
          Extract(a, b, y, d, pl);
        }
        start = 1;
        last = y;
      }
      ok += found;
    }
    if (ok==0 && start>0 && (d1>=0 || d2>=0)) {
      strdiv(d1, strd1); strdiv(d2, strd2);
      if (a==b) printf("%d  -\n", y); 
      else printf("%d [%s|%s] ", y, strd1, strd2);
      char strrk1[25], strrk2[25];
      if (d1<ND*100 && rka>0) sprintf(strrk1, "%s%2d.", hicols[7-d1/100], rka); 
        else strcpy(strrk1, "   ");
      if (d2<ND*100 && rkb>0) sprintf(strrk2, "%s%2d.", hicols[7-d2/100], rkb); 
        else strcpy(strrk2, "   ");
      if (a!=b) printf("%s%-15s- %s\n%s", strrk1, " ", strrk2, normal);
    }
  }

  int ng = wh[ND]+dh[ND]+lh[ND]+wg[ND]+dg[ND]+lg[ND];
  if (ng>0) {
    printf("\n   Overall: (%3d) %3d %3d %3d %4d-%3d %6.2f%%\n",
      ng, wh[ND]+wg[ND], dh[ND]+dg[ND], lh[ND]+lg[ND], sh[ND]+sg[ND], rh[ND]+rg[ND],
      (100.0*(wh[ND]+wg[ND])+50.0*(dh[ND]+dg[ND]))/ng);
    printf("   Home   : (%3d) %3d %3d %3d %4d-%3d %6.2f%%\n",
      wh[ND]+dh[ND]+lh[ND], wh[ND], dh[ND], lh[ND], sh[ND], rh[ND], 
      (100.0*wh[ND]+50.0*dh[ND])/(wh[ND]+dh[ND]+lh[ND]));
    printf("   Guest  : (%3d) %3d %3d %3d %4d-%3d %6.2f%%\n",
      wg[ND]+dg[ND]+lg[ND], wg[ND], dg[ND], lg[ND], sg[ND], rg[ND],
      (100.0*wg[ND]+50.0*dg[ND])/(wg[ND]+dg[ND]+lg[ND]));
  }

  for (int d=0; d<ND; d++) {
    ng = wh[d]+dh[d]+lh[d]+wg[d]+dg[d]+lg[d];
    if (ng>0) {
      printf("\n Div %d   : (%3d) %3d %3d %3d %4d-%3d %6.2f%%\n",
        d+1, wh[d]+dh[d]+lh[d]+wg[d]+dg[d]+lg[d], wh[d]+wg[d], dh[d]+dg[d], lh[d]+lg[d], sh[d]+sg[d], rh[d]+rg[d],
        (100.0*(wh[d]+wg[d])+50.0*(dh[d]+dg[d]))/ng);
      printf("  Home   : (%3d) %3d %3d %3d %4d-%3d %6.2f%%\n",
        wh[d]+dh[d]+lh[d], wh[d], dh[d], lh[d], sh[d], rh[d],
        (100.0*wh[d]+50.0*dh[d])/(wh[d]+dh[d]+lh[d]));
      printf("  Guest  : (%3d) %3d %3d %3d %4d-%3d %6.2f%%\n",
        wg[d]+dg[d]+lg[d], wg[d], dg[d], lg[d], sg[d], rg[d],
      (100.0*wg[d]+50.0*dg[d])/(wg[d]+dg[d]+lg[d]));
    }
  }

  printf("%s\n\n", normal);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  int a, b;
  details = 0;

  for (int k=1; k<argc; k++) {
    if (strstr(argv[k], "-d")==argv[k]) details = 1;
    if (strcmp(argv[k], "-fd")==0) {
      if (k+1<argc) fd = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-ld")==0) {
      if (k+1<argc) ld = atoi(argv[k+1]);
    }
  }

  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }

  if (fd < FY[0]) fd = FY[0];
  if (ld < fd || ld > LY[0]) ld = LY[0];
  do {
    a = GetUnique("First team : ");
    printf("  %s\n", NameOf(L, a, 3000));
    b = GetUnique("Second team: ");
    printf("  %s\n", NameOf(L, b, 3000));
    Head2Head(a, b);
  } while (1);
  return 0;
}
