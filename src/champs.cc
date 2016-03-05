#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_MED 20

int MED;
int FYA, LYA;
int fd, ld;

int  NC;
char **club;
char **mnem;
int  **parta;
int  **med;
int  *rank;
char *filename;
int count_last;

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
//      printf("Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
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

  parta = new int*[LYA-FYA+1];
  for (int i=FYA; i<=LYA; i++) {
    parta[i-FYA] = new int[34];
  }

// quick data
  f = fopen("part.a", "rt");
  for (int y=0; y<=(LYA+count_last-1)-FYA; y++) {
    if (f==NULL) parta[y][1] = 0;
    else {
      fscanf(f, "%d %d", &dummy, &n);
      parta[y][0] = dummy;
      parta[y][1] = n;
      for (int i=0; i<n; i++) {
        fscanf(f, "%d", &t); parta[y][i+2] = t;
      }
      fgets(s, 200, f);
    }
  }
  if (f) fclose(f);
  if (fd < FYA) fd = FYA;
  if (ld < fd || ld > LYA) ld = LYA;  
  return 1;
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
  printf("\n");
  for (int i=0; i<NC; i++) {
    int x = rank[i];
    int sum = 0;
    for (int j=0; j<MED; j++) sum += med[x][j];
    if (sum) {
      printf("%2d.%-30s", i+1, club[x]);
      for (int j=0; j<MED; j++) printf(" %2d%s",  med[x][j], j==2?"  ":"");
      printf("\n");
    }
  }
}

int Medals() {
  int num;
  printf("\n");
  med = new int*[NC];
  for (int t=0; t<NC; t++) {
    med[t] = new int[MED];
    for (int j=0; j<MED; j++) med[t][j] = 0;
  }
  for (int y=fd; y<=ld; y++) {
    num = parta[y-FYA][1];
    if (num>=3) {
      printf("%4d: %-15s %-15s %-15s\n", y,
        NickOf(L,parta[y-FYA][2],y),
        NickOf(L,parta[y-FYA][3],y),
        NickOf(L,parta[y-FYA][4],y)
      );
    }
    else if (num>=2) {
      printf("%4d: %-15s %-15s\n", y,
        NickOf(L,parta[y-FYA][2],y),
        NickOf(L,parta[y-FYA][3],y)
      );
    }
    for (int j=0; j<(MED>num?num:MED); j++) 
      med[parta[y-FYA][j+2]][j]++;
  }
  Ranking();
  Listing();
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
    if (strcmp(argv[i], "-fd")==0) {
      if (i+1<argc) fd = atoi(argv[i+1]);
    }
    if (strcmp(argv[i], "-ld")==0) {
      if (i+1<argc) ld = atoi(argv[i+1]);
    }
  }
  if (!Load()) {
    fprintf(stderr, "ERROR: files not found.\n");
    return 1;
  }
  else 
    Medals();
  return 0;
}
