#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  int longmnem = 0;
  if (argc > 1) {
    longmnem = 1;
  }
  FILE *f = fopen("teams.dat", "rt");
  char s[100], t[100], *tok[3];
  if (f==NULL) {
    printf("ERROR: main file (teams.dat) not found.\n");
    return 1;
  }
  fgets(s, 100, f);
  int n = atoi(s); 
  printf("%d\n", n);
  for (int i=0; i<n; i++) {
    fgets(s, 100, f); 
    s[strlen(s)-1] = 0;
    strcpy(t, s+15);
    s[15] = 0;
    int j = 14;
    while (j>=0 && s[j]==' ') j--;
    s[j+1] = 0;
    if (longmnem) {
      printf("%s,%s\n", t, t);
    }
    else {
      printf("%s,%s\n", s, t);
    }
  }
  fclose(f);
  return 0;
}
