#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char **club;
char **mnem;
int  NC;
int  id[24], win[24], drw[24], los[24], gsc[24], gre[24];
int  pts[24], rank[24], round[24][24], res[24][24];
int  round2[24][24], res2[24][24];
int  n, ppv, tbr, p1, p2, r1, r2;

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
  if (res[a][b]<0) res[a][b] = 100*x+y;
  else if (res2[a][b]<0) res2[a][b] = 100*x+y;
  else return 0;
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
  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n"); return 0; }
  // Loading file
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &p1, &p2, &r1, &r2);
  char s[100];
  for (i=0; i<n; i++) {
    fgets(s, 100, f);
    sscanf(s, "%d", id+i);
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = -1;
      res[i][j]   = -1;
      round2[i][j] = -1;
      res2[i][j]  = -1;
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

int Extract(char *filename) {
  FILE *f;
  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n"); return 0; }
  // Loading file
  char *s, *t, *tk1[32], *tk2[32], *tka, *tkb;
  int x, y;
  s = new char[240];
  t = new char[240];
  fgets(s,240,f); 
  fgets(t,240,f);
  int k = 0; 
  while (!((s[k]=='X' && s[k+1]=='X') || 
           (s[k]=='x' && s[k+1]=='x') ||
           (s[k]=='*' && s[k+1]=='*')
        )) k++;

  for (int i=0; i<n; i++) {
    tk1[0] = strtok(s+k, " \t\n");
    for (int h=1; h<n; h++) tk1[h] = strtok(NULL, " \t\n");
    tk2[0] = strtok(t+k, " \t\n");
    for (int h=1; h<n; h++) tk2[h] = strtok(NULL, " \t\n");
    for (int j=0; j<n; j++) if (i!=j) {
      x = y = -1;
      tka = strtok(tk1[j], "-");
      tkb = strtok(NULL, "-");
      if (tka!=NULL && strlen(tka)>0) x = atoi(tka); 
      if (tkb!=NULL && strlen(tkb)>0) y = atoi(tkb);
      if (x>=0 && y>=0 && x<=20 && y<=20) {
        AddResult(i,j,x,y);
        printf("%s %d-%d %s\n", mnem[id[i]], x, y, mnem[id[j]]);
      }
      x = y = -1;
      tka = strtok(tk2[j], "-");
      tkb = strtok(NULL, "-");
      if (tka!=NULL && strlen(tka)>0) x = atoi(tka); 
      if (tkb!=NULL && strlen(tkb)>0) y = atoi(tkb);
      if (x>=0 && y >=0 && x<=20 && y<=20) {
        AddResult(i,j,x,y);
        printf("%s %d-%d %s\n", mnem[id[i]], x, y, mnem[id[j]]);
      }
    }
    fgets(s,240,f);
    fgets(t, 240, f);
  }
  delete[] s;
  delete[] t;
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
  for (i=0; i<n; i++)
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d\n",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", round[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10)
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", round2[i][j], res2[i][j]);
    fprintf(f, "\n");
  }
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  printf("Usage: tables <a.1999> (table in \"a.1999.txt\")\n");
  if (argc < 2) { printf("No input file.\n"); return 1; }
  LoadFile(argv[1]);
  char src[64];
  sprintf(src, "html/%s", argv[1]);
  Extract(src);
  Dump(argv[1]);
  for (int i=0; i<NC; i++) delete[] mnem[i];
  delete[] mnem;
  return 0;
}
