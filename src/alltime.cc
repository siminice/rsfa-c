#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int FYA, LYA, FYB, LYB, FYC, LYC, MAXA, MAXB, MAXC;

int  NC;
char **club;
char **mnem;
int  **parta;
int  ***partb;
int  ***partc;
int id[64];
int *ed, *win, *drw, *los, *gsc, *gre, *pts, *rank, *last;
char inputfile[64], *outputfile;
int n, ppv, tbr;
int quiet, pause;
int fd, ld;

const char *hiyellow  = "\033[33;1m";
const char *normal  = "\033[0m";

void Load() {
  FILE *f;
  char s[200];
  int dummy, n, p, t, d, y;
  char *dv, *pl, *yr, *suf;

  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
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
  ed  = new int[NC];
  win = new int[NC];
  drw = new int[NC];
  los = new int[NC];
  gsc = new int[NC];
  gre = new int[NC];
  pts = new int[NC];
  rank= new int[NC];
  last= new int[NC];
  for (int i=0; i<NC; i++)
    ed[i] = win[i] = drw[i] = los[i] = gsc[i] = gre[i] = pts[i] = 0;

  FYA = 2100; LYA = 1800;
  FYB = 2100; LYB = 1800;
  FYC = 2100; LYC = 1800;
  MAXA = MAXB = MAXC = 0;
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
          if (l>0) p = atoi(dv+1); else p = 1;
          y = atoi(yr);
          if (d>0 && y>1888 && y<2100) {
            switch(d) {
              case 1:
               if (y<FYA) FYA = y;
               if (y>LYA) LYA = y;
               if (p>MAXA) MAXA = p;
               break;
              case 2:
               if (y<FYB) FYB = y;
               if (y>LYB) LYB = y;
               if (p>MAXB) MAXB = p;
               break;
              case 3:
               if (y<FYC) FYC = y;
               if (y>LYC) LYC = y;
               if (p>MAXC) MAXC = p;
               break;
            }
          }
        }
      }
      closedir(dp);  
      printf("A: %d - %d\n", FYA, LYA);
      printf("B: %d - %d (max: %d)\n", FYB, LYB, MAXB);
      printf("C: %d - %d (max: %d)\n", FYC, LYC, MAXC);
  }
  else
   printf("ERROR: Couldn't open the directory.\n");  

  fd = FYA;
  ld = LYA;
  parta = new int*[LYA-FYA+1];
  for (int i=FYA; i<=LYA; i++)
    parta[i-FYA] = new int[64];
  if (MAXB>0) {
    partb = new int**[LYB-FYB+1];
    for (int i=FYB; i<=LYB; i++) {
      partb[i-FYB] = new int*[MAXB];
      for (int j=0; j<MAXB; j++) {
        partb[i-FYB][j] = new int[64];
      }
    }
  }
  if (MAXC>0) {
    partc = new int**[LYC-FYC+1];
    for (int i=FYC; i<=LYC; i++) {
      partc[i-FYC] = new int*[MAXC];
      for (int j=0; j<MAXC; j++) {
        partc[i-FYC][j] = new int[64];
      }
    }
  }
// quick data
  f = fopen("part.a", "rt");
  for (int y=0; y<=LYA-FYA; y++) {
    fscanf(f, "%d %d", &dummy, &n);
    parta[y][0] = dummy;
    parta[y][1] = n;
    for (int i=0; i<n; i++) {
      fscanf(f, "%d", &t); parta[y][i+2] = t;
    }
    fgets(s, 200, f);
  }
  fclose(f);

  char *filename = new char[15];
  if (MAXB>0) {
  if (MAXB>1)
    strcpy(filename, "part.b1");
  else 
    strcpy(filename, "part.b");
  for (int i=0; i<MAXB; i++) {
    if (MAXB>1)
      filename[6] = i+49;
    f = fopen(filename, "rt");
    for (int y=0; y<=LYB-FYB; y++) {
      fscanf(f, "%d %d", &dummy, &n);
      partb[y][i][0] = dummy;
      partb[y][i][1] = n;
      for (int j=0; j<n; j++) {
	fscanf(f, "%d", &t); partb[y][i][j+2] = t;
      }
      fgets(s, 200, f);
    }
    fclose(f);
  }
  }

  if (MAXC>0) {
  if (MAXC>1)
    strcpy(filename, "part.c1");
  else
    strcpy(filename, "part.c");
  for (int i=0; i<MAXC; i++) {
    if (MAXC>1)
      sprintf(filename, "part.c%d", i+1);
    f = fopen(filename, "rt");
    for (int y=0; y<=LYC-FYC; y++) {
      fscanf(f, "%d %d", &dummy, &n);
      partc[y][i][0] = dummy;
      partc[y][i][1] = n;
      for (int j=0; j<n; j++) {
	fscanf(f, "%d", &t); partc[y][i][j+2] = t;
      }
      fgets(s, 200, f);
    }
    fclose(f);
  }
  }

}

int sup(int i, int j) {
    int p1 = pts[i];
    int p2 = pts[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (i > j) return 0;
    return 1; 
}
    
void Ranking() {
  // BubbleSort
  int i, j;
  for (i=0; i<NC; i++) rank[i] = i;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<NC-1; i++)
      if (sup(rank[i+1], rank[i])) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
  } while (!sorted);
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j;
  f = fopen(filename, "rt");
  if (NULL == f) { return 0; }
  // Loading file
  char s[100], *tok[10];
  fgets(s, 100, f);
  sscanf(s, "%d %d %d", &n, &ppv, &tbr);
  bool isA = (filename[0]=='a');
  for (i=0; i<NC; i++) last[i] = 0;
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<8; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    if (filename[1]=='.') ed[id[i]]++;
    int _w = atoi(tok[2]);
    int _d = atoi(tok[3]);
    int _l = atoi(tok[4]);
    int _s = atoi(tok[5]);
    int _r = atoi(tok[6]);
    win[id[i]] += _w;
    drw[id[i]] += _d;
    los[id[i]] += _l;
    gsc[id[i]] += _s;
    gre[id[i]] += _r;
//    pts[id[i]] += atoi(tok[7]);
    pts[id[i]] += 2*_w+_d;
    last[id[i]] = 1;
  }
  fclose(f);
  return 1;
}

void Listing() {
  for (int i=0; i<NC; i++) {
    int x = rank[i];
    double pct = ((double)(50.00*pts[x]))/((double)(win[x]+drw[x]+los[x]));
    if (pts[x]>0)
    printf("%s%2d.%-30s(%3d) %5d %4d %4d %4d %5d-%4d %4dp [%5.2f%%]%s\n",
     (last[x]?hiyellow:normal),
     i+1, club[x], ed[x], win[x]+drw[x]+los[x],
     win[x], drw[x], los[x], gsc[x], gre[x], pts[x], pct, normal);
  }
}

void Evolution() {
  char filename[64];
  for (int y=fd; y<=ld; y++) {
    for (int d=1; d<=MAXA; d++) {
      sprintf(filename, "a%d.%d", d, y);
      LoadFile(filename);
      Ranking();
    }
    sprintf(filename, "a.%d", y);
    LoadFile(filename);
    if (!quiet) {
      Ranking();
      printf("\n%d standings:\n", y);
    }
    else if (y==LYA) {
      Ranking();
    }
    if (!quiet || y==LYA || y==ld) {
      Listing();
      if (pause) getc(stdin);
    }
  }
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();
  quiet = 0;
  pause = 0;
  for (int j=1; j<argc; j++) {
    if (strcmp(argv[j], "-")==0) quiet = 1;
    else if (strcmp(argv[j], "-q")==0) quiet = 1;
    else if (strcmp(argv[j], "-p")==0) pause = 1;
    else if (strcmp(argv[j], "+")==0) pause = 1;
    if (strcmp(argv[j], "-fy")==0 || strcmp(argv[j], "-f")==0) {
      if (j+1<argc) fd = atoi(argv[j+1]);
    }
    if (strcmp(argv[j], "-ly")==0 || strcmp(argv[j], "-l")==0) {
      if (j+1<argc) ld = atoi(argv[j+1]);
    }
  }
  quiet = (argc>1 && argv[1][0]=='-');
  Evolution();
}
