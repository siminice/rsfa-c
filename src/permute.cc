#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **club;
char **mnem;
int  NC;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pts[64], pen[64], pdt[64];
int  round[64][64], res[64][64];
int  perm[64];
char desc[64][32];
int  n, r, tbr, pr1, pr2, rel1, rel2, ppv;

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

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[20];
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
      round[i][j] = r;
      res[i][j] = z;
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

void Dump() {
  int i, j, k, l;
  printf("%d %d %d %d %d %d %d\n", n, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<n; i++) {
    k = perm[i];
    printf("%4d%4d%4d%4d%4d%4d%4d%4d",
      id[k], win[k]+drw[k]+los[k], 
      win[k], drw[k], los[k], 
      gsc[k], gre[k], pts[k]);
    if (pen[k]!=0) printf("%4d%4d", pen[k], pdt[k]);
    if (strlen(desc[k])>0) {
      if (pen[k]==0)  printf("%4d%4d", pen[k], pdt[k]);
      printf(" %s", desc[k]);
    }
    printf("\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      printf("%6d%5d", round[perm[i]][perm[j]], res[perm[i]][perm[j]]);
    printf("\n");
  }
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  if (argc < 3) { printf("Usage: permute file i1 i2 ... in\n"); return 1; }
  LoadFile(argv[1]);
//  if (argc != n+2) { printf("Usage: permute file i1 i2 ... in\n"); return 1; }
  for (int k=0; k<n; k++) perm[k]=k;
  for (int k=2; k<argc; k++) {
    perm[k-2] = atoi(argv[k]);
  }
  Dump();
  return 0;
}
