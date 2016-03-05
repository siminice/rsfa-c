#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int  id[30], win[30], drw[30], los[30], gsc[30], gre[30], pts[30], pen[30], pdt[30];
int  lid[30], lwin[30], ldrw[30], llos[30], lgsc[30], lgre[30], lpts[30], lpen[30], lpdt[30];
int  rank[30], round[30][30], round2[30][30], res[30][30], res2[30][30];
int  n, r, lastr, tbr, pr1, pr2, rel1, rel2, ppv, year;
int  bonus;

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
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        round2[i][j] = r;
        res2[i][j] = z;
      }
      fscanf(f, "\n");
    }
  }
  fclose(f);
}

void Dump(char *filename) {

  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
    win[i] = drw[i] = los[i] = gsc[i] = gre[i] = pts[i] = 0;
   lwin[i] =ldrw[i] =llos[i] =lgsc[i] =lgre[i] =lpts[i] = 0;
    }
  }

  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (res[i][j] >=0) {
         int x = res[i][j]/100;
         int y = res[i][j]%100;
         gsc[i] += x; gre[i] += y;
         gsc[j] += y; gre[j] += x;
         if (x>y) { win[i]++; los[j]++; pts[i]+=ppv;}
          else if (x==y) { drw[i]++; drw[j]++; pts[i]++; pts[j]++;}
           else { los[i]++; win[j]++; pts[j]+=ppv;}
      }
      if (res2[i][j] >=0) {
         int x = res2[i][j]/100;
         int y = res2[i][j]%100;
         lgsc[i] += x; lgre[i] += y;
         lgsc[j] += y; lgre[j] += x;
         if (x>y) { lwin[i]++; llos[j]++; lpts[i]+=ppv;}
          else if (x==y) { ldrw[i]++; ldrw[j]++; lpts[i]++; lpts[j]++;}
           else { llos[i]++; lwin[j]++; lpts[j]+=ppv;}
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
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], ppv*win[i]+drw[i]);
    if (pen[i]>0) fprintf(f, "%4d %4d\n", pen[i], pdt[i]);
    else fprintf(f, "\n");
  }
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++)
      fprintf(f, " %5d %5d", round[i][j], res[i][j]);
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
      id[i], lwin[i]+ldrw[i]+llos[i], lwin[i], ldrw[i], llos[i], lgsc[i], lgre[i], ppv*lwin[i]+ldrw[i]);
    if (bonus) fprintf(f, " %d", -pts[i]);
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
  bonus = 0;
  if (argc < 2) { printf("No input file specified.\n"); return 1; }
  for (int j=2; j<argc; j++) {
    if (strcmp(argv[j], "-")==0) bonus = 0;
    if (strcmp(argv[j], "+")==0) bonus = 1;
  }
  LoadFile(argv[1]);
  Dump(argv[1]);
  return 0;
}
