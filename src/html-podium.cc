#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_TEAMS 2000
#define MAX_MED 20

int MED;
int FYA, LYA;

int  NC, NE;
char **club;
char **mnem;
int  **parta;
//int  **med;
int  *rank;
char *filename;
int count_last;
int num_winter;
int *start_winter, *end_winter;
int med[MAX_TEAMS][MAX_MED];
FILE *of;

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
      k++;
    }
  }
  fclose(f);

  FYA = 2100; LYA = 1800;
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
          if (l==0 || l>3) d=-1;
          d = ((int) s[0]) - 96; if (d<1 || d>3) d=-1;
          if (l>1) p = atoi(dv+1); else p = 1;
          y = atoi(yr);
          if (d==1 && y>1888 && y<2100) {
            if (y<FYA) FYA = y;
            if (y>LYA) LYA = y;
          }
        }
      }
      closedir(dp);  
  }
  else
   printf("ERROR: Couldn't open the directory.\n");  

  parta = new int*[1000];
  for (int i=0; i<1000; i++) {
    parta[i] = new int[34];
  }

// quick data
  f = fopen("part.a", "rt");
  y = 0;
  while (!feof(f)) {
//  for (int y=0; y<=2*((LYA+count_last-1)-FYA); y++) {
      fscanf(f, "%d %d", &dummy, &n);
      if (feof(f)) continue;
      parta[y][0] = dummy;
      parta[y][1] = n;
      for (int i=0; i<n; i++) {
        fscanf(f, "%d", &t); parta[y][i+2] = t;
      }
      fgets(s, 200, f);
      y++;
  }
  if (f) fclose(f);
  y--;
  NE = y;
  while (NE>0 && parta[NE-1] == parta[y]) NE--;
}


void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);   
  else sprintf(ss, "%d/%02d", y-1, y%100);
}


int sup(int x, int y) {
  for (int i=0; i<MED; i++) {
    if (med[x][i] > med[y][i]) return 1;
    if (med[x][i] < med[y][i]) return 0;
  }
  return x < y;
}

void Ranking() {
  rank = new int[NC];
  for (int i=0; i<NC; i++) rank[i] = i;
  int sorted = 1;
  do {
    sorted = 1;
    for (int i=0; i<NC-1; i++) 
      if (sup(rank[i+1], rank[i])) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
  } while (!sorted);
}

void Listing() {

  fprintf(of, "<TABLE BORDER=\"1\" CELLPADDING=\"2\" RULES=\"rows\" FRAME=\"box\">\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">");
  fprintf(of, "<TH ALIGN=\"right\">#</TH>");
  fprintf(of, "<TH ALIGN=\"left\">Echipa</TH>");
  for (int k=1; k<=MED; k++)
    fprintf(of, "<TH WIDTH=\"30\" ALIGN=\"right\">%d</TH>", k);
  fprintf(of, "</TR>\n");


  for (int i=0; i<NC; i++) {
    int x = rank[i];
    int sum = 0;
    for (int j=0; j<MED; j++) sum += med[x][j];
    if (sum) {
      fprintf(of, "<TR BGCOLOR=\"%s\">", (i%2==1?"FFFFFF":"DDDDFF"));
      fprintf(of, "<TD>%d.</TD>", i+1);
      fprintf(of, "<TD><A HREF=\"istoric-%d.html\">%s</A></TD>", x, NameOf(L, x, 3000));
      for (int j=0; j<MED; j++) 
        fprintf(of, "<TD ALIGN=\"right\">%d</TD>",  med[x][j]);
      fprintf(of, "</TR>");
    }
  }

  fprintf(of, "</TABLE>\n</BODY>\n</HTML>\n");
}

int Medals() {
  int num;
  for (int t=0; t<NC; t++) {
    for (int j=0; j<MED; j++) med[t][j] = 0;
  }

  char outputfile[256];
  sprintf(outputfile, "html/podium.html");
  of = fopen(outputfile, "wt");
  if (!of) return 0;

  fprintf(of, "<HTML>\n<TITLE>Podium</TITLE>\n");
  fprintf(of, "<HEAD>\n<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(of, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2\">\n");
  fprintf(of, "</HEAD>\n<BODY>\n");

  fprintf(of, "<H2>Prezenþe pe podium</H2>\n");

  fprintf(of, "<TABLE BORDER=\"1\" CELLPADDING=\"3\" RULES=\"cols\" FRAME=\"box\">\n");
  fprintf(of, "<TR BGCOLOR=\"DDDDDD\">");
  fprintf(of, "<TH WIDTH=\"100\" ALIGN=\"left\">Ediþia</TH>");
  for (int k=1; k<=MED; k++)
    fprintf(of, "<TH ALIGN=\"center\">%d</TH>", k);
  fprintf(of, "</TR>\n");

  char ssn[64];
  for (int y=0; y<=NE; y++) {
    int ss = parta[y][0];
    SeasonName(ss, ssn);
    num = parta[y][1];

    if (num>0) {
      fprintf(of, "<TR BGCOLOR=\"%s\">", (y%2==1?"FFFFFF":"DDDDFF"));

      fprintf(of, "<TD><A HREF=\"a.%d-r%d.html\">%s </A></TD>", ss, 2*(num-1), ssn);

      for (int j=0; j<(MED>num?num:MED); j++)  {
        fprintf(of, "<TD>%s</TD>", NickOf(L, parta[y][j+2], ss));
        med[parta[y][j+2]][j]++;
      }
      for (int j=num; j<MED; j++) 
        fprintf(of, "<TD></TD>");
      fprintf(of, "</TR>\n");
    }
  }
  fprintf(of, "</TABLE><BR><HR>\n");

  Ranking();
  Listing();
  fclose(of);

  return 0;
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  int a, b;
  count_last = 1;
  MED = 6;
  for (int i=1; i<argc; i++) {
    if (strcmp(argv[i], "-")==0) count_last = 0;
    if (strcmp(argv[i], "-n")==0) {
      if (i+1<argc) MED = atoi(argv[i+1]);
      if (MED<1 || MED>MAX_MED) MED = 6;
    }
  }
  Load();
  Medals();
  return 0;
}
