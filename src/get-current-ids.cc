#include <stdio.h>
#include <string.h>
#include "catalog.hh"

int main(int argc, char **argv) {
  Catalog P;
  P.Load("players.dat");
  if (argc < 2) {
    fprintf(stderr, "ERROR: No edition specified.\n");
    return -1;
  }
  char filename[128];
  sprintf(filename, "ncatalog-%s.dat", argv[1]);
  FILE *f = fopen(filename, "rt");
  FILE *fp = fopen("current-pids", "wt");
  FILE *ft = fopen("current-tids", "wt");
  if (f==NULL || fp==NULL || ft==NULL) {
    fprintf(stderr, "ERROR: Catalog %s not found.\n", filename);
    return -2;
  }
  char s[1000], *tk[20];
  while (!feof(f)) {
    fgets(s, 1000, f);
    if (strlen(s)<20) continue;
    tk[0]=strtok(s, ",\n");
    for (int j=1; j<6; j++) tk[j] = strtok(NULL, ",\n");
    int p = P.binFindMnem(tk[0]);
    if (p>=0) fprintf(fp, "%d\n", p);
    s[0] = 0;
  }
  fclose(f);
  fclose(fp);
  fclose(ft);
}
