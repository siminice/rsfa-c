#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "catalog.hh"

char ***ldb;
char ***edb;
int NM;

void LoadDB(int year, int comp) {
  char filename[64], s[5000], *tk[60];
  FILE *f;
    switch (comp) {
        case 1:
            sprintf(filename, "/var/www/html/rom/cup-lineups-%d.db", year);
            break;
        case 2:
            sprintf(filename, "/var/www/html/rom/euro-lineups-%d.db", year);
            break;
        case 3:
            sprintf(filename, "/var/www/html/rom/nat-lineups-%d.db", year);
            break;
        default:
          sprintf(filename, "/var/www/html/rom/lineups-%d.db", year);
    }
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stdout, "ERROR: database %s not found.\n", filename); return; }
  ldb = new char**[400];
  int i = 0;
  while (!feof(f)) {
    fgets(s, 5000, f);
    if (strlen(s)<100) continue;
    tk[0] = strtok(s, ",\n");
    ldb[i] = new char*[60];
    for (int j=1; j<60; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<60; j++) {
      ldb[i][j] = new char[30];
      if (tk[j]!=NULL) strcpy(ldb[i][j], tk[j]);
      else strcpy(ldb[i][j], " ");
    }
    i++;
    s[0] = 0;
  }
  NM = i;
  fclose(f);
  fprintf(stdout, "NM = %d.\n", NM);
}

void LoadEvents(int year, int comp) {
  char filename[64], s[5000], *tk[60];
  FILE *f;
    switch (comp) {
        case 1:
            sprintf(filename, "/var/www/html/rom/cup-events-%d.db", year);
            break;
        case 2:
            sprintf(filename, "/var/www/html/rom/euro-events-%d.db", year);
            break;
        case 3:
            sprintf(filename, "/var/www/html/rom/nat-events-%d.db", year);
            break;
        default:
          sprintf(filename, "/var/www/html/rom/events-%d.db", year);
    }
  f = fopen(filename, "rt");
  if (f==NULL) { fprintf(stdout, "ERROR: database %s not found.\n", filename); return; }
  edb = new char**[400];
  int i = 0;
  while (!feof(f)) {
    fgets(s, 5000, f);
    tk[0] = strtok(s, ",\n");
    edb[i] = new char*[60];
    for (int j=1; j<60; j++) tk[j]=strtok(NULL, ",\n");
    for (int j=0; j<60; j++) {
      edb[i][j] = new char[30];
      if (tk[j]!=NULL) strcpy(edb[i][j], tk[j]);
      else strcpy(edb[i][j], " ");
    }
    i++;
    s[0] = 0;
  }
  fclose(f);
}

int main(int argc, char **argv) {
  if (argc<2) return 1;
  int y = atoi(argv[1]);
  int comp = 0;
  if (argc>2) comp = atoi(argv[2]);
  Catalog *C = new Catalog();
  C->Load("/var/www/html/rom/players.dat");
  PlayerStats *S = new PlayerStats(C, 0);
  LoadDB(y, comp);
  LoadEvents(y, comp);
  S->extract(ldb, edb, NM);
  S->print();
}
