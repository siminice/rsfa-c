#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char *romon[] = {"", "I", "II", "III", "IV", "V", "VI",
                     "VII", "VIII", "IX", "X", "XI", "XII"};
const char *numon[] = {"", "01", "02", "03", "04", "05", "06",
                     "07", "08", "09", "10", "11", "12"};
const char *funkm[] = {"", "o1", "o2", "o3", "o4", "o5", "o6",
                     "o7", "o8", "o9", "Io", "II", "I2"};

char **club;
char **mnem;
int  NC;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64];
int  pts[64], pen[64], pdt[64], rank[64], round[64][64], res[64][64];
int  ziua[100];
int  n, ppv, tbr, p1, p2, r1, r2;
char desc[64][32];
int  wr;
//--------------------------------------

int getMonth(char *s) {
  for (int i=1; i<=12; i++) {
    if (strcmp(s, numon[i])==0) return i;
  }  
  for (int i=1; i<=12; i++) {
    if (strcmp(s, romon[i])==0) return i;
  }
  for (int i=1; i<=12; i++) {
    if (strcmp(s, funkm[i])==0) return i;
  }  
  return 0;
}

int isNumeric2(char *s) {
  if (s==NULL) return 0;
  int l = strlen(s);
  if (l<=0 || l>2) return 0;	// too long
  if (s[0]<'0' || s[0] >'9') return 0;
  if (l>1 && (s[1]<'0' || s[1] >'9')) return 0;
  return 1;
}

void Load() {
  FILE *f;
  char s[155], *tok[10], *ystr, *name;
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
}

void LoadDates(char *filename) {
  FILE *f = fopen(filename, "rt");
  if (f==NULL){
    fprintf(stderr, "ERROR: file %s not found.\n", filename);
    return;
  }
  char s[200];
  ziua[0] = -1;
  while (!feof(f)) {
     fgets(s, 200, f);
     char *rn = strtok(s, " \t-\n");
     char *rd = strtok(NULL, " -\n\t");
     if (rn!=NULL && rd!=NULL) {
       int xn = atoi(rn);
       int xd = atoi(rd);
       if (xn>0 && xn<100 && xd>0 && xd<650)
         ziua[xn] = xd;
     }
     s[0] = 0;
  }
  fclose(f);
}

int AddResult(int a, int b, int x, int y) {
  if (x<0 || y<0) return 0;
  gsc[a] += x; gre[a] += y;
  gsc[b] += y; gre[b] += x;
  if (x>y) { win[a]++; los[b]++; pts[a] += ppv; }
    else if (x==y) { drw[a]++; drw[b]++; pts[a]++; pts[b]++; }
     else { los[a]++; win[b]++; pts[b] += ppv; }
  return 1;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  char *tok[12];
  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &p1, &p2, &r1, &r2);
  char s[250];
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      res[i][j]   = z;
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

int Extract(char *filename) {
  FILE *f;
  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char *s, *t, *tkr[100], *tkd[100];
  int x, y, k, w, q, z, m, r, found;
  s = new char[1024];
  t = new char[1024];
  for (int i=0; i<n; i++) {
    fgets(s,1024,f);
    tkr[0] = strtok(s, "\t\n");
    for (int j=1; j<100; j++) tkr[j] = strtok(NULL, "\t\n");
    fgets(t,1024,f);
    tkd[0] = strtok(t, "\t\n");
    for (int j=1; j<100; j++) tkd[j] = strtok(NULL, "\t\n");

    /* get scores */
    k=1;
    found=0;
    while (found==0 && tkr[k]!=NULL) {
      w = isNumeric2(tkr[k]);
      if (w>0) found = 1;
      else k++;
    }
    if (found>0) {
      q = k;
      for (int j=0; j<n; j++) {
        if (i!=j) {
          x = atoi(tkr[q++]);
          y = atoi(tkr[q++]);
          res[i][j] = 100*x+y;
          AddResult(i, j, x, y);
        }
      }
    }
    /* get dates */
    k=0;
    found=0;
    while (found==0 && tkd[k]!=NULL) {
      w = isNumeric2(tkd[k]);
      if (w>0) found = 1;
      else k++;
    }
    if (found>0) {
      q = k;
      for (int j=0; j<n; j++) {
        if (i!=j) {
          r = atoi(tkd[q++]);
          z = ziua[r];
          round[i][j] = 1000*r+z;
          if (tkd[q]!=NULL && strchr(tkd[q], '.')!=NULL) {
            char *tkz = strtok(tkd[q], ".");
            char *tkm = strtok(NULL, ".\n");
            if (tkz!=NULL && tkm!=NULL) {
              int xz = atoi(tkz);
              int xm = getMonth(tkm);
              if (xz>0 && xz<32 && xm>0 && xm<13)
                round[i][j] = 1000*r+50*xm+xz;
            }
            q++;
          }
        }
      }
    }
  }

  fclose(f);
  delete[] s;
  delete[] t;
}

void Dump(char *inputfile) {
  FILE *f;
  int i, j;
  char *outputfile = new char[strlen(inputfile)+5];
  strcpy(outputfile, inputfile);
  strcat(outputfile, ".old");
  rename(inputfile, outputfile);
  f = fopen(inputfile, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, p1, p2, r1, r2);
  for (i=0; i<n; i++) {
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
    if (pen[i]!=0) fprintf(f, "%4d%4d", pen[i], pdt[i]);
    if (strlen(desc[i])>0) {
      if (pen[i]==0)  fprintf(f, "%4d%4d", pen[i], pdt[i]);
      fprintf(f, " %s", desc[i]);
    }
    fprintf(f, "\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", round[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  wr = 0;
  Load();  
  printf("Usage: excel <a.1999> // source must be in h[tml]/a.1999>\n");
  if (argc < 2) { printf("No input file.\n"); return 1; }
  char sy[100], dfn[100];
  strcpy(sy, argv[1]);
  char *dv = strtok(sy, ".");
  char *yr = strtok(NULL, ".");
  if (yr!=NULL) {
    int y = atoi(yr);
    sprintf(dfn, "Sportul/%d/date.txt", y);
    LoadDates(dfn);
  }
  for (int i=2; i<argc; i++) {
    if (strcmp(argv[i], "-d")==0 && i<argc-1) 
      LoadDates(argv[i+1]);
  }
  LoadFile(argv[1]);
  char src[64];  
//  Extract(argv[2]);
  sprintf(src, "h/%s", argv[1]);
  Extract(src);
  Dump(argv[1]);
  for (int i=0; i<NC; i++) delete[] mnem[i];
  delete[] mnem;
  return 0;
}
