#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char **club;
char **mnem;
int  NC;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64];
int  pts[64], pen[64], pdt[64], rank[64], round[64][64], res[64][64];
int  n, ppv, tbr, p1, p2, r1, r2;
char desc[64][32];
//--------------------------------------

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
  char *s, *tok;
  int x, y;
  s = new char[240];
//  do { fgets(s,120,f); } while (s[2]!='.');
  fgets(s,240,f);
  for (int i=0; i<n; i++) {
    int k; 
    if (i==0) {
      k = 0;
      while (k<strlen(s)-2 && !((s[k]=='X' && s[k+1]=='X') || (s[k]=='x' && s[k+1]=='x') || (s[k]=='*' && s[k+1]=='*'))) k++;
    }
    else {
      k = 0;
      while (k<strlen(s)-2 && 
           !(s[k]=='x' && s[k+1]=='x') &&
           !( ((s[k]>='0' && s[k]<='9') || s[k]=='-' || s[k]=='+' ) && 
              (s[k+1]=='-' || s[k+1]==':' || s[k+1]=='=' || s[k+1]=='/') &&
              ((s[k+2]>='0' && s[k+2]<='9') || s[k+2]=='-' || s[k+2]=='+')
            )
            ) k++;
    }
    for (int j=0; j<n; j++) {
      if (!j) tok=strtok(s+k," |\t"); else tok = strtok(NULL, " |\t");
      if (i!=j) {
        int h = 1; 
        while (h<strlen(tok) && tok[h]!='-' && tok[h]!=':' && tok[h]!='=') h++;
        if (h>=strlen(tok)) {
          AddResult(i,j,0,-1);
          continue;
        }
        tok[h] = 0;
        if (tok[h-1]=='+') {x=3; y=0;}
        else if (tok[h-1]=='-') {x=0; y=3;}
        else { x = atoi(tok); y = atoi(tok+h+1); }
        res[i][j] = 100*x+y;
        AddResult(i,j,x,y);
        printf("%s %d-%d %s\n", mnem[id[i]], x, y, mnem[id[j]]);
      }
    }
    do {fgets(s, 240, f);} while (strlen(s) < 20 && !feof(f));
  }
  delete[] s;
  fclose(f);
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
  Load();  
  printf("Usage: tables <a.1999> // source must be in html/a.1999>\n");
  if (argc < 2) { printf("No input file.\n"); return 1; }
  LoadFile(argv[1]);
  char src[64];  
//  Extract(argv[2]);
  sprintf(src, "html/%s", argv[1]);
  Extract(src);
  Dump(argv[1]);
  for (int i=0; i<NC; i++) delete[] mnem[i];
  delete[] mnem;
  return 0;
}
