#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXT 2000
#define MAXY 1000
#define MAXJ 50

const char* fxcol[] = {"77FF77", "FFFF99", "FF5050"};

int  NJ, NC, ND, fd, ld;
char **club;
char **mnem;
int  ****part;
int *FY, *LY, *MAX;
int  ng;
int  tm[MAXT];
int num_winter;
int *start_winter, *end_winter;

char judet[MAXJ][16];
char jmnem[MAXJ][3];
int  jd[MAXT];

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


int FindJ(char *s) {
  int l = strlen(s);
  for (int i=0; i<l; i++)
    if (s[i]>='a' && s[i]<='z') s[i] -= 32;
  for (int i=0; i<NJ; i++)
   if (strcmp(jmnem[i], s)==0) return i;
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

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  int dummy, n, p, t, d, y;
  char *dv, *pl, *yr, *suf;
  char filename[64];

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

  f = fopen("judete.txt", "rt");
  if (f!=NULL) {
    fscanf(f, "%d\n", &NJ);
    for (int i=0; i<NJ; i++) {
      fgets(s, 100, f);
      int l = strlen(s);
      tok[0] = strtok(s, ",");
      tok[1] = strtok(NULL, "\n");
      strcpy(jmnem[i], tok[0]);
      strcpy(judet[i], tok[1]);
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

  f = fopen("judete.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    if (!s) continue;
    if (strlen(s) < 3) continue;
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, ":");
    jd[i] = FindJ(tok[0]);
    s[0] = 0;
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
    tok[1] = strtok(s, "*");
    for (int j=2; j<20; j++) {
      tok[j] = strtok(NULL, "*");
    }
    int k=1;
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

void Judete(int j) {
  int i, ij[10], dv[MAXT][3][100], nt[MAXY][3], tm[MAXY][3][100], maxt[3];
  char filename[100];
  
  maxt[0] = maxt[1] = maxt[2] = 0;
  for (int y=fd; y<=LY[0]; y++) {
    int yy = y-FY[0];
    nt[yy][0] = nt[yy][1] = nt[yy][2] = 0;
    for (int d=0; d<ND; d++) {
      for (int p=(d==0?0:1); p<=MAX[d]; p++) {
        if (y<FY[d] || y>LY[d]) continue;
        int n = part[d][y-FY[d]][p][1];
        for (int h=2; h<n+2; h++) {
          int x = part[d][y-FY[d]][p][h];
          if (jd[x] == j) {
            int m = nt[yy][d];
            tm[yy][d][m] = x;
            dv[yy][d][m] = 100*d+p;
            nt[yy][d]++;
            if (nt[yy][d] > maxt[d]) maxt[d] = nt[yy][d];
          }
        }
      }
    }
  }
  int tnt = maxt[0] + maxt[1] + maxt[2];

  char ssn[32];
  int start;
  sprintf(filename, "html/%s.html", jmnem[j]);
  FILE *f = fopen(filename, "wt");
  fprintf(f, "<HTML>\n<HEAD><TITLE>Judeþul %s</TITLE></HEAD>\n<BODY>\n", judet[j]);
  fprintf(f, "<TABLE BORDER=\"1\" RULES=\"all\" FRAME=\"box\">\n");
  fprintf(f, "<COLGROUP></COLGROUP>\n");
  if (maxt[0]>0) fprintf(f, "<COLGROUP SPAN=\"%d\"></COLGROUP>\n", maxt[0]);
  if (maxt[1]>0) fprintf(f, "<COLGROUP SPAN=\"%d\"></COLGROUP>\n", maxt[1]);
  if (maxt[2]>0) fprintf(f, "<COLGROUP SPAN=\"%d\"></COLGROUP>\n", maxt[2]);

  start = 0;
  for (int y=fd; y<=LY[0]; y++) {
    int yy = y-FY[0];
    if (nt[yy][0]+nt[yy][1]+nt[yy][2] > 0) start = 1;
      if ((part[0][yy][0][1]>0 || part[0][yy][1][1]>0) && start>0) {
        fprintf(f, "<TBODY><TR>\n");
        SeasonName(y, ssn);
        fprintf(f, "<TD>%s</TD>", ssn);
        for (int d=0; d<3; d++) {
        for (int k=0; k<maxt[d]; k++) {
          if (k<nt[yy][d]) {
            fprintf(f, "<TD BGCOLOR=\"%s\">%s</TD>\n", fxcol[d], NickOf(L, tm[yy][d][k], y));
          }
          else {
            fprintf(f, "<TD></TD>");
          } // if k
        } // for k
      } // for d
      fprintf(f, "</TR></TBODY>\n");
    } // if start
  } // for y
  fprintf(f, "</TABLE>\n");
  fprintf(f, "</BODY></HTML>\n");
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  fd = 1933;
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
  int j = -1;

/*
  do {
    printf("Judet: "); 
    scanf("%s", s);
    j = FindJ(s);
  } while (j<0);
*/

  for (int j=0; j<NJ; j++) {
    Judete(j);
  }
  return 0;
}
