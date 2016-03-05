
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void Sort(char *dbfile) {
 FILE *f;
 int M;
 char **data;
 char s[120];
 int *perm;

  f = fopen(dbfile, "rt");
  if (!f) {
    printf("ERROR: database file %s not found.\n", dbfile);
    exit(0);
  }
  M = 0;
  while(!feof(f)) {
    fgets(s, 100, f);
    if (strlen(s) > 2) M++;
    s[0] = 0;
  }
  fclose(f);

  data = new char*[M];
  perm = new int[M];

  f = fopen("unsorted.archive.dat", "rt");
  for (int j=0; j<M; j++) {
    fgets(s, 100, f);
    int len = strlen(s);
    s[len-1] = 0;
    data[j] = new char[len+1];
    strcpy(data[j], s);
    perm[j] = j;
  }
  fclose(f);
  printf("Loaded %d official games.\n", M);
  printf("Sorting...\n");
  int sorted, last, max;
  max = M-1;
  int pass = 0;
  char *aux = new char[100];
  int p;
  do {
    sorted = 1;
    last = max;
    max = 0;
    for (int i=0; i<last; i++) {
      if (strncmp(data[perm[i]], data[perm[i+1]], 4)>0) {
        sorted = 0;
        p = perm[i];
        perm[i] = perm[i+1];
        perm[i+1] = p;
//        strcpy(aux, data[i]);
//        strcpy(data[i], data[i+1]);
//        strcpy(data[i+1], aux);
        max = i;
      }
    }
    printf("\r%d", ++pass);
  } while(!sorted);
  delete aux;
  f = fopen("archive.dat", "wt");
  for (int i=0; i<M; i++)
    fprintf(f, "%s\n", data[perm[i]]);
  fclose(f);
  printf("\n");
  delete[] perm;
  delete[] data;
};

//--------------------------------------------

int main(int argc, char** argv) {
 char dbf[100];
 strcpy(dbf, "unsorted.archive.dat");
 if (argc > 1) strcpy(dbf, argv[1]);
 Sort(dbf); 
 return 0;
}
