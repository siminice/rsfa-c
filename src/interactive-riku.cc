#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *dow[] = {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char **club;
char **mnem;
int  NC;
int  id[30], win[30], drw[30], los[30], gsc[30], gre[30];
int  pts[30], rank[30], round[30][30], res[30][30];
int  round2[30][30], res2[30][30];
int  n, ppv, tbr, pr1, pr2, rel1, rel2;
char errorstr[100], inputfile[64];

//--------------------------------------

void Load() {
  FILE *f;
  char s[155], *tok[10], *ystr, *name;
  f = fopen("riku.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    if (strchr(s, ',')==NULL) {
      mnem[i] = new char[16];
      club[i] = new char[32];
      memmove(mnem[i], s, 15); mnem[i][15] = 0;
      int k = 15; while (mnem[i][k-1]==' ') k--; mnem[i][k]=0;
      for (int j=0; j<30; j++) club[i][j] = 32;
      memmove(club[i], s+15, 30);
    }
    else {
      tok[0] = strtok(s, ",");
      tok[1] = strtok(NULL, "\n");
      mnem[i] = new char[strlen(tok[0])+1];
      club[i] = new char[strlen(tok[1])+1];
      strcpy(mnem[i],tok[0]);
      strcpy(club[i],tok[1]);
    }
  }
  fclose(f);
}

int GetName(char *s) {
  strcpy(errorstr, "");
  char *t = s;
//  int k=0; while (s[k]==' ') k++;
//  t = strtok(s+k," ");
  for (int i=0; i<n; i++) {
//    if (strstr(mnem[id[i]],t)==mnem[id[i]]) return i;
    if (strcmp(mnem[id[i]],t)==0) return i;
  }
  sprintf(errorstr, "ERROR: cannot identify %s.", t);
  return -1;
}

int GetMonth(char *s) {
  for (int i=1; i<=12; i++) {
    if (strcmp(s, month[i])==0) return i;
  }
  return 0;
}

int GetDow(char *s) {
  for (int i=1; i<=7; i++) {
    if (strcmp(s, dow[i])==0) return i;
  }
  return 0;
}

int AddResult(int a, int b, int x, int y) {
  gsc[a] += x; gre[a] += y;
  gsc[b] += y; gre[b] += x;
  if (x>y) { win[a]++; los[b]++; pts[a] += ppv; }
    else if (x==y) { drw[a]++; drw[b]++; pts[a]++; pts[b]++; }
     else { los[a]++; win[b]++; pts[b] += ppv; }
  return 1;
}

int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n"); return 0; }
  // Loading file
  char s[100];
  fgets(s, 100, f);
  sscanf(s, "%d %d %d %d %d %d %d", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 100, f);
    sscanf(s, "%d", id+i);
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
//      fscanf(f, "%d %d", &r, &z);
      round[i][j] = -1;// r;
      res[i][j]   = -1;// z;
      round2[i][j] = -1;// r;
      res2[i][j]   = -1;// z;
    }
    fscanf(f, "\n");
  }
  fclose(f);
}

int Extract(char *filename) {
  FILE *f;
  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n"); return 0; }
  // Loading file
  char s[100];
  int r = 1;
  int g = 0;
  int w = 0;
  int m, d, a, b, x, y;
  int r1, m1, w1, d1;
  int **pld = new int*[n];
  for (int i=0; i<n; i++) {
    pld[i] = new int[(n-1)*2];
    for (int j=0; j<(n-1)*2; j++) pld[i][j]=0;
  }
  char *tok[20];
  do {
    fgets(s,100,f);
  } while (strstr(s, "<HR>")==NULL && strstr(s, "<hr>")==NULL);
  do {
    fgets(s,100,f);
    a = b = -2;
    if (strlen(s) > 15 && s[0]!='<') { // not an empty line
     // date + game on same line
      int k = 0;
      tok[0] = strtok(s,", \t");
      tok[1] = strtok(NULL,", ");
      tok[2] = strtok(NULL,", \t");
      for (int i=3; i<15; i++) tok[i] = strtok(NULL, " \t");
      if (tok[0]) w1 = GetDow(tok[0]);
      if (tok[1]) d1 = atoi(tok[1]);
      if (tok[2]) m1 = GetMonth(tok[2]);
      if (w1) {
        k = 4; r = r1; w = w1; d = d1; m = m1;
      } 
      else k = 0;
      char *home, *guest;
      home  = new char[100];
      guest = new char[100];
      if (tok[k]) strcpy(home,tok[k]); else strcpy(home, "");
      k++;
      while (home!=NULL && k<10 && GetName(home)<0 && tok[k]!=NULL) {
        home = strcat(home, " ");
        home = strcat(home, tok[k]);
        k++;
      }
      if (tok[k]) x = atoi(tok[k]);
      if (tok[k+2]) y = atoi(tok[k+2]);
      k += 3;
      if (tok[k]) strcpy(guest, tok[k]); else strcpy(guest, "");
      k++;
      while (guest!=NULL && k<20 && tok[k]!=NULL && GetName(guest)<0) {
        guest = strcat(guest, " ");
        guest = strcat(guest, tok[k]);
        k++;
      }
      a = GetName(home);
      b = GetName(guest);
      s[0] = 0;
    }
    if (a==-1)
      printf("ERROR: Cannot identify home in %s %s %s.\n", tok[0], tok[1], tok[2]);
    else if (b==-1)
      printf("ERROR: Cannot identify guest in %s %s %s.\n", tok[0], tok[1], tok[2]);
    else if (a>=0 && b>=0) {
      g++;
      r=2*(n-1); while (r>1 && pld[a][r-1]+pld[b][r-1]>0) r--;
      if (r<n-1 && round[b][a]>=0) r = round[b][a]/1000 - (n-1);
      if (r<=0) r=1;
      if (res[a][b]<0) {
        round[a][b] = 1000*r+50*m+d;
        res[a][b] = 100*x+y;
        pld[a][r-1]=1; pld[b][r-1]=1;
        printf("[Round %2d, %2d %s] %-15s %d-%d %s\n", 
                 r, d, month[m], mnem[id[a]], x, y, mnem[id[b]]);
        AddResult(a,b,x,y);
      }
      else if (tbr>=10 && res2[a][b]<0) {
        round2[a][b] = 1000*r+50*m+d;
        res2[a][b] = 100*x+y;
        pld[a][r-1]=1; pld[b][r-1]=1;
        printf("[Round %2d, %2d %s] %-15s %d-%d %s\n", 
                 r, d, month[m], mnem[id[a]], x, y, mnem[id[b]]);
        AddResult(a,b,x,y);
      }
      else if (res[a][b]>=0 && tbr<10) {
        printf("[Round %2d, %2d %s] %-15s %d-%d %s\n", 
                 r, d, month[m], mnem[id[a]], x, y, mnem[id[b]]);
        printf(" *** ERROR: already played:\n");
        printf(" *** [Round %2d, %s %d] %-15s %d-%d %s\n",
              round[a][b]/1000, month[(round[a][b]%1000)/50], round[a][b]%50, mnem[id[a]], res[a][b]/100, res[a][b]%100, mnem[id[b]]);
        printf("[Round %2d, %2d %s] %-15s %d-%d %s\n", 
                 r, d, month[m], mnem[id[a]], x, y, mnem[id[b]]);
      }
      else if (res[a][b]>=0 && res2[a][b]>=0) {
        printf("[Round %2d, %2d %s] %-15s %d-%d %s\n", 
                 r, d, month[m], mnem[id[a]], x, y, mnem[id[b]]);
        printf(" *** ERROR: already played twice:\n");
        printf(" *** [Round %2d, %s %d] %-15s %d-%d %s\n",
              round[a][b]/1000, month[(round[a][b]%1000)/50], round[a][b]%50, mnem[id[a]], res[a][b]/100, res[a][b]%100, mnem[id[b]]);
        printf(" *** [Round %2d, %s %d] %-15s %d-%d %s\n",
              round2[a][b]/1000, month[(round[a][b]%1000)/50], round2[a][b]%50, mnem[id[a]], res2[a][b]/100, res2[a][b]%100, mnem[id[b]]);
      }
    }
  } while (!feof(f) && !strstr(s,"</pre>"));
  fclose(f);
  delete[] pld;
}

void Dump(char *inputfile) {
  FILE *f;
  int i, j;
  // fix min round
  int minr = 100;
  for (i=0; i<n; i++) for (j=0; j<n; j++) 
    if (round[i][j]>=0 && round[i][j]/1000<minr) minr = round[i][j]/1000;
  for (i=0; i<n; i++) for (j=0; j<n; j++) 
    if (round[i][j]>=0) round[i][j] = 1000*(round[i][j]/1000 - minr + 1) + round[i][j]%1000;
  
  char *outputfile = new char[strlen(inputfile)+5];
  strcpy(outputfile, inputfile);
  strcat(outputfile, ".old");
  rename(inputfile, outputfile);
  f = fopen(inputfile, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<n; i++)
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d \n",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", round[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++)
        fprintf(f, "%6d%5d", round2[i][j], res2[i][j]);
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  printf("Usage: riku <a.1999> <games.txt>\n");
  if (argc < 2) { printf("Not all input files specified.\n"); return 1; }
  if (argc > 2) {
    strcpy(inputfile, argv[2]);
  }
  else sprintf(inputfile, "html/%s", argv[1]);
  LoadFile(argv[1]);
  Extract(inputfile);
  Dump(argv[1]);
  for (int i=0; i<NC; i++) delete[] mnem[i];
  delete[] mnem;
  return 0;
}
