#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  id[30];
int  win[30], drw[30], los[30], gsc[30], gre[30], pts[30], pen[30], pdt[30];
int  win1[30], drw1[30], los1[30], gsc1[30], gre1[30], pts1[30], pen1[30], pdt1[30];
int  win2[30], drw2[30], los2[30], gsc2[30], gre2[30], pts2[30], pen2[30], pdt2[30];
int  rank[30], round[30][30], round1[30][30], round2[30][30], res[30][30], res1[30][30], res2[30][30];
int  n, r, lastr, tbr, pr1, pr2, rel1, rel2, ppv, year;

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[500], *tok[10];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<10; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = drw[i] = los[i] = 0;
    gsc[i] = gre[i] = pts[i] = 0;
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      round[i][j] = r;
      res[i][j] = z;
      if (round[i][j]/1000 < n) {
        round1[i][j] = r;
        round2[i][j] = -1;
        res1[i][j] = z;
        res2[i][j] = -1;
      }
      else {
        round2[i][j] = r;
        round1[i][j] = -1;
        res2[i][j] = z;
        res1[i][j] = -1;
      }
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

void Dump(char *filename) {

  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      win1[i] = drw1[i] = los1[i] = gsc1[i] = gre1[i] = pts1[i] = 0;
      win2[i] = drw2[i] = los2[i] = gsc2[i] = gre2[i] = pts2[i] = 0;
    }
  }

  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (res1[i][j] >=0) {
         int x = res1[i][j]/100;
         int y = res1[i][j]%100;
         gsc1[i] += x; gre1[i] += y;
         gsc1[j] += y; gre1[j] += x;
         if (x>y) { win1[i]++; los1[j]++; pts1[i]+=ppv;}
          else if (x==y) { drw1[i]++; drw1[j]++; pts1[i]++; pts1[j]++;}
           else { los1[i]++; win1[j]++; pts1[j]+=ppv;}
      }
      if (res2[i][j] >=0) {
         int x = res2[i][j]/100;
         int y = res2[i][j]%100;
         gsc2[i] += x; gre2[i] += y;
         gsc2[j] += y; gre2[j] += x;
         if (x>y) { win2[i]++; los2[j]++; pts2[i]+=ppv;}
          else if (x==y) { drw2[i]++; drw2[j]++; pts2[i]++; pts2[j]++;}
           else { los2[i]++; win2[j]++; pts2[j]+=ppv;}
      }
    }
  }

  FILE *f;
  char tempf[100];

  sprintf(tempf, "%s.1", filename);
  f = fopen(tempf, "wt");
  if (f==NULL) {
    printf("Cannot open %s\n", tempf);
    return;
  }
  printf("Splitting %s into %s.1 and %s.2\n", filename, filename, filename);
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr%10, pr1, pr2, rel1, rel2);
  for (int i=0; i<n; i++) {
    fprintf(f, "%4d %4d %4d %4d %4d %4d %4d %4d ",
      id[i], win1[i]+drw1[i]+los1[i], win1[i], drw1[i], los1[i], gsc1[i], gre1[i], ppv*win1[i]+drw1[i]);
    if (pen1[i]>0) fprintf(f, "%4d %4d\n", pen1[i], pdt1[i]);
    else fprintf(f, "\n");
  }
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++)
      fprintf(f, " %5d %5d", round1[i][j], res1[i][j]);
    fprintf(f, "\n");
  }
  fclose(f);

  sprintf(tempf, "%s.2", filename);
  f = fopen(tempf, "wt");
  if (f==NULL) {
    printf("Cannot open %s\n", tempf);
    return;
  }
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr%10, pr1, pr2, rel1, rel2);
  for (int i=0; i<n; i++) {
    fprintf(f, "%4d %4d %4d %4d %4d %4d %4d %4d ",
      id[i], win2[i]+drw2[i]+los2[i], win2[i], drw2[i], los2[i], gsc2[i], gre2[i], ppv*win2[i]+drw2[i]);
    fprintf(f, "\n");
  }
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++)
      fprintf(f, " %5d %5d", round2[i][j], res2[i][j]);
    fprintf(f, "\n");
  }
  fclose(f);


}


//---------------------------------------------

int main(int argc, char* argv[]) {
  if (argc < 2) { printf("No input file specified.\n"); return 1; }
  LoadFile(argv[1]);
  Dump(argv[1]);
  return 0;
}
