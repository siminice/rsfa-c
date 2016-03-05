#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS 12

int ND;
int  ****part;
int *FY, *LY, *MAX;

int NC;
char **club;
char **mnem;

int n, ppv, tbr, pr1, pr2, rel1, rel2;
int id[64], rank[64], pts[64]; 
int win[64], drw[64], los[64], gsc[64], gre[64];
int pen[64], pdt[64], rnd[64][64], res[64][64], rnd2[64][64], res2[64][64];
int tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
char desc[64][32];
char **deco;
char **oldd;
int concat;
int single;
int league[MAX_LEVELS];
int  pos[MAX_LEVELS];
int  pox[MAX_LEVELS];
char mnemo[100];
int  cup_winner;
int  cup_finalist;
int  num_euro;
char filename[95];

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

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
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
//   printf("%3d.%-30s [%-15s] extracted...\n", i, club[i], mnem[i]);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    fgets(s, 2000, f);
    if (!s) continue;
    if (s[0]==0) continue;
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

//-----------------------------------

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
    int p1 = pts[i] - pen[i];
    int p2 = pts[j] - pen[j];
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
  for (int i=h; i<=k; i++) {
    for (int j=h; j<=k; j++) {
      int t1 = rank[i];
      int t2 = rank[j];
      if (res[t1][t2] >= 0) {
        int x = res[t1][t2] / 100;
        int y = res[t1][t2] % 100;
        tbgsc[t1] += x; tbgre[t2] += x;
        tbgsc[t2] += y; tbgre[t1] += y;
        if (x>y) {tbwin[t1]++; tblos[t2]++;}
        else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
        else {tbwin[t2]++; tblos[t1]++;}
      }
      if (res2[t1][t2] >= 0) {
        int x = res2[t1][t2] / 100;
        int y = res2[t1][t2] % 100;
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
      if (sup(tbrk[i+1], tbrk[i], tbr)) {
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
  if (tbr) {
    i = 0;
    while (i<n-1) {
      j = i;
      while (j+1<n && (pts[rank[j+1]]-pen[rank[j+1]] == pts[rank[i]]-pen[rank[i]])) j++;
      if (j>i) Tiebreak(i,j);  
      i = j+1;
    }
  }
}

void Dump(char *inputfile) {
  char outputfile[100];
  FILE *f;
  int i, j;
  strcpy(outputfile, inputfile);
  strcat(outputfile, ".old");
  rename(inputfile, outputfile);
  f = fopen(inputfile, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<n; i++) {
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
    if (pen[i]!=0) fprintf(f, "%4d%4d", pen[i], pdt[i]);
    if (concat)  
      strcat(desc[i], deco[id[i]]);
    else 
      strcpy(desc[i], deco[id[i]]);
    if (strlen(desc[i])>0) {
      if (pen[i]==0)  fprintf(f, "%4d%4d", pen[i], pdt[i]);
      fprintf(f, " %s", desc[i]);
    }
    fprintf(f, "\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", rnd[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++)
        fprintf(f, "%6d%5d", rnd2[i][j], res2[i][j]);
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

void LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  f = fopen(filename, "rt");
  // Loading file
  char s[200], *tok[20];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);   
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      rnd[i][j] = r;
      res[i][j]   = z;
      rnd2[i][j] = -1;
      res2[i][j] = -1;
    }
    fscanf(f, "\n");
  }

  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        rnd2[i][j] = r;
        res2[i][j] = z;
      }
      fscanf(f, "\n");
    }
  }

  fclose(f);
}

int in(int y, int d, int p, int t) {
  if (d<0 || d>=ND) return 0;
  if (MAX[d]==0) return 0;
  if (p>MAX[d]) return 0;
  if (y<FY[d] || y>LY[d]) return 0;
  int m = part[d][y-FY[d]][p][1];
  for (int j=2; j<m+2; j++)
    if (part[d][y-FY[d]][p][j] == t) return j-1;
  return -1;
}

int Movements(int d, int p , int year) {
       if (year < FY[d] || year > LY[d]) {
          return -1;
       }
       int nx = part[d][year-FY[d]][p][1];
       if (nx<=0) {
         return -2;
       }
       printf("Divisional movements in %c%d:\n\n", (char)(d+65), p);
       for (int i=0; i<nx; i++) {
         int x = part[d][year-FY[d]][p][i+2];

         for (int l=0; l<ND; l++) {
           pos[l] = in(year-1, l, 0, x);
           if (pos[l] <= 0) {
             for (int q=1; q<=MAX[l]; q++) {
               int pq = in(year-1, l, q, x);
               if (pq > 0) pos[l] = pq;
             }
           }
           pox[l] = in(year+1, l, 0, x);
           if (pox[l] <= 0) {
             for (int q=1; q<=MAX[l]; q++) {
               int pq = in(year+1, l, q, x);
               if (pq > 0) pox[l] = pq;
             }
           }
         }
 
         strcpy(mnemo, NameOf(L, x, year));
         char dm = (char)(d+65);
         if (d==0) {
           if (pos[d]==1) {
             printf("%3d.%-30s [1]\n", x, mnemo);
             strcpy(deco[x], "(1)");
           }
           else if (pos[d]>0 && pos[d]<=num_euro) {
             printf("%3d.%-30s [E]\n", x, mnemo);
             strcpy(deco[x], "(E)");
           }
         }        
         if (pos[d] <= 0 || pox[d] <= 0) {
           if (d>0 && pos[d-1]>0) {
              printf("%3d.%-30s [R]", x, mnemo);
              strcpy(deco[x], "(-)");
           }
           else if (d<ND && pos[d+1]>0) {
             printf("%3d.%-30s [N]", x, mnemo);
             strcpy(deco[x], "(+)");
           }
           else if (pos[d]<=0) {
             printf("%3d.%-30s [+]", x, mnemo);
             strcpy(deco[x], "(+)");
           }

           if (year < LY[d] && pox[d] <= 0) {
             if (pos[d]>0) printf("%3d.%-30s    ", x, mnemo);
             if (d>0 && pox[d-1]>0) { 
                printf(" ^^^^");
             }
             else if (d<ND && pox[d+1]>0) {
               printf(" \\/\\/");
             }
             else {
               printf(" ????");
             }
             if (pos[d] > 0) printf("\n");
           }
           if (pos[d] <= 0) printf("\n");
         } 
       }

       int ok = 0;
       printf("Confirm? [0/1]: "); scanf("%d", &ok);
       if (ok==1) {
         if (p==0) sprintf(filename, "%c.%d", (char) (d+97), year);
         else sprintf(filename, "%c%d.%d", (char) (d+97), p, year);
         LoadFile(filename);
         Dump(filename);
       }  
  return 0;
}

int main(int argc, char **argv) {
  FILE *f, *f2, *h;
  char s[100], *dv, *pl, *yr, *suf;
  int  year, dvn, pool;
  num_euro = 3;
  concat = 0;
  single = 0;

  league[0] = 1;
  for (int i=1; i<MAX_LEVELS; i++)  league[i] = 1;
  for (int k = 1; k<argc; k++) {
    if (strcmp(argv[k], "-h")==0 || strcmp(argv[k],"-help")==0) {
      printf("Usage: decorate [-y year] [-C cup_winner] [-c cup_finalist] [-e num_euro] [-lA]\n");
      return 1;
    }
    if ((strcmp(argv[k], "-f")==0 || strcmp(argv[k], "-s")==0) && k<argc-1) {
      single = 1;
      strcpy(filename, argv[k+1]);
      char sf[100];
      strcpy(sf, filename);
      dv = strtok(sf, ".");
      yr = strtok(NULL, ".");
      suf = strtok(NULL, ".");
      if (dv==NULL || yr==NULL || suf!=NULL) {
        fprintf(stderr, "ERROR: Invalid input file %s\n", sf);
       return -1;
      }
      dvn = dv[0] - 97;
      pool = 0;
      if (strlen(dv)>1) pool = atoi(dv+1);
      year = atoi(yr);
    }
    if (strcmp(argv[k], "-y")==0)
      year = atoi(argv[k+1]);
    else if (strcmp(argv[k], "-C")==0)
      cup_winner = atoi(argv[k+1]);
    else if (strcmp(argv[k], "-c")==0) {
      cup_finalist = atoi(argv[k+1]);
    }
    else if (strcmp(argv[k], "-e")==0) {
      num_euro = atoi(argv[k+1]);
    }
    else if (strcmp(argv[k], "-cat")==0 || strcmp(argv[k], "-c")==0) {
      concat = 1;
    }
    else if (strstr(argv[k], "-l")!=NULL) {
      for (int i=1; i<MAX_LEVELS; i++)  league[i] = 0;
      for (int h=2; h<strlen(argv[k]); h++) {
        char c = argv[k][h];
        if (c>='a' && c<='j') league[c-97] = 1;
        if (c>='A' && c<='J') league[c-65] = 1;
      }
    }
  }

  part = new int***[MAX_LEVELS];
  FY = new int[MAX_LEVELS];
  LY = new int[MAX_LEVELS];
  MAX = new int [MAX_LEVELS];

  ND = 0;
  for (int d=0; d<MAX_LEVELS; d++) {
    MAX[d] = 0;
    FY[d] = 2100;
    LY[d] = 1800;
  }
  int t, d, p, y;

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
          d = ((int) s[0]) - 97; 
          if (d<0 || d>=MAX_LEVELS) d=-1;
          if (d+1>ND) ND = d+1;
          if (l>1) p = atoi(dv+1); else p = 1;
          y = atoi(yr);
          if (d>=0 && y>1888 && y<2100) {
//            printf("%s: %d %d %d\n", ep->d_name, d, p, y);
            if (p>MAX[d]) MAX[d] = p;
            if (y<FY[d]) FY[d] = y;
            if (y>LY[d]) LY[d] = y;
          }
        }
      }
      closedir(dp);
      ND = MAX_LEVELS-1;
      while (ND>=0 && MAX[ND]==0) ND--;
      ND++;
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
          part[d][i-FY[d]][j][1] = 0;
        }
      }
    }
  }


//-------------------------------
  if (year < FY[0] || year > LY[0]) {
    printf("ERROR: Year out of range (%d-%d).\n", FY[0], LY[0]);
    return -1;
  }

//-------------------------------
// --- Fill in data

 Load();

 for (int d=0; d<ND; d++) if (league[d]>0) {

//   printf("Compiling level %c...\n", (char) (d+65));
   if (MAX[d]>-100) {
     for (int i=0; i<=MAX[d]; i++) {
       for (int y=FY[d]; y<=LY[d]; y++) {
         part[d][y-FY[d]][i][0] = y;
         part[d][y-FY[d]][i][1] = 0;
         if (i==0) sprintf(filename, "%c.%d", (char) (d+97), y);
         else sprintf(filename, "%c%d.%d", (char) (d+97), i, y);
         h = fopen(filename, "rt");
         if (h==NULL) { continue; }
         fclose(h);
         LoadFile(filename);
         for (int hh=0; hh<n; hh++) rank[hh] = hh;
         if (y<LY[d]) Ranking();
//         Ranking();
         part[d][y-FY[d]][i][0] = y;
         part[d][y-FY[d]][i][1] = n;
         for (int j=0; j<n; j++) 
           part[d][y-FY[d]][i][j+2] = id[rank[j]];
       }
     }
/*
     for (int y=FY[d]; y<=LY[d]; y++) {
       part[d][y-FY[d]][0][0] = y;
       part[d][y-FY[d]][0][1] = 0;
       h = fopen(filename, "rt");
       if (h==NULL) { continue; }
       fclose(h);
       LoadFile(filename);
       Ranking();
       part[d][y-FY[d]][0][0] = y;
       part[d][y-FY[d]][0][1] = n;
       for (int j=0; j<n; j++)  
 	part[d][y-FY[d]][0][j+2] =  id[rank[j]];
     }
    fclose(f);
*/
   }
 }
    
//-------------------------------
//--- Check all

 deco = new char*[NC];
 for (int i=0; i<NC; i++) {
  deco[i] = new char[5];
  strcpy(deco[i], "");
 }

 if (single) {
   Movements(dvn, pool, year);
   return 0;
 }


 for (int d=0; d<ND; d++) {
   printf("--------------------------------------------------------------------\n");
   if (MAX[d]>-100) {
     for (int p=0; p<=MAX[d]; p++) {

     Movements(d, p, year);

/*
       if (year < FY[d] || year > LY[d]) continue;
       int nx = part[d][year-FY[d]][p][1];
       if (nx<=0) continue;
       printf("Divisional movements in %c%d:\n\n", (char)(d+65), p);
       for (int i=0; i<nx; i++) {
         int x = part[d][year-FY[d]][p][i+2];

         for (int l=0; l<ND; l++) {
           pos[l] = in(year-1, l, 0, x);
           if (pos[l] <= 0) {
             for (int q=1; q<=MAX[l]; q++) {
               int pq = in(year-1, l, q, x);
               if (pq > 0) pos[l] = pq;
             }
           }
           pox[l] = in(year+1, l, 0, x);
           if (pox[l] <= 0) {
             for (int q=1; q<=MAX[l]; q++) {
               int pq = in(year+1, l, q, x);
               if (pq > 0) pox[l] = pq;
             }
           }
         }
 
         strcpy(mnemo, NameOf(L, x, year));
         char dm = (char)(d+65);
         if (d==0) {
           if (pos[d]==1) {
             printf("%3d.%-30s [1]\n", x, mnemo);
             strcpy(deco[x], "(1)");
           }
           else if (pos[d]>0 && pos[d]<=num_euro) {
             printf("%3d.%-30s [E]\n", x, mnemo);
             strcpy(deco[x], "(E)");
           }
         }        
         if (pos[d] <= 0 || pox[d] <= 0) {
           if (d>0 && pos[d-1]>0) {
              printf("%3d.%-30s [R]", x, mnemo);
              strcpy(deco[x], "(-)");
           }
           else if (d<ND && pos[d+1]>0) {
             printf("%3d.%-30s [N]", x, mnemo);
             strcpy(deco[x], "(+)");
           }
           else if (pos[d]<=0) {
             printf("%3d.%-30s [+]", x, mnemo);
             strcpy(deco[x], "(+)");
           }

           if (year < LY[d] && pox[d] <= 0) {
             if (pos[d]>0) printf("%3d.%-30s    ", x, mnemo);
             if (d>0 && pox[d-1]>0) { 
                printf(" ^^^^");
             }
             else if (d<ND && pox[d+1]>0) {
               printf(" \\/\\/");
             }
             else {
               printf(" ????");
             }
             if (pos[d] > 0) printf("\n");
           }
           if (pos[d] <= 0) printf("\n");
         } 
       }

       int ok = 0;
       printf("Confirm? [0/1]: "); scanf("%d", &ok);
       if (ok==1) {
         if (p==0) sprintf(filename, "%c.%d", (char) (d+97), year);
         else sprintf(filename, "%c%d.%d", (char) (d+97), p, year);
         LoadFile(filename);
         Dump(filename);
       }
*/

     }
   }
 }
   printf("--------------------------------------------------------------------\n");
  
  

  return 1;
}
