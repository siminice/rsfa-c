#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *normal  = "\033[0m";
const char *yellow  = "\033[33m";
const char *cyan    = "\033[36m";
int verbosity;

char **club;
char **mnem;
int  NC;
int year;

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

//--------------------------------------

int Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  if (f==NULL) return 0;
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
  return 1;
}

   
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

/* *************************************** */

#define MAX_NAMES	20000
#define TD_ID		 0
#define TD_PRENUME	 1
#define TD_NUME		 2
#define TD_DOB		 3
#define TD_NAT		 4
#define TD_CLUB		 5
#define TD_NR		 6
#define TD_POST		 7
#define TD_MECIURI	 8
#define TD_TITULAR	 9
#define TD_REZERVA	10
#define TD_GOLURI	11
#define TD_MINUTE	12
#define TD_GALBENE	13
#define TD_ROSII	14
#define TD_NUM		TD_GOLURI+1

int NP;
char **pmnem;
char **pname;
char **ppren;
char **pdob;
char **pcty;

int ntd;
char ***td;
char ofilename[128];
FILE *of;

int FindMnem(char *mnem) {
  for (int i=0; i<NP; i++)
    if (strcmp(pmnem[i], mnem)==0) return i;
  return -1;
}

int FindName(char *name, char *pren, char *dob, char *cty) {
  for (int i=0; i<NP; i++) {
    if (strstr(pname[i], name)!=NULL) {
      if (verbosity > 1) {
        printf(">...\033[36m%s,%s\033[0m [%s]\n", pname[i], ppren[i], pdob[i]);
      }
    }
  }
  for (int i=0; i<NP; i++) {
    if ((strstr(pname[i], name)!=NULL) && (pren==NULL || strcmp(ppren[i],pren)==0)) {
        if (dob==NULL || strcmp(pdob[i], dob)!=0) {
        }
        return i;
    }
  }
  return -1;
}

char low(char c) {
  if (c=='ª') return 'º';
  if (c=='Þ') return 'þ';
  if (c=='Î') return 'î';
  if (c>='A' && c<='Z') return c+32;
  return c;
}

void LoadPlayers() {
  NP = 0;
  FILE *f = fopen("players.dat", "rt");
  if (f==NULL) {
    printf("ERROR: player database not found!\n");
    exit(0);
  }
  char s[100], *t[10];
  fgets(s, 100, f);
  sscanf(s, "%d", &NP);
  pname = new char*[MAX_NAMES];
  pmnem = new char*[MAX_NAMES];
  ppren = new char*[MAX_NAMES];
  pdob  = new char*[MAX_NAMES];
  pcty  = new char*[MAX_NAMES];
  for (int i=0; i<NP; i++) {
    fgets(s, 100, f);
//    printf("... parsing %s", s);
    t[0] = strtok(s, ",\t\n");
     pmnem[i] = new char[7];
     strcpy(pmnem[i], t[0]);
    t[1] = strtok(NULL, ",\t\n");
     pname[i] = new char[strlen(t[1])+1];
     strcpy(pname[i], t[1]);
    t[2] = strtok(NULL, ",\t\n");
     ppren[i] = new char[strlen(t[2])+1];
     strcpy(ppren[i], t[2]);
    t[3] = strtok(NULL, ",\t\n");
     pdob[i] = new char[12];
     strcpy(pdob[i], t[3]);
    t[4] = strtok(NULL, ",\t\n");
     pcty[i] = new char[4];
     strcpy(pcty[i], t[4]);
     if (verbosity > 1)
       printf("Loaded %s: %s %s [%s] (%s)\n", pmnem[i], ppren[i], pname[i], pdob[i], pcty[i]);
  }
  fclose(f);
}

void SavePlayers() {
  FILE *f = fopen("players.new", "wt");
  fprintf(f, "%d\n", NP);
  for (int i=0; i<NP; i++) {
    fprintf(f, "%s,%s,%s,%s,%s\n", pmnem[i], pname[i], ppren[i], pdob[i], pcty[i]);
  }
  fclose(f);
  rename("players.dat", "players.old");
  rename("players.new", "players.dat");
}


void Extract(char *filename) {
  char s[1000], w[1000], *u, *t[10], *tk[100];
  char r;
  int tm;
  int da, db, cont;
  int pa, pb, sa[5], sb[5];

  FILE *f = fopen(filename, "rt");
  if (f==NULL) {
    printf("ERROR: Cannot open file %s.\n", filename);
    return;
  }
  
  strtok(filename, ".");
  char *sy = strtok(NULL, ".\n");
  if (sy!=NULL) year = atoi(sy);

  ntd = 0;
  td = new char**[MAX_NAMES];
  for (int j=0; j<MAX_NAMES; j++) td[j] = new char*[TD_NUM];

  do {
    fgets(s, 1000, f);
    if (s[0]=='#') {
      fgets(s, 1000, f);
      t[0]= strtok(s, "\t");
      tm = atoi(t[0]);
      fgets(s, 1000, f);
      fgets(s, 1000, f);
      fgets(s, 1000, f);
    }
    if (s==NULL || s[0]==0) continue;
    tk[0] = strtok(s, "\t\n");
    for (int i=1; i<100; i++) tk[i] = strtok(NULL, "\t");
    fgets(w, 1000, f);
    if (w==NULL || s[0]==0) continue;
    u = w;
    t[4] = strtok(u, " \t\n");
    if (t[4]!=NULL && ((t[4][0]>='0' && t[4][0]<='9') || (strlen(t[4])!=3))) {
      strcpy(t[4], "ROM");
    }
   
    pa = FindName(tk[2], tk[1], tk[3], t[4]);
    if (pa>=0) {
      if (verbosity > 1) {
        printf("Found: %s %s [%s] (%s)\n", ppren[pa], pname[pa], pdob[pa], pcty[pa]);
      }
    }
    else if (pa<0) {
      printf("Name %s %s [%s] (%s) not found. Add? [1]=yes ", tk[1], tk[2], tk[3], t[4]);
      int add = 1;
      scanf("%d", &add);
      if (add==1) {
        pname[NP] = new char[strlen(tk[2])+1];
        strcpy(pname[NP], tk[2]);
        ppren[NP] = new char[strlen(tk[1])+1];
        strcpy(ppren[NP], tk[1]);
        pdob[NP] = new char[12];
        if (tk[3]!=NULL) strcpy(pdob[NP], tk[3]); else strcpy(pdob[NP], "00/00/0000");
        pcty[NP] = new char[4];
        if (t[4]!=NULL) strcpy(pcty[NP], t[4]); else strcpy(pcty[NP], "ROM");

        pmnem[NP] = new char[7];
        pmnem[NP][0] = low(pname[NP][0]);
        int d = 0; int k=1;
        while (k<4) {
          if (k>=strlen(tk[2])) pmnem[NP][k]='x'; 
          else if (pname[NP][k+d]==' ') {d++; k--;}
          else pmnem[NP][k] = pname[NP][k+d];
          k++;
        }
        if (tk[1]!=NULL) pmnem[NP][4] = low(ppren[NP][0]); else pmnem[NP][4] = '_';
        if (tk[1]!=NULL) {
          if (strlen(tk[1])<2) pmnem[NP][5] = '_';
          else pmnem[NP][5] = low(ppren[NP][1]);
        }
        pmnem[NP][6] = 0;
        if (FindMnem(pmnem[NP])>=0) {
          printf("...Mnemonic %s already in use.\n", pmnem[NP]);
          int k=0;
          do {
            k++;
            pmnem[NP][5] = (char) (k+48);
          } while (FindMnem(pmnem[NP])>=0);
        }
        pa = NP;
        printf(".Added player #%d, \033[33;1m%s\033[0m: %s %s [%s] (%s)\n", NP+1, pmnem[NP], ppren[NP], pname[NP], pdob[NP], pcty[NP]);
        NP++;
      }
    }
//    else printf("Identified: %s %s (%s)\n", ppren[pa], pname[pa], pdob[pa]);
    
    int num_t = 0;
    int num_r = 0;
    printf("%s. %s %s (%s) %s %s%s%s: \n", tk[0], tk[1], tk[2], tk[3], tk[4], yellow, NickOf(L, tm, year), normal);
    for (int k=5; (k<39 && tk[k]!=NULL); k++) {
      if (tk[k][0]!=0 && tk[k][0]>='1' && tk[k][0]<='9') { // is numeric 
        int v = atoi(tk[k]);
        if (v>=1 && v<=11) num_t++;
        if (v>=12 && v<=15) num_r++;
      }
    }
    int c_t = 0;
    int c_r = 0;
    int c_m = 0;
    int c_g = 0;
    if (tk[39]) c_t = atoi(tk[39]);
    if (tk[40]) c_r = atoi(tk[40]);
    if (tk[41]) c_m = atoi(tk[41]);
    if (tk[42]) c_g = atoi(tk[42]);
    printf("\tT:%2d + R:%2d = %2d [t:%2d, r:%2d, m:%2d, g:%2d]\n", num_t, num_r, num_t+num_r, c_t, c_r, c_m, c_g);

    td[ntd][TD_ID] = new char[5];
      sprintf(td[ntd][TD_ID], "%d", ntd+1);
    td[ntd][TD_PRENUME] = new char[strlen(tk[1])+1];
      strcpy(td[ntd][TD_PRENUME], tk[1]);
    td[ntd][TD_NUME] = new char[strlen(tk[2])+1];
      strcpy(td[ntd][TD_NUME], tk[2]);
    td[ntd][TD_DOB] = new char[strlen(tk[3])+1];
      strcpy(td[ntd][TD_DOB], tk[3]);
    td[ntd][TD_NAT] = new char[4];
      strcpy(td[ntd][TD_NAT], t[4]);
    td[ntd][TD_CLUB] = new char[20];
      strcpy(td[ntd][TD_CLUB], NickOf(L, tm, year));
    td[ntd][TD_NR] = new char[strlen(tk[0])+1];
      strcpy(td[ntd][TD_NR], tk[0]);
    td[ntd][TD_POST] = new char[3];
      strcpy(td[ntd][TD_POST], tk[4]);
    td[ntd][TD_MECIURI] = new char[3];
      sprintf(td[ntd][TD_MECIURI], "%d", c_m);
    td[ntd][TD_TITULAR] = new char[3];
      sprintf(td[ntd][TD_TITULAR], "%d", c_t);
    td[ntd][TD_REZERVA] = new char[3];
      sprintf(td[ntd][TD_REZERVA], "%d", c_r);
    td[ntd][TD_GOLURI] = new char[3];
      sprintf(td[ntd][TD_GOLURI], "%d", c_g);
    ntd++;

    s[0] = 0;

  } while (!feof(f));
  fclose(f);
}

void HTMLHeader() {  
  fprintf(of, "<HTML>\n<TITLE>Loturi Divizia A %d-%d</TITLE>\n", year-1, year);
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");
}

void HTMLTable() {
  fprintf(of, "<script src=\"sorttable.js\"></script>\n");
  fprintf(of, "<TABLE class=\"sortable\" cellpadding=\"2\">\n");  
  fprintf(of, "<THEAD><TR>\n");
  fprintf(of, "<TH>#</TH>");
  fprintf(of, "<TH>Prenume</TH>");
  fprintf(of, "<TH>Nume</TH>");
  fprintf(of, "<TH>Data naºterii</TH>");
  fprintf(of, "<TH>Naþionalitate</TH>");
  fprintf(of, "<TH>Club</TH>");
  fprintf(of, "<TH>Nr</TH>");
  fprintf(of, "<TH>Post</TH>");
  fprintf(of, "<TH>Meciuri</TH>");
  fprintf(of, "<TH>Titular</TH>");
  fprintf(of, "<TH>Rezervã</TH>");
  fprintf(of, "<TH>Goluri</TH>");
  fprintf(of, "</TR></THEAD>\n"); 

  for (int i=0; i<ntd; i++) {
    fprintf(of, "\n<TR>");
    for (int j=0; j<TD_NUM; j++) {
      fprintf(of, "<TD align=\"");
      if (j==TD_ID || j==TD_PRENUME || j==TD_NUME || j==TD_CLUB) fprintf(of, "left");
      else if (j==TD_NAT || j==TD_POST) fprintf(of, "center");
      else fprintf(of, "right");
      if (j==TD_NAT) {
        fprintf(of, "\">%s<IMG SRC=\"../../thumbs/22/3/%s.png\"></IMG></TD>", td[i][j], td[i][j]);
      }
      else {
        fprintf(of, "\">%s</TD>", td[i][j]);
      }
    }
    fprintf(of, "</TR>");
  }
  fprintf(of, "</TABLE>\n");
}

void HTMLFooter() {
  fprintf(of, "</BODY>\n</HTML>");
  fclose(of);
}

int main(int argc, char **argv) {
  char filename[64];
  strcpy(filename, "");
  verbosity = 2;
  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-q")==0) verbosity = 0;
    if (strcmp(argv[k], "-f")==0 && k<argc-1) strcpy(filename, argv[++k]);
  }
  if (strlen(filename)==0) {
    printf("ERROR: input file not specified.\n");
    return -1;
  }
  Load();
  LoadPlayers();
  Extract(filename);
  SavePlayers();

  sprintf(ofilename, "catalog-%d.html", year);
  of = fopen(ofilename, "wt");
  if (of==NULL) {
    return 1;
  }

  HTMLHeader();
  HTMLTable();
  HTMLFooter();
  fclose(of);
  return 0;
}
