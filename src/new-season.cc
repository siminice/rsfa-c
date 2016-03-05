#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAD_TO_HEAD 0

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char **club;
char **mnem;
int  NC;
int  id[30], win[30], drw[30], los[30], gsc[30], gre[30], pts[30], pen[30], pdt[30];
int  lid[30], lwin[30], ldrw[30], llos[30], lgsc[30], lgre[30], lpts[30], lpen[30], lpdt[30];
int  rank[30], round[30][30], res[30][30], allres[30][30];
int  tbwin[30], tbdrw[30], tblos[30], tbgsc[30], tbgre[30], tbrk[30];
int  day[12], home[12], 
guest[12];
int  n, r, lastr, tbr, pr1, pr2, rel1, rel2, ppv, year;

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

void Load() {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
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
    fgets(s, 500, f);
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

int after(int dt, int r) {
  if (dt==0) return 1;
  if (dt>400) {
    if (r<400) return 1;
    else return (dt<=r);
  }
  if (r>400) return 0;
  else return (dt<=r);
}

int sup(int i, int j, int tbr=0) {
  if (tbr==1) {  
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    if (tbwin[i] > tbwin[j]) return 1;
    if (tbwin[i] < tbwin[j]) return 0;
    return sup(i, j, 0);
  }
  else if (tbr==2) {  
    int p1 = ppv*win[i]+drw[i];
    int p2 = ppv*win[j]+drw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    double gc1 = 0.0;
    if (gre[i]>0) gc1 = (double) gsc[i]/ (double) gre[i];
    double gc2 = 0.0;
    if (gre[j]>0) gc2 = (double) gsc[j]/ (double) gre[j];
    if (gc1>gc2) return 1;
    if (gc1<gc2) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (i > j) return 0;
  }
  else {
    int p1 = pts[i];
    int p2 = pts[j];
    if (pen[i]!=0 && after(pdt[i],lastr)) p1 -= pen[i];
    if (pen[j]!=0 && after(pdt[j],lastr)) p2 -= pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;   
    if (gsc[i] < gsc[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
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
  int gm = 0;
  for (int i=h; i<=k; i++) {
    for (int j=h; j<=k; j++) {
      int t1 = rank[i];
      int t2 = rank[j];
      if (res[t1][t2] >= 0) {
        gm++;
        int x = res[t1][t2] / 100;
        int y = res[t1][t2] % 100;
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
      if (sup(tbrk[i+1], tbrk[i], (tbr<2 ? gm>=(k-h+1)*(k-h) : tbr))) {  
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
      if (sup(rank[i+1], rank[i], (tbr<2?0:tbr))) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
  } while (!sorted);
  if (tbr==1) {
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
//  printf("\nStandings:\n");
  printf("\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    printf("%2d.%-30s%2d%3d%3d%3d%4d-%2d", i+1,
     NameOf(L,id[x],year), win[x]+drw[x]+los[x], 
     win[x], drw[x], los[x], gsc[x], gre[x]);
     if (pen[x]>0 && after(pdt[x],lastr)) printf("%3d (-%dp pen)\n" , pts[x]-pen[x], pen[x]);
     else if (pen[x]<0 && after(pdt[x],lastr)) printf("%3d (+%dp bonus)\n" , pts[x]-pen[x], -pen[x]);
     else printf("%3d\n", pts[x]);
     if (i==pr1-1)
        printf("------------------------------------------------------\n");
     if (i==pr1+pr2-1 && pr2>0) 
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-(rel1+rel2)-1 && rel2>0) 
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-rel1-1 && rel1>0)
        printf("------------------------------------------------------\n");
  }
  printf("\n");
}

int AddResult(int a, int b, int x, int y) {
  res[a][b] = allres[a][b];
  gsc[a] += x; gre[a] += y;
  gsc[b] += y; gre[b] += x;
  if (x>y) { win[a]++; los[b]++; pts[a] += ppv; }
    else if (x==y) { drw[a]++; drw[b]++; pts[a]++; pts[b]++; }
     else { los[a]++; win[b]++; pts[b] += ppv; }
//  printf("%-15s%d-%d %s\n", mnem[id[a]], x, y, mnem[id[b]]);
  return 1;
}

int PreloadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  int ln, lppv, ltbr, lpr1, lpr2, lrel1, lrel2;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found for preloading.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &ln, &lppv, &ltbr, &lpr1, &lpr2, &lrel1, &lrel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    lid[i] = atoi(tok[0]);
    lwin[i] = atoi(tok[2]);
    ldrw[i] = atoi(tok[3]);
    llos[i] = atoi(tok[4]);
    lgsc[i] = atoi(tok[5]);
    lgre[i] = atoi(tok[6]);
    lpts[i] = atoi(tok[7]);
    if (tok[8]) lpen[i] = atoi(tok[8]); else lpen[i] = 0;
    if (tok[9]) lpdt[i] = atoi(tok[9]); else lpdt[i] = 0;
  }
  fclose(f);
  for (int i=0; i<ln; i++) {
    int j=0; 
    while (j<n && id[j]!=lid[i]) j++;
    if (j>=n) {
      printf("Error: Could not match %s in preloaded file.\n", mnem[lid[i]]);
      return 0;
    }
    win[j] += lwin[i];
    drw[j] += ldrw[i];
    los[j] += llos[i];
    gsc[j] += lgsc[i];
    gre[j] += lgre[i];
    pts[j] += lpts[i];
//    pts[j] += lpts[i] - lpen[i];
    pen[j] = 0; pdt[j] = 0;
  }
  return 1;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = drw[i] = los[i] = 0;
    gsc[i] = gre[i] = pts[i] = 0;
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      allres[i][j] = z;
      res[i][j] = -1;
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

void Synoptical() {
  char *symbol = new char[4];
  symbol[3] = 0;
  printf("\n                ");
  for (int i=0; i<n; i++) {
    memmove(symbol, NickOf(L, id[rank[i]], year), 3);
    printf("%3s ", symbol);
  }
  printf("\n");
  for (int i=0; i<n; i++) {
    printf("%-15s", NickOf(L, id[rank[i]], year));
    for (int j=0; j<n; j++)
      if (i==j) printf(" xxx");
       else if (res[rank[i]][rank[j]] >=0)
         printf("%2d-%d", res[rank[i]][rank[j]]/100, res[rank[i]][rank[j]]%100);
        else printf("  - ");
    printf("\n");
  }
}

int consecutive(int r, int last) {
  if (last%50>=30 && r%50==1) return 0;
  if (last>0 && (r-last>1 || r<last)) return 1;
  return 0;
}

void DisplayData(int full, bool winter, int fd) {
  int ng;
  lastr = -1;

  int fmo = 7; int lmo = 20;
  if (winter) { fmo=1; lmo=13; }
//  for (int m=fmo; m<lmo; m++) 
//  for (int d=(m==fmo?15:1); d<(m==lmo?16:32); d++) {
  for (int d=fd; d<fd+650; d++) {
    r = d%650;
    ng = 0;
    for (int i=0; i<n; i++)
      for (int j=0; j<n; j++)
        if (round[i][j]%1000==r && allres[i][j]>=0) {
          if (ng==0) {
            if (consecutive(r,lastr)) {
              if (full) { Ranking(); Listing(); }
              else printf("\n");
            }
            printf("Round %d [%s %d]\n", round[i][j]/1000, month[r/50], d%50);
          }
          AddResult(i, j, allres[i][j]/100, allres[i][j]%100);
          printf("%-15s%d-%d %s\n", NickOf(L, id[i], year), res[i][j]/100, 
                  res[i][j]%100, NickOf(L, id[j], year));
          ng++;
        }
    if (ng>0) lastr = r;
  }
  if (win[0]+drw[0]+los[0]==0) {
    for (int i=0; i<n; i++) 
      for (int j=0; j<n; j++) if (allres[i][j]>=0 && round[i][j]<0)
        AddResult(i,j, allres[i][j]/100, allres[i][j]%100);
  }
  Ranking();
  Listing();
  if (full) Synoptical();
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  if (argc < 2) { printf("No input file specified.\n"); return 1; }
  LoadFile(argv[1]);
  char *ystr = strtok(argv[1], ".");
  ystr = strtok(NULL, " ");
  if (ystr) year = atoi(ystr); else year = 0;
  bool full = true;
  bool winter = false;
  int fd = 365;
  if (argc > 2) {
    full = (argv[2][0] != '-');
    winter = (strchr(argv[2], 'w')!=NULL);
    if (winter) fd = 1;
    else if ('1' <= argv[2][0] && '9'>=argv[2][0])
      fd = atoi(argv[2]);
  }
  if (argc > 3) {
    PreloadFile(argv[3]);
    Ranking(); 
    Listing();
  }
  if (argc > 4) {
    PreloadFile(argv[4]);
    Ranking(); 
    Listing();
  }
  DisplayData(full, winter, fd);
  for (int i=0; i<NC; i++) {
    if (L[i]) delete L[i];
    delete[] club[i];
    delete[] mnem[i];
  }
  delete[] club;
  delete[] mnem;
  delete[] L;
  return 0;
}
