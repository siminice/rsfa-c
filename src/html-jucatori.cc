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
char inputfile[64], outputfile[64], filename[64];
char liga[64], cupa[64];
FILE *of;
int details;
int num_winter;
int *start_winter, *end_winter;

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

  f = fopen("liga.dat", "rt");
  if (f!=NULL) {
    fgets(s, 200, f);
    s[strlen(s)-1] = 0;
    strcpy(liga, s);
    fgets(s, 200, f);
    s[strlen(s)-1] = 0;
    strcpy(cupa, s);
    fclose(f);
  }

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
          if (d<0 || d>=1) continue;
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

void SeasonName(int y, char *ss) {
  int winter = 0;
  for (int  i=0; i<num_winter; i++) {
    if (y>=start_winter[i] && y<=end_winter[i]) winter = 1;
  }
  if (winter) sprintf(ss, "%d", y);
  else sprintf(ss, "%d/%02d", y-1, y%100);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  int n;
  char ss[32];
  int nw[2000];

  for (int k=1; k<argc; k++) {
    if (strcmp(argv[k], "-fy")==0) {
      if (k+1<argc) fd = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-ly")==0) {
      if (k+1<argc) ld = atoi(argv[k+1]);
    }
  }

  if (!Load()) {
    printf("ERROR: called from invalid directory.\n");
    return -1;
  }

  if (fd < FY[0]) fd = FY[0];
  if (ld < fd || ld > LY[0]) ld = LY[0];

  FILE *f = fopen("html/jucatori.html", "wt");
  if (f==NULL) { 
    fprintf(stderr, "ERROR: cannot open output file.\n");
    return 1;
  }
  fprintf(f, "<html>\n");
  fprintf(f, "<HEAD><title>Jucãtori în prima divizie</title>\n", liga);
  fprintf(f, "<link href=\"css/seasons.css\" rel=\"stylesheet\" type=\"text/css\"/>\n");
  fprintf(f, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-2>\"\n");
  fprintf(f, "</head>\n");
  fprintf(f, "<center>\n");
  fprintf(f, "<H2>Jucãtori în prima divizie</H2>\n");
  fprintf(f, "<A HREF=\"catalog.html\"><I><B>All-time</B></I></A><BR><BR>");
  fprintf(f, "<TABLE BORDER=\"1\" CELLPADDING=\"2\" CELLSPACING=\"3\" FRAME=\"box\">\n<TR>");
  for (int y=1931; y<=2013; y++) {

    int played = 0;
    for (int p=0; p<=MAX[0]; p++)
      if (part[0][y-FY[0]][p][1]>0) played = 1;
    int interval = 0;
    if (y>=fd && y<=ld) interval = 1;
    SeasonName(y, ss);
    if (y<1933 || y==1942 || y==1943 || y==1944 || y==1957 || !played) fprintf(f, "<TD></TD>");
    else if (!interval) fprintf(f, "<TD ALIGN=\"right\">%s</TD>", ss);
    else fprintf(f, "<TD ALIGN=\"right\"> <A HREF=\"catalog-%d.html\">%s</A> </TD>", y, ss);
    if (y%10==0) fprintf(f, "</TR>\n");
  }
  fprintf(f, "</TR>");
  fprintf(f, "</TABLE>\n</CENTER><HR>\n");
  fprintf(f, "<BR>NB: Tabele au toate coloanele sortabile, cu <I>click</I> pe numele coloanei:\n<UL>\n");
  fprintf(f, "<LI>Primul <I>click</I>: ordine crescãtoare</LI>\n");
  fprintf(f, "<LI>Al doilea <I>click</I>: ordine descrescãtoare</LI>\n");
  fprintf(f, "<LI>Al treilea <I>click</I>: reset</LI>\n");
  fprintf(f, "</UL>\n");
  fprintf(f, "</html>");
  fclose(f);  
 
  return 0;
}
