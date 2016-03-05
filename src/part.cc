#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS 10

int ND;
int  ****part;
int *FY, *LY, *MAX;

int n, ppv, tbr, pr1, pr2, rel1, rel2;
int id[64], rank[64], pts[64]; 
int win[64], drw[64], los[64], gsc[64], gre[64];
int pen[64], pdt[64], round[64][64], res[64][64], round2[64][64], res2[64][64];
int tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];

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
    if (res[i][j]/100 + res[j][i]%100 > res[j][i]/100 + res[i][j]%100) return 1;
    if (res[i][j]/100 + res[j][i]%100 < res[j][i]/100 + res[i][j]%100) return 0;
    if (res[i][j]%100 < res[j][i]%100) return 1;
    if (res[i][j]%100 > res[j][i]%100) return 0;
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

void LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  f = fopen(filename, "rt");
  // Loading file
  char s[200], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);   
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      res[i][j]   = z;
      round2[i][j] = -1;
      res2[i][j] = -1;
    }
    fscanf(f, "\n");
  }

  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        round2[i][j] = r;
        res2[i][j] = z;
      }
      fscanf(f, "\n");
    }
  }

  fclose(f);
}

int main(int argc, char **argv) {
  FILE *f, *f2, *h;
  char *filename = new char[15];
  char *outfile  = new char[15];
  char *outfile2 = new char[15];
  char *year     = new char[5];
  char s[100], *dv, *pl, *yr, *suf;

  if (argc>1 && strcmp(argv[1], "4")==0) ND = 4;

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


//-------------------------------

 for (int d=0; d<ND; d++) {

  printf("Compiling level %c...\n", (char) (d+65));
  if (MAX[d]>-100) {
    sprintf(outfile,  "part.%c", (char)(d+97));
    f = fopen(outfile, "wt");
    for (int y=FY[d]; y<=LY[d]; y++) {
      sprintf(filename, "%c.%d", (char) (d+97), y);
//      printf("Reading from %s...\n", filename);
      h = fopen(filename, "rt");
      if (h==NULL) { 
//          printf("File %s not found...\n", filename);
          fprintf(f, "%d 0\n", y);
   	  continue; 
      }
      fclose(h);
      LoadFile(filename);
      Ranking();
      fprintf(f, "%d %d ", y, n);
      for (int j=0; j<n; j++)
	fprintf(f, "%3d ", id[rank[j]]);
      fprintf(f, "\n");
    }
    fclose(f);

    for (int i=1; i<=MAX[d]; i++) {
      sprintf(outfile, "part.%c%d", (char) (d+97), i);
      f = fopen(outfile, "wt");
      for (int y=FY[d]; y<=LY[d]; y++) {
        sprintf(filename, "%c%d.%d", (char) (d+97), i, y);
//        printf("Reading from %s...\n", filename);
        h = fopen(filename, "rt");
        if (h==NULL) { 
//          printf("File %s not found...\n", filename);
          fprintf(f, "%d 0\n", y);
          continue; 
        }
        fclose(h);
        LoadFile(filename);
        Ranking();
        fprintf(f, "%d %d ", y, n);
        for (int j=0; j<n; j++) 
          fprintf(f, "%3d ", id[rank[j]]);
        fprintf(f, "\n");
      }
      fclose(f);
    }
  }
 }
  
  
//-------------------------------

  return 1;
}
