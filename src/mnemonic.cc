#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int NC;

char **club;
char **mnem;

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
  f = fopen("rsssfteams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  for (int i=0; i<NC; i++) {
   fgets(s, 60, f);
   s[strlen(s)-1] = 0;
   mnem[i] = new char[32];
   club[i] = new char[32];
   memmove(mnem[i], s, 15);
   for (int j=0; j<30; j++)
     club[i][j] = 32;
   memmove(club[i], s+15, 30);
  }
  fclose(f);
}

int main(int argc, char* argv[]) {
  Load();
  if (argc < 2) return 1;
  int width = atoi(argv[1]);
  FILE *f;
  char filename[64], s[100];
  sprintf(filename, "mnem%d.dat", width);
  f = fopen(filename, "wt");
  if (f==NULL) return 2;
  fprintf(f, "%d\n", NC);
  for (int i=0; i<NC; i++) {
    memmove(s, mnem[i], 15);
    for (int j=15; j<width; j++) s[j] = ' ';
    memmove(s+width, club[i], 30);
    fprintf(f, "%s\n", s);
  }
  fclose(f);
  return 0;
}
