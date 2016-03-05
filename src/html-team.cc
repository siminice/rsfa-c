#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define NUMORD      10
#define MAX_LEVELS 12
#define MAX_RR      4
#define MAX_N      64

#define SPECIAL         50
#define LOSS_BOTH_0     50
#define LOSS_BOTH_9     59

int  NC, ND;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;
int  ****part;
int **cupp;
int num_winter;
int *start_winter, *end_winter;
int numlg, numcp;

int  id[MAX_N], win[MAX_N], drw[MAX_N], los[MAX_N], gsc[MAX_N], gre[MAX_N], pts[MAX_N], pen[MAX_N], pdt[MAX_N];
int  rank[MAX_N];

char inputfile[64], outputfile[64], filename[64];
FILE *of;

//const char* fxcol[] = {"E4FEE4", "FFFFCC", "FFE4E4", "F0F0FF"};
//const char* fxcol[] = {"77FF77", "FFFF99", "FF5050", "9999FF", "FFCC99", "FF99FF", "CC33FF", "CCCCCC"};
const char* fxcol[] = {"E4FEE4", "FFFFCC", "FFE4E4", "F0F0FF", "9999FF", "FFCC99", "FF99FF", "CC33FF", "CCCCCC"};
const char* cupround[] = {"*", "F", "S", "Q", "1/8", "1/16", "1/32", "1/64", "1/128", "1/256"};
const int rlo[] = {100, 225, 255, 0, 0, 0, 0, 0} ;
const int rhi[] = {144, 255, 240, 0, 0, 0, 0, 0} ;
const int glo[] = {255, 255, 144, 0, 0, 0, 0, 0} ;
const int ghi[] = {255, 255, 100, 0, 0, 0, 0, 0} ;
const int blo[] = { 54,  54, 100, 0, 0, 0, 0, 0} ;
const int bhi[] = {144, 144, 255, 0, 0, 0, 0, 0} ;

int n, ppv, tbr;
int d1, rka, start, last;

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

int CupData() {
  char s[128];
  int score[8];
  FILE *f = fopen("cuparchive.dat", "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: cannot find file 'cuparchive.dat'\n");
    return 0;
  }
  int na = LY[0]-FY[0]+1;
  cupp = new int*[na];
  for (int y=0; y<na; y++) {
    cupp[y] = new int[NC];
    for (int t=0; t<NC; t++) cupp[y][t] = -1;
  }
  while (!feof(f)) {   
    fgets(s, 128, f);
    if (strlen(s) > 10) {
      int year     = 75*(s[0]-48) + (s[1]-48) + 1870;
      int mon      = s[2] - 48;
      int ssn      = year;
      int round    = (int) (s[6] - 48);
      int home     = 75*((int)(s[7]-48)) + s[8] - 48;
      int guest    = 75*((int)(s[9]-48)) + s[10] - 48;
      int len  = strlen((char *)s);
      for (int i=0; i<8; i++)
        if (i+11 < len) score[i] = s[i+11] - 48;
        else score[i] = -1;
      int gdiff = score[0] - score[1];

      if (ssn>=FY[0] && ssn<=LY[0]) {
        cupp[ssn-FY[0]][home] = round;
        cupp[ssn-FY[0]][guest] = round;
      }
      if (round==1) {
        if (gdiff>0) cupp[ssn-FY[0]][home] = 0;
        else if (gdiff<0) cupp[ssn-FY[0]][guest] = 0;
        else {
          if (score[6]>score[7]) cupp[ssn-FY[0]][home] = 0;
          if (score[6]<score[7]) cupp[ssn-FY[0]][guest] = 0;
        }
      }
    }
    s[0] = 0;
  }
  fclose(f);
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

void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);
  else sprintf(ss, "%d/%02d", y-1, y%100);
}

void StatLine(int a, int y, int d1, int *pl, int rk) {
  int t;
  char filename[15], strdiv[15], strsez[64], ssn[64];

  int d = d1/100;
  int p = d1%100;
  SeasonName(y, ssn);
  int cpr = cupp[y-FY[0]][a];

  int lgwinner = 0;
  if (d==0 && p==0 && rk==1) { lgwinner = 1; numlg++; }
  int cupwinner = 0;
  if (cpr==0) { cupwinner = 1; numcp++; }

  if (part[0][y-FY[0]][0][1]==0 && (y<FY[1] || part[1][y-FY[1]][1][1]==0) && (cpr<0)) return;
  if (rk<0) {
    fprintf(of, "<TR BGCOLOR=\"FFFFFF\">\n");
    fprintf(of, "<TD ALIGN=\"right\">%s</TD>", ssn);
    fprintf(of, "<TD COLSPAN=\"12\"></TD>\n");
        if (cpr>=0) {
           fprintf(of, "<TD ALIGN=\"center\"><A HREF=\"cupa-%d.html\">%s</A>", y, cupround[cpr]);
           if (cupwinner) fprintf(of, " (%d)", numcp);
           fprintf(of, "</TD></TR>");
        }
        else {
           fprintf(of, "<TD></TD></TR>");
        }
    return;
  }

  int tg, tw, td, tl, ts, tr, tp;
  tg = tw = td = tl = ts = tr = tp = 0; 
  for (int j=MAX[d]; j>=0; j--) {
    if (pl[j]) {
      if (j==0) sprintf(filename, "%c.%d", d+97, y);
        else sprintf(filename, "%c%d.%d", d+97, j, y);
      LoadFile(filename);
      t = FindId(a);
      if (t>=0) {
//        int red = rlo[d] + (rhi[d]-rlo[d])*(rk-1)/n;
//        int grn = glo[d] + (ghi[d]-glo[d])*(rk-1)/n;
//        int blu = blo[d] + (bhi[d]-blo[d])*(rk-1)/n;
//        fprintf(of, "<TR BGCOLOR=\"rgb(%d,%d,%d)\">\n", red, grn, blu);
        tg = tg + win[t]+drw[t]+los[t];
        tw = tw + win[t];
        td = td + drw[t];
        tl = tl + los[t];
        ts = ts + gsc[t];
        tr = tr + gre[t];
        tp = tp + 2*win[t]+drw[t];
      }
    }
  }

        fprintf(of, "<TR BGCOLOR=\"%s\">\n", fxcol[d]);
        fprintf(of, "<TD ALIGN=\"right\">%s</TD>", ssn);
        if (p==0) {
          sprintf(strdiv, "%c",   (char) (d+65));
          sprintf(strsez, "%c.%d-r%d.html",   (char) (d+97), y, 2*(n-1+n%2));
        }
        else {
          sprintf(strdiv, "%c%d", (char) (d+65), p);
          sprintf(strsez, "%c%d.%d-r%d.html",   (char) (d+97), p, y, 2*(n-1+n%2));
        }
        fprintf(of, "<TD ALIGN=\"center\"><A HREF=\"%s\">%s</A></TD>", strsez, strdiv);
        fprintf(of, "<TD ALIGN=\"right\">%d.</TD>", rk);
        if (lgwinner) 
          fprintf(of, "<TD ALIGN=\"left\"><B>%s</B> (%d)</TD>", NameOf(L, a, y), numlg);
        else 
          fprintf(of, "<TD ALIGN=\"left\">%s</TD>", NameOf(L, a, y));
      if (tg>0) {
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tg);
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tw);
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", td);
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tl);
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ts);
        fprintf(of, "<TD ALIGN=\"center\">-</TD>");
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tr);
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>", tp);
        fprintf(of, "<TD ALIGN=\"right\">[%d%%]</TD>", (int) ((100.0*tw+50*td) / (tg)));
      }
      else {
        fprintf(of, "<TD COLSPAN=\"9\"></TD>");
      }

  if (cpr>=0) {
    fprintf(of, "<TD ALIGN=\"center\"><A HREF=\"cupa-%d.html\">%s</A>", y, cupround[cpr]);
    if (cupwinner) fprintf(of, " (%d)", numcp);
    fprintf(of, "</TD>");
  }
  else {
    fprintf(of, "<TD></TD>");
  }
  fprintf(of, "</TR>\n");
}


void Evol(int a) {
  int d1, rk1, pl[20], found;
  char s[200];

  sprintf(outputfile, "html/istoric-%d.html", a);
  of = fopen(outputfile, "wt");
  if (!of) return;
  numlg = 0;
  numcp = 0;

  fprintf(of, "<HTML>\n<TITLE>Evoluþie istoricã %s</TITLE>\n", NameOf(L, a, 3000));
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY BGCOLOR=\"777777\">\n");
  fprintf(of, "<H2>Evoluþie istoricã %s</H2>\n", NameOf(L, a, 3000));
  fprintf(of, "<UL><LI><A HREF=\"palmares-%d.html\"><FONT COLOR=\"000000\" SIZE=+1>Palmares</FONT></A></LI>\n", a);
  fprintf(of, "<LI><A HREF=\"consecutive-%d.html\"><FONT COLOR=\"000000\" SIZE=+1>Serii consecutive</FONT></A></LI>\n", a);
  fprintf(of, "<LI><A HREF=\"lot-%d.html\"><FONT COLOR=\"000000\" SIZE=+1>Loturi jucãtori</FONT></A></LI>\n", a);
  fprintf(of, "</UL>\n");

  fprintf(of, "<TABLE  WIDTH=\"85%%\" BORDER=\"1\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<THEAD>\n<TR BGCOLOR=\"EEEEEE\">\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"4%%\">Sezon</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"4%%\">Div</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"2%%\">#</TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"   WIDTH=\"36%%\">Echipa</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"5%%\">J</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"7%%\">V</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"5%%\">E</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"5%%\">Î</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"7%%\">gm</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"1%%\">-</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"2%%\">gp</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"7%%\">P</TH>\n");
  fprintf(of, "<TH ALIGN=\"right\"  WIDTH=\"10%%\">%%</TH>\n");
  fprintf(of, "<TH ALIGN=\"center\" WIDTH=\"5%%\">CR</TH>\n");
  fprintf(of, "</TR>\n</THEAD>\n<TBODY>\n");

  start = 0; last = 3000;
  for (int y = fd; y<=ld; y++) {
//    if (part[0][y-FY[0]][0][1] == 0 && part[0][y-FY[0]][1][1] == 0) continue;

    found = 0;
    for (int d=ND-1; d>=0; d--) {
      rka = -1;
      for (int p = 0; p < 20; p++) pl[p] = 0;
      for (int p = MAX[d]; p>=0; p--) {
        rk1 = in(y, d, p, a);
        if (rk1>0) { 
          rka = rk1; 
          d1 = 100*d+p; 
          pl[p] = 1;
          found = 1;
          start = 1;         
        }
      }
      if (rka>0) {
         if (y>last+1) {
           for (int j=last+1; j<y; j++)
             StatLine(a, j, d1, pl, -1);
         }
         StatLine(a, y, d1, pl, rka);
         last = y;
      }
    }
  }
  fprintf(of, "</TBODY>\n</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);

}

//---------------------------------------------

int main(int argc, char* argv[]) {
  int a, b;

  a = -1;
  for (int k=1; k<argc; k++) {
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

  numlg = 0;
  numcp = 0;
  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }
  CupData();

  if (fd < FY[0]) fd = FY[0];
  if (ld < fd || ld > LY[0]) ld = LY[0];

  if (a<0) {
//    a = GetUnique("Team : ");
    for (int i=0; i<NC; i++)  {
      printf("%3d.%s\n", i+1, NameOf(L, i, 3000));
      Evol(i);
    }
  }
  else {
    Evol(a);
  }
  return 0;
}
