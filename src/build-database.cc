#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_LEVELS 15

int winter;

int n, ppv, tbr, pr1, pr2, rel1, rel2, dvn, ser, year;
int id[64], rank[64], pts[64]; 
int win[64], drw[64], los[64], gsc[64], gre[64];
int pen[64], pdt[64], round[64][64], res[64][64];
int round2[64][64], res2[64][64];
int ND, MAX[MAX_LEVELS], FY[MAX_LEVELS], LY[MAX_LEVELS];
FILE *dbfile;
int fd, fr, frd, ld, lrd, lr;

void LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  fprintf(stderr, "Reading from %s...\n", filename);
  f = fopen(filename, "rt");
  if (f==NULL) return;
  // Loading file
  char s[200], *tok[10], w[100];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);   
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
  }

  lrd = lr = -1;
  fr = frd = 1000;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      res[i][j]   = z;
      if (round[i][j]/1000 == 0 && round[i][j]%1000 < frd) frd = round[i][j]%1000;
      if (round[i][j]/1000 == 1 && round[i][j]%1000 < frd) frd = round[i][j]%1000;
      if (round[i][j]/1000 == 0 && round[i][j]%1000 > lrd) lrd = round[i][j]%1000;
      if (round[i][j]/1000 >=lr && round[i][j]%1000 > lrd) {
        lrd = round[i][j]%1000;
        lr = round[i][j]/1000;
      }
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
   for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round2[i][j] = r;
      res2[i][j]   = z;
      if (round2[i][j]/1000 == 0 && round2[i][j]%1000 > lrd) lrd = round2[i][j]%1000;
      if (round2[i][j]/1000 >=lr && round2[i][j]%1000 > lrd) {
        lrd = round2[i][j]%1000;
        lr = round2[i][j]/1000;
      }
    }
    fscanf(f, "\n");
   }
  }

  if (winter < 2) {
    winter = 0;
    for (int i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        if (res[i][j]>=0 && round[i][j]/1000==1) {
          if (round[i][j]%1000 <= 300) winter = 1;
        }
      }
    }    
  }

  if (frd > lrd) winter = 0;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      if (res[i][j]>=0) {
        int mon = (round[i][j]<0?13:(round[i][j]%1000)/50);
        int zi  = (round[i][j]<0?0:round[i][j]%50);
        if ((winter==0 && mon<7) || (winter>0)) mon+=12;
        w[0] = 48 + (year - 1870)/75;
        w[1] = 48 + (year - 1870)%75;
        w[2] = (char) 48+mon;
        w[3] = (char) 48+zi;
        w[4] = (char) 65+dvn;
        w[5] = (char) 48+ser;
        w[6] = (char) 48+round[i][j]/1000;
        w[7] = (char) 48+id[i]/75;
        w[8] = (char) 48+id[i]%75;
        w[9] = (char) 48+id[j]/75;
        w[10] = (char) 48+id[j]%75;
        w[11] = (char) 48+res[i][j]/100;
        w[12] = (char) 48+res[i][j]%100;
        w[13] = 0;
        printf("%s\n", w);
      }
    }
  }
  if (tbr>=10) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      if (res2[i][j]>=0) {
        int mon = (round2[i][j]%1000)/50;
        int zi  = round2[i][j]%50;
        if ((winter==0 && mon<7) || (winter==1)) mon+=12;
        w[0] = 48 + (year - 1870)/75;
        w[1] = 48 + (year - 1870)%75;
        w[2] = (char) 48+mon;
        w[3] = (char) 48+zi;
        w[4] = (char) 65+dvn;
        w[5] = (char) 48+ser;
        w[6] = (char) 48+round2[i][j]/1000;
        w[7] = (char) 48+id[i]/75;
        w[8] = (char) 48+id[i]%75;
        w[9] = (char) 48+id[j]/75;
        w[10] = (char) 48+id[j]%75;
        w[11] = (char) 48+res2[i][j]/100;
        w[12] = (char) 48+res2[i][j]%100;
        w[13] = 0;
        printf("%s\n", w);
      }
    }
  }
  }
  fclose(f);
}

int main(int argc, char**argv) {
  FILE *f, *f2, *h;
  char *filename = new char[15];
  char *outfile  = new char[15];
  char *outfile2 = new char[15];
  char s[100], *dv, *pl, *yr, *suf;
  int t;

//  dbfile = fopen("unsorted.archive.dat", "wt");

  for (int d=0; d<MAX_LEVELS; d++) {
    MAX[d] = 0; FY[d] = 2100; LY[d] = 1870;
  }

  winter = 0;
  for (int j=1; j<argc; j++) {
    if (strcmp(argv[j],"-w")==0) winter = 2;
    else if (strcmp(argv[j], "-d")==0) {
      if (j<argc-1) ND = atoi(argv[j+1]);
      if (ND<1 || ND>MAX_LEVELS) {
        fprintf(stderr, "ERROR: Illegal number of levels: %d.\n", ND);
        ND=3;
      }
    }
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
          if (l==0 || l>3) dvn=-1;
          dvn = ((int) s[0]) - 97; 
          if (dvn<0 || dvn>=ND) dvn=-1;
          if (l>1) ser = atoi(dv+1); else ser = 0;
          year = atoi(yr);
          if (dvn>=0 && year>1870 && year<2100) {
//            if (dvn+1>ND) ND = dvn+1;
            if (year > LY[dvn]) LY[dvn] = year;
            if (year < FY[dvn]) FY[dvn] = year;
            if (ser > MAX[dvn]) MAX[dvn] = ser;
          }
        }
      }
      closedir(dp);
      for (int d=0; d<ND; d++)
      fprintf(stderr,"%c: %d-%d (max:%d)\n", (char) d+65, FY[d], LY[d], MAX[d]);
  }
  else 
   fprintf(stderr, "ERROR: Couldn't open the directory.\n");

  for (year=FY[0]; year<=LY[0]; year++) {
    for (dvn=0; dvn<ND; dvn++) {
      for (ser=1; ser<=MAX[dvn]; ser++) {
        sprintf(filename, "%c%d.%d", (char) dvn+97, ser, year);
        LoadFile(filename);
      }
      ser = 0;
      sprintf(filename, "%c.%d", (char) dvn+97, year);
      LoadFile(filename);
    }
  }

//  fclose(dbfile);
  return 1;
}
