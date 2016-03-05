#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **club;
char **mnem;
int  NC;
int  id[30], pts[30], pen[30];
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

int PreloadFile(char *filename) {
  FILE *f;
  int i, j;
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found for preloading.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
  }
  fclose(f);
}

void Dump() {
  int i, j;
  printf("%d %d %d %d %d %d %d\n", n, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<n; i++) {
    printf("%4d   0   0   0   0   0   0   0%4d\n", id[i], pen[i]-pts[i]);
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      printf("    -1   -1");
    printf("\n");
  }
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  if (argc < 2) { printf("Usage: copy <filename>\n"); return 1; }
  PreloadFile(argv[1]);
  Dump();
  return 0;
}
