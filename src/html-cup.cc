#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define NUMORD      10
#define MAX_LEVELS 12
#define MAX_RR      4
#define MAX_N      64
#define MAX_CP	 50000

int  NC, ND, NM;
char **club;
char **mnem;
int *FY, *LY, *MAX;
int fd, ld;
int  ****part;
char **cup;
int num_winter;
int *start_winter, *end_winter;
char lgname[128], cpname[128];

char inputfile[64], outputfile[64], filename[64];
FILE *of;

const char* fxcol[] = {"E4FEE4", "FFFFCC", "FFE4E4", "F0F0FF"};
const char* cupround[] = {"", "Finala", "Semifinale", "Sferturi", "Optimi", "ªaisprezecimi", "1/64", "1/128", "1/256", "9"};
const char* cuprnd[] = {"C", "F", "S", "Q", "1/8", "1/16", "1/32", "1/64", "1/128", "1/256"};
const int rlo[] = {100, 225, 255, 0, 0, 0, 0, 0} ;
const int rhi[] = {144, 255, 240, 0, 0, 0, 0, 0} ;
const int glo[] = {255, 255, 144, 0, 0, 0, 0, 0} ;
const int ghi[] = {255, 255, 100, 0, 0, 0, 0, 0} ;
const int blo[] = { 54,  54, 100, 0, 0, 0, 0, 0} ;
const int bhi[] = {144, 144, 255, 0, 0, 0, 0, 0} ;

int m_year, m_month, m_day, m_div, m_pool, m_round, m_home, m_guest, m_gdiff, m_qual;
int m_score[8];
int ntm, tid1, tid2, tmid[48];
int ngm[48][48];
int nreplay;
int last_r;

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

  strcpy(lgname, "Campionat");
  strcpy(cpname, "Cupa");
  f = fopen("liga.dat", "rt");
  if (f!=NULL) {
    fgets(s, 128, f);
    s[strlen(s)-1] = 0;
    strncpy(lgname, s, 100);
    fgets(s, 128, f);
    s[strlen(s)-1] = 0;
    strncpy(cpname, s, 100);
    fclose(f);
  }

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
  FILE *f = fopen("cuparchive.dat", "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: cannot find file 'cuparchive.dat'\n");
    return 0;
  }
  cup = new char*[MAX_CP];
  int m = 0;
  while (!feof(f)) {
    fgets(s, 128, f);
    if (!feof(f)) {
      cup[m] = new char[24];
      strncpy(cup[m], s, 24); 
      m++;
    }
    s[0] = 0;
  }
  fclose(f);
  NM = m;
  return 1;
}

int in(int y, int d, int p, int t) {
  if (MAX[d]==0) return 0;
  if (y<FY[d] || y>LY[d]) return 0;
  int m = part[d][y-FY[d]][p][1];
  for (int j=2; j<m+2; j++)
    if (part[d][y-FY[d]][p][j] == t) return j-1;
  return -1;
}

int dvn(int y, int t) {
  for (int d=0; d<3; d++) {
    for (int p=0; p<=MAX[d]; p++) {
      if (in(y, d, p, t) > 0) return d;
    }
  }
  return 3;
}

void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);
  else sprintf(ss, "%d/%02d", y-1, y%100);
}

int ResetCupData() {
	ntm = 0;
	for (int i=0; i<32; ++i)
		for (int j=0; j<32; ++j) ngm[i][j] = 0;
}

int FindId(int t) {
	int i;
	for (i=0; i<ntm; ++i) if (tmid[i]==t) return i;
	if (i==ntm && ntm<32) {
		tmid[ntm++] = t;
		return ntm-1;
	}
	return -1;
}

void Decode(char *s) {
  int i;
  m_year     = 75*(s[0]-48) + (s[1]-48) + 1870;
  m_month    = s[2] - 48;
//  m_year    += (m_month/13) - 1;
  m_day      = s[3] - 48;
  if (m_day<0) m_day = 0;
  m_div      = s[4];
  m_pool     = s[5] - 48;
  m_round    = (int) (s[6] - 48);
  m_home     = 75*((int)(s[7]-48)) + s[8] - 48;
  m_guest    = 75*((int)(s[9]-48)) + s[10] - 48;
  int len  = strlen((char *)s);
  for (i=0; i<8; i++)
    if (i+11 < len) m_score[i] = s[i+11] - 48;
    else m_score[i] = -1;
  m_gdiff    = m_score[0] - m_score[1];
  m_qual = 0;
  if (m_round == 2 && (
      m_year==1969 || m_year==1971 || 
     (m_year>=1991 && m_year<=1995) ||
     (m_year>=1999 && m_year<=2006) || 
     (m_year>=2010)
    ) 
  )
  { return; }
  if (m_round == 3 && (
      m_year==1971 || m_year==1999 ||
     (m_year>=1991 && m_year<=1993) ||
     (m_year>=2003 && m_year<=2005)  
    ) 
  )
  { return; }
  
  if (m_gdiff>0) m_qual = 1;
  else if (m_gdiff<0) m_qual = 2;
  else if (m_score[6]>=0 && m_score[7]>=0) {
    if (m_score[6]>m_score[7]) m_qual = 1;
    if (m_score[6]<m_score[7]) m_qual = 2;
  }
  else if (m_year > 1962 && m_round > 4) m_qual = 2;
  else if (m_year > 1962 && m_year < 1974 && m_round<5 && m_round >1) {
    if (dvn(m_year, m_home) > dvn(m_year, m_guest)) m_qual = 1;
    if (dvn(m_year, m_home) < dvn(m_year, m_guest)) m_qual = 2;
  }
}

void TableHeader(int r) {
  fprintf(of, "<TABLE WIDTH=\"75%%\" BORDER=\"1\" RULES=\"rows\" FRAME=\"box\" BGCOLOR=\"DDDDDD\">\n");
  fprintf(of, "<THEAD>\n<TR>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"12%%\"></TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"32%%\"></TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"1%%\"></TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"32%%\"></TH>\n");
  fprintf(of, "<TH ALIGN=\"left\"  WIDTH=\"23%%\"></TH>\n");
  fprintf(of, "</TR>\n</THEAD>\n<TBODY>\n");
  fprintf(of, "<TR><TD COLSPAN=\"5\" BGCOLOR=\"FFFFFF\">%s</TD></TR>\n", cupround[r]);
}

void TableFooter() {
  fprintf(of, "</TBODY>\n</TABLE>\n");
}

void Cup(int y) {
  sprintf(outputfile, "html/cupa-%d.html", y);
  of = fopen(outputfile, "wt");
  if (!of) return;
  
  char strsez[32];
  SeasonName(y, strsez);
  fprintf(of, "<HTML>\n<TITLE>%s %s</TITLE>\n", cpname, strsez);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY BGCOLOR=\"333333\">\n");
	fprintf(of, "<H2>");
  fprintf(of, "<A HREF=\"cupa-%d.html\"><IMG HEIGHT=\"32\" SRC=\"prev.gif\"></IMG></A>", y-1);
	fprintf(of, "<FONT COLOR=\"EEEEEE\"> %s %s </FONT>", cpname, strsez);
  fprintf(of, "<A HREF=\"cupa-%d.html\"><IMG HEIGHT=\"32\" SRC=\"next.gif\"></IMG></A>", y+1);
	fprintf(of, "</H2>\n");

  last_r = -1;
  for (int m=0; m<NM; m++) {
    Decode(cup[m]);
    if (m_year == y) {
			tid1 	= FindId(m_home);
			tid2	= FindId(m_guest);
			if (tid1>=0 && tid2>=0 && tid1<32 && tid2<32)
				nreplay = ++ngm[tid1][tid2];
      if (m_round!=last_r) {
        if (last_r > 0) TableFooter();
        TableHeader(m_round);
      }
      fprintf(of, "<TR>");
      int d = m_month/13;
      fprintf(of, "<TD ALIGN=\"right\">%d-%02d-%d</TD>", m_day, (m_month-1)%12+1, m_year-1+d);
      fprintf(of, "<TD ALIGN=\"right\" BGCOLOR=\"%s\"><A HREF=\"istoric-%d.html\"><FONT COLOR=\"000000\">%s%s%s</FONT></A></TD>", 
				fxcol[dvn(y, m_home)], m_home,
        (m_qual==1? "<B>":""), NameOf(L, m_home, y), (m_qual==1? "</B>":""));
      fprintf(of, "<TD>-</TD>");
      fprintf(of, "<TD ALIGN=\"left\" BGCOLOR=\"%s\"><A HREF=\"istoric-%d.html\"><FONT COLOR=\"000000\">%s%s%s</FONT></A></TD>", 
				fxcol[dvn(y, m_guest)], m_guest,
        (m_qual==2? "<B>":""), NameOf(L, m_guest, y), (m_qual==2? "</B>":""));
			char srepl[12];
			for (int ri=2; ri<=nreplay; ri++) srepl[ri-2]='r';
			srepl[nreplay-1] = 0;
      fprintf(of, "<TD ALIGN=\"center\"><A HREF=\"reports/%d/c%d-%d%s.html\">", m_year, m_home, m_guest, srepl);
      if (m_score[0]>=0 && m_score[1]>=0) fprintf(of, "%d-%d", m_score[0], m_score[1]);
      if ((m_score[0]+m_score[1]>0) && m_score[2]>=0 && m_score[3]>=0) {
        fprintf(of, " (%d-%d", m_score[2], m_score[3]);
        if (m_score[4]>=0 && m_score[5]>=0) {
          fprintf(of, ",%d-%d", m_score[4], m_score[5]);
        }
        fprintf(of, ")");
      }
      if (m_score[6]>=0 && m_score[7]>=0) {
        fprintf(of, ", pen: %d-%d", m_score[6], m_score[7]);
      }

/*
      if (m_score[4]<0 && m_score[5] < 0)
        fprintf(of, "%d-%d", m_score[0], m_score[1]);
      if (m_score[4]>=0 && m_score[5]>=0 && m_score[6]<0) 
        fprintf(of, "%d-%d, prel", m_score[0], m_score[1]);
      if (m_score[6]>=0 && m_score[7]>=0)
        fprintf(of, "%d-%d, pen: %d-%d", m_score[0], m_score[1], m_score[6], m_score[7]);
*/
      fprintf(of, "</A></TD>");
      fprintf(of, "</TR>");
      last_r = m_round;
    }
  }
  if (last_r>0) TableFooter();

  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);

}

//---------------------------------------------

int main(int argc, char* argv[]) {

  for (int k=1; k<argc; k++) {
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
  CupData();

  if (fd < FY[0]) fd = FY[0];
  if (ld < fd || ld > LY[0]) ld = LY[0];

  for (int y=fd; y<=ld; y++) {
		ResetCupData();
    Cup(y);
  }

  return 0;
}
