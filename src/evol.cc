#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXG 10

int  NC, ND, fd, ld;
char **club;
char **mnem;
int  ****part;
int *FY, *LY, *MAX;
int  ng, tm[MAXG];

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
  char filename[64];

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

  part = new int***[ND];
  FY = new int[ND];
  LY = new int[ND];
  MAX = new int[ND];

  for (int d=0; d<ND; d++) {
    FY[d] = 3000;
    LY[d] = 1000;
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
          if (d>=ND) continue;
          if (l>1) p = atoi(dv+1); else p = 1;
          if (p<0 && p>12) continue;
          y = atoi(yr);
          if (d>=0 && y>1000 && y<3000) {
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

  if (FY[0]>fd) fd = FY[0];
  if (LY[0]<ld) ld = LY[0];

  for (int d=0; d<ND; d++) {
    if (MAX[d]>0) {
      part[d] = new int**[LY[d]-FY[d]+1];
      for (int i=FY[d]; i<=LY[d]; i++) {
        part[d][i-FY[d]] = new int*[MAX[d]+1];
        for (int j=0; j<=MAX[d]; j++) {
          part[d][i-FY[d]][j] = new int[30];
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
}


int Find(char* s) {
  int found = 0;
  int i = 0;
  int multi = 0;
  int j;

  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
    else i++;
  }
  if (found) return i;
  else return -1;
}

int GetUnique(const char *prompt) {
  char name[30];
  int res;
  do {
   printf("%s", prompt); 
   do { gets(name); } while (!strlen(name));
   res = Find(name);
  } while (res < 0);
  return res;
}

int place(int t, int y, int d, int p) {
  int m = part[d][y-FY[d]][p][1];
  for (int j=2; j<m+2; j++)
    if (part[d][y-FY[d]][p][j] == t) 
       return ((ND-d-1)*100 + 100*(m-j+2)/m);
  return 0;
}

void Head2Head() {
  int pos[MAXG][250];
  int rk[MAXG];
  for (int i=0; i<ng; i++) {
    rk[i] = 0;

    for (int y = fd; y<=ld; y++) {
      rk[i] = place(tm[i],y,0,0);
      for (int k=1; k<ND; k++) {
        if (!rk[i]) {
          for (int p=0; p<=MAX[k]; p++)
            if (y>=FY[k] && (!rk[i])) rk[i]=place(tm[i],y,k,p) ;
        }
      }
      pos[i][y-FY[0]] = rk[i];
    }
  }

  FILE *f = fopen("evol.dat", "wt");
  for (int y = fd; y<=ld; y++) {
    if (part[0][y-FY[0]][0][1] > 0) {
      fprintf(f, "%d", y);
      for (int i=0; i<ng; i++) {
        int w = pos[i][y-FY[0]];
        if (w>=0) 
          fprintf(f, "\t%d", w);
        else
          fprintf(f, "\t-");
      }
      fprintf(f, "\n");
    }
  }
  fclose(f);

  f = fopen("evol.plot", "wt");
  fprintf(f, "set term post eps color \"Helvetica 20\" \n");
  fprintf(f, "set output \"evol.eps\" \n");

  fprintf(f, "set key below spacing 0.95 box\n");
  fprintf(f, "set grid\n");
  fprintf(f, "set yrange [0:%d]\n", ND*100);
  for (int d=1; d<ND; d++) {
    fprintf(f, "f%d(x) = %d;\n", d, d*100);
  }
  fprintf(f, "plot [y=%d:%d] \\\n", fd, ld);
  for (int i=0; i<ng; i++)
    fprintf(f, "\"evol.dat\" using 1:%d title \"%s\" with lines lw 3, \\\n", i+2, mnem[tm[i]]);
  for (int d=1; d<ND; d++) {
    fprintf(f, "f%d(y) notitle lt -1", d);
    if (d<ND-1) fprintf(f, ", "); else fprintf(f, "\n");
  }
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  fd = 1000;
  ld = 3000;
  ND = 4;

  for (int k=1; k<argc; k++) {   
    if (strcmp(argv[k], "-fd")==0) {
      if (k+1<argc) fd = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-ld")==0) {
      if (k+1<argc) ld = atoi(argv[k+1]);
    }
    if (strcmp(argv[k], "-nd")==0) {
      if (k+1<argc) ND = atoi(argv[k+1]);
      if (ND<2 || ND>8) ND = 4;
    }
  }

  Load();
  char s[100];
  do {
    printf("Number of plots: "); 
    scanf("%d", &ng);
  } while (ng<0 || ng > MAXG);
  for (int i=0; i<ng; i++) {
    sprintf(s, "Team #%d: ", i+1);
    tm[i] = GetUnique(s);
    printf("  %s\n", club[tm[i]]);
  }
  Head2Head();
  return 0;
}
