#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *normal  = "\033[0m";
const char *red     = "\033[31m";
const char *green   = "\033[32m";
const char *yellow  = "\033[33m";

int NC;

char **club;
char **mnem;
int n, ppv, tbr, promo1, promo2, releg1, releg2, id[32], tkn;

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("riku.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  for (int i=0; i<NC; i++) {
    fgets(s, 2000, f);
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, ",");
    tok[1] = strtok(NULL, "\n,");
    int l1 = strlen(tok[0]);
    int l2 = strlen(tok[1]);
    mnem[i] = new char[l1+1];
    club[i] = new char[l2+1];
    strcpy(mnem[i], tok[0]);
    strcpy(club[i], tok[1]);
  }
  fclose(f);
}

int Find(char* s) {
  int found = 0;
  int i = 0;
  int multi = 0;
  int j;

  if (s==NULL) return -1;
  int l = strlen(s);
  if (s[l-1]=='?') s[l-1] = 0;
  for (int i=0; i<NC; i++)
    if (strcmp(mnem[i],s)==0) return i;
  for (int i=0; i<NC; i++)
    if (strstr(mnem[i],s)==mnem[i]) return i;
  return -1;
}

int Detect() {
  char s[200], *tok[10];
  int k = 0;
  while (!feof(stdin) && k<n) {
    fgets(s, 200, stdin);
    tok[0] = strtok(s, "\t\n");
    for (int j=1; j<10; j++) tok[j] = strtok(NULL, "\t");

    int i = -1;
    if (tkn>=0 && tkn<10) {
      i = Find(tok[0]);
    }

    if (i>=0) {
      fprintf(stderr, "%s%2d.%-30s [%3d]%s\n", green, k+1, club[i], i, normal);
    }
    else {
      fprintf(stderr, "%s%2d.%-30s = ???%s\n", yellow, k+1, tok[tkn], normal);
    }
    id[k++] = i;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  FILE *f;
  char *s = new char[40];
  int i, j;

  tkn = 1;
  ppv = 3; tbr = 1;
  promo1 = 3; promo2 = 0;
  releg1 = 3; releg2 = 0;

  for (int i=1; i<argc; i++) {
    if (strcmp(argv[i],"-n")==0   && i+1<argc) n      = atoi(argv[i+1]);
    if (strcmp(argv[i],"-k")==0   && i+1<argc) tkn    = atoi(argv[i+1]);
    if (strcmp(argv[i],"-ppv")==0 && i+1<argc) ppv    = atoi(argv[i+1]);
    if (strcmp(argv[i],"-tbr")==0 && i+1<argc) tbr    = atoi(argv[i+1]);
    if (strcmp(argv[i],"-p1")==0  && i+1<argc) promo1 = atoi(argv[i+1]);
    if (strcmp(argv[i],"-p2")==0  && i+1<argc) promo2 = atoi(argv[i+1]);
    if (strcmp(argv[i],"-r1")==0  && i+1<argc) releg1 = atoi(argv[i+1]);
    if (strcmp(argv[i],"-r2")==0  && i+1<argc) releg2 = atoi(argv[i+1]);
  }

  Load();
  Detect();
  printf("%d %d %d %d %d %d %d\n", n, ppv, tbr, promo1, promo2, releg1, releg2);
  for (i=0; i<n; i++)
    printf("%3d   0   0   0   0   0   0   0\n", id[i]);
  int nr = 1+tbr/10;
  for (int r=0; r<nr; r++) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++)
        printf("   -1    -1 ");
      printf("\n");
    }
  }
  return 0;
}
