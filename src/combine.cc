#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **club;
char **mnem;
int  NC;
int  id[30], win[30], drw[30], los[30], gsc[30], gre[30], pts[30], pen[30], pdt[30];
int  lid[30], lwin[30], ldrw[30], llos[30], lgsc[30], lgre[30], lpts[30], lpen[30], lpdt[30];
int  round[30][30], res[30][30], lround[30][30], lres[30][30];
int  ln, n, r, tbr, pr1, pr2, rel1, rel2, ppv;

//--------------------------------------

void Load() {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
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

int PreloadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  int lppv, ltbr, lpr1, lpr2, lrel1, lrel2;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found for preloading.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &ln, &lppv, &ltbr, &lpr1, &lpr2, &lrel1, &lrel2);
  for (i=0; i<ln; i++) {
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
  for (i=0; i<ln; i++) {
    for (j=0; j<ln; j++) {
      fscanf(f, "%d %d", &r, &z);
      lround[i][j] = r;
      lres[i][j] = z;
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  int *per = new int[n];
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    per[i] = i;
    win[per[i]] = atoi(tok[2]);
    drw[per[i]] = atoi(tok[3]);
    los[per[i]] = atoi(tok[4]);
    gsc[per[i]] = atoi(tok[5]);
    gre[per[i]] = atoi(tok[6]);
    pts[per[i]] = atoi(tok[7]);
    if (tok[8]) pen[per[i]] = atoi(tok[8]); else pen[per[i]] = 0;
    if (tok[9]) pdt[per[i]] = atoi(tok[9]); else pdt[per[i]] = 0;
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[per[i]][per[j]] = r;
      res[per[i]][per[j]] = z;
    }
    fscanf(f, "\n");
  }
  fclose(f);
  delete[] per;
}

void Dump() {
  int i, j;
  printf("%d %d %d %d %d %d %d\n", n+ln, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<ln; i++) {
    printf("%4d%4d%4d%4d%4d%4d%4d%4d",
      lid[i], 
      lwin[i]+ldrw[i]+llos[i],
      lwin[i], 
      ldrw[i], 
      llos[i], 
      lgsc[i], 
      lgre[i], 
      lpts[i]);
    if (lpen[i]!=0) printf("%4d%4d", lpen[i], lpdt[i]);
    printf("\n");
  }
  for (i=0; i<n; i++) {
    printf("%4d%4d%4d%4d%4d%4d%4d%4d",
      id[i], 
      win[i]+drw[i]+los[i], 
      win[i], 
      drw[i], 
      los[i], 
      gsc[i], 
      gre[i], 
      pts[i]);
    if (pen[i]!=0) printf("%4d%4d", pen[i], pdt[i]);
    printf("\n");
  }
  for (i=0; i<ln; i++) {
    for (j=0; j<ln; j++)
      printf("%6d%5d", lround[i][j], lres[i][j]);
    for (j=0; j<n; j++) printf("    -1   -1");
    printf("\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<ln; j++) printf("    -1   -1");
    for (j=0; j<n; j++)
      printf("%6d%5d", round[i][j], res[i][j]);
    printf("\n");
  }
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  if (argc < 3) { printf("Usage: combine f1 f2\n"); return 1; }
  PreloadFile(argv[1]);
  LoadFile(argv[2]);
  Dump();
  return 0;
}
