#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_TEAMS 2000
#define MAX_ROUNDS   7

int  NC;
char **club;
char **mnem;

int  win[MAX_TEAMS], drw[MAX_TEAMS], los[MAX_TEAMS], gsc[MAX_TEAMS], gre[MAX_TEAMS];
int  rank[MAX_TEAMS];
int  cupp[MAX_TEAMS][MAX_ROUNDS];

char filename[64];
FILE *of;

const char* cupround[] = {"C", "F", "S", "Q", "8", "16", "32", "64", "128", "256"};

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
  char s[2000], *tok[20];
  char *ystr, *name, *nick;

  for (int t=0; t<MAX_TEAMS; t++) {
    win[t] = drw[t] = los[t] = gsc[t] = gre[t] = 0;
    for (int r=0; r<MAX_ROUNDS; r++) cupp[t][r] = 0;
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
  return 1;
}

int CupData() {
  char s[128];
  int score[8];
  int py = -1;
  int pld[MAX_TEAMS];
  for (int t=0; t<MAX_TEAMS; t++) pld[t] = -1;

  FILE *f = fopen("cuparchive.dat", "rt");
  if (f==NULL) {
    fprintf(stderr, "ERROR: cannot find file 'cuparchive.dat'\n");
    return 0;
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
      
			if (score[0]>=0 && score[1]>=0) {
	      gsc[home]  += score[0];
 	     gre[home]  += score[1];
 	     gsc[guest] += score[1];
 	     gre[guest] += score[0];
 	     if (gdiff>0) {
 	       win[home]++; los[guest]++;
 	     }
 	     else if (gdiff==0) {
 	       drw[home]++; drw[guest]++;
 	     }
 	     else {
 	       los[home]++; win[guest]++;
 	     }
			}

      if (year!=py) {
        for (int t=0; t<MAX_TEAMS; t++) {
          if (pld[t]>=0 && pld[t]<MAX_ROUNDS) {
            cupp[t][pld[t]]++;
          }
        }
        for (int t=0; t<MAX_TEAMS; t++) pld[t] = -1;
      }

      pld[home]  = round;
      pld[guest] = round;

      if (round==1) {
        if (gdiff>0) pld[home] = 0;
        else if (gdiff<0) pld[guest] = 0;
        else {
          if (score[6]>score[7]) pld[home] = 0;
          if (score[6]<score[7]) pld[guest] = 0;
        }
      }
      py = year;
    }
    s[0] = 0;
  }

  for (int t=0; t<MAX_TEAMS; t++) {
    if (pld[t]>=0 && pld[t]<MAX_ROUNDS) {
      cupp[t][pld[t]]++;
    }
  }

  fclose(f);
  return 1;
}

int sup(int i, int j) {
  int wi = win[i];
  int wj = win[j];
  int di = drw[i];
  int dj = drw[j];
  int li = los[i];
  int lj = los[j];
  if (wi+di+li==0) return 0;
  if (wj+dj+lj==0) return 1;
  int pi = 2*wi + drw[i];
  int pj = 2*wj + drw[j];
  if (pi > pj) return 1;
  if (pi < pj) return 0;
  int gsi = gsc[i];
  int gsj = gsc[j];
  int gdi = gsi - gre[i];
  int gdj = gsj - gre[j];
  if (gdi > gdj) return 1;
  if (gdi < gdj) return 0;
  if (wi > wj) return 1;
  if (wi < wj) return 0;
  if (gsi > gsj) return 1;
  if (gsi < gsj) return 0;
  return 0;
}

void Ranking() {
  for (int i=0; i<NC; i++) rank[i] = i;
  int i, aux, sorted;
  int last = NC-1;
  do {
      sorted = 1;
      for (i=0; i<=last-1; i++) {
         if (sup(rank[i+1], rank[i])) {
           sorted = 0;
           aux = rank[i];
           rank[i] = rank[i+1];
           rank[i+1] = aux;
         }
      }
      last--;
  } while (!sorted);
}

void CupAlltime() {
  char s[200];

  sprintf(filename, "html/alltime-cup.html");
  of = fopen(filename, "wt");
  if (!of) return;

  Ranking();

  fprintf(of, "<HTML>\n<TITLE>Clasament <I>all-time</I> Cupa României</TITLE>\n");
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<H2>Clasament <I>all-time</I> Cupa României</H2>\n");
  fprintf(of, "<TABLE BORDER=\"1\" WIDTH=\"90%%\" CELLPADDING=\"2\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">");
  fprintf(of, "<TH WIDTH=\"2%%\" ALIGN=\"right\">#</TH>");
  fprintf(of, "<TH WIDTH=\"30%%\" ALIGN=\"left\">Echipa</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">J</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">V</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">E</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">Î</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">gm</TH>");
  fprintf(of, "<TH WIDTH=\"1%%\" ALIGN=\"center\">-</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">gp</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">P</TH>");
  fprintf(of, "<TH WIDTH=\"5%%\" ALIGN=\"right\">%%</TH>");
  fprintf(of, "<TH WIDTH=\"3%%\">  </TH>");
  for (int r=0; r<=5; r++) {
    fprintf(of, "<TH WIDTH=\"3%%\" ALIGN=\"center\">%s</TH>", cupround[r]);
  }
  fprintf(of, "</TR>\n");

  for (int i=0; i<NC; i++) {
    int k = rank[i];
    int ng = win[k]+drw[k]+los[k];
    if (ng>0) {
      fprintf(of, "<TR BGCOLOR=\"%s\">", (i%2==1?"FFFFFF":"DDDDFF"));
      fprintf(of, "<TD ALIGN=\"right\">%d.</TD>", i+1);
      fprintf(of, "<TD ALIGN=\"left\"><A HREF=\"istoric-%d.html\">%s</A></TD>", k, NameOf(L, k, 3000));
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", ng);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", win[k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", drw[k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", los[k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gsc[k]);
      fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", gre[k]);
      fprintf(of, "<TD ALIGN=\"right\">%d</TD>", 2*win[k]+drw[k]);
      fprintf(of, "<TD ALIGN=\"right\">[%3d]</TD>", (int) ((100.0*win[k]+50*drw[k]) / (ng)));
      fprintf(of, "<TD></TD>");
      for (int r=0; r<=5; r++) {
        if (cupp[k][r] > 0) 
          fprintf(of, "<TD ALIGN=\"center\">%d</TD>", cupp[k][r]);
        else 
          fprintf(of, "<TD ALIGN=\"center\">-</TD>");
      }
      fprintf(of, "</TR>\n");
    }
  }

  fprintf(of, "</TABLE>\n");
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}


//---------------------------------------------

int main(int argc, char* argv[]) {

  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }

  CupData();
  Ranking();
  CupAlltime();

  return 0;
}
