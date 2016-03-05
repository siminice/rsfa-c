#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


int  NC, ND;
char **club;
char **mnem;
int  ****part;
int  *used;
int *FY, *LY, *MAX;

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  int dummy, n, p, t, d, y;
  char *dv, *pl, *yr, *suf;
  char filename[64];

  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  used = new int[NC];

  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[32];
    memmove(mnem[i], s, 15); mnem[i][15] = 0;
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
    used[i] = 0;
  }
  fclose(f);

  part = new int***[ND];
  FY = new int[ND];
  LY = new int[ND];
  MAX = new int[ND];

  for (int d=0; d<ND; d++) {
    FY[d] = 3000;
    LY[d] = 1000;
    MAX[d] = 0;
  }
  DIR *dp;
  struct dirent *ep;
  dp = opendir("./");
  if (dp != NULL) {
      while (ep = readdir (dp)) {
        strcpy(s, ep->d_name);
        dv = strtok(s, ".");
        yr = strtok(NULL, ".");
        suf = strtok(NULL, ".");
        if (dv!=NULL && yr!=NULL && suf==NULL) {
          int l = strlen(dv);
          if (l==0 || l>3) continue;
          d = ((int) s[0]) - 97;
          if (d>=ND) continue;
          if (l>1) p = atoi(dv+1); else p = 1;
          if (p<0 && p>12) continue;
          y = atoi(yr);
          if (d>=0 && y>1000 && y<3000) {
            if (y<FY[d]) FY[d] = y;
            if (y>LY[d]) LY[d] = y;
            if (p>MAX[d]) MAX[d] = p;
          }
        }
      }
      closedir(dp);
      for (int d=0; d<ND; d++)
        printf("%c: %d - %d (max: %d)\n", (char) (d+65), FY[d], LY[d], MAX[d]);
  }
  else
   printf("ERROR: Couldn't open the directory.\n");

  for (int d=0; d<ND; d++) {
    if (MAX[d]>0) {
      part[d] = new int**[LY[d]-FY[d]+1];
      for (int i=FY[d]; i<=LY[d]; i++) {
        part[d][i-FY[d]] = new int*[MAX[d]+1];
        for (int j=0; j<=MAX[d]; j++) {
          part[d][i-FY[d]][j] = new int[30];
        }
      }
    }
  }

// quick data
  for (int d=0; d<ND; d++) {
    sprintf(filename, "part.%c", (char) (d+97));
    f = fopen(filename, "rt");
    for (int y=0; y<=LY[d]-FY[d]; y++) {
      if (f==NULL) part[d][y][0][1] = 0;
      else {
        fscanf(f, "%d %d", &dummy, &n);
        part[d][y][0][0] = dummy;
        part[d][y][0][1] = n;
        for (int i=0; i<n; i++) {
          fscanf(f, "%d", &t); part[d][y][0][i+2] = t;
          used[t] = 1;
        }
        fgets(s, 200, f);
      }
    }
    if (f) fclose(f);
  }

  for (int d=0; d<ND; d++) {
    for (int i=1; i<=MAX[d]; i++) {
      sprintf(filename, "part.%c%d", (char) (d+97), i);
      f = fopen(filename, "rt");
      for (int y=0; y<=LY[d]-FY[d]; y++) {
        if (f==NULL) part[d][y][i][1] = 0;
        else {
          fscanf(f, "%d %d", &dummy, &n);
          part[d][y][i][0] = dummy;
          part[d][y][i][1] = n;
          for (int j=0; j<n; j++) {
            fscanf(f, "%d", &t);
            part[d][y][i][j+2] = t;
            used[t] = 1;
          }
          fgets(s, 200, f);
        }
      }
      if (f) fclose(f);
    }
  }
}

int CupData() {
  char s[128];
  FILE *f = fopen("cuparchive.dat", "rt");
  if (f==NULL) return 0;
  while (!feof(f)) {
    fgets(s, 128, f);
    if (strlen(s) > 10) {
      int home     = 75*((int)(s[7]-48)) + s[8] - 48;
      int guest    = 75*((int)(s[9]-48)) + s[10] - 48;
      used[home] = 1;
      used[guest] = 1;
    }
    s[0] = 0;
  }
  fclose(f);
  return 1;
}


//---------------------------------------------

int main(int argc, char* argv[]) {
  ND = 4;
  int cup = 0;

  for (int k=1; k<argc; k++) {   
    if (strcmp(argv[k], "-nd")==0) {
      if (k+1<argc) ND = atoi(argv[k+1]);
      if (ND<2 || ND>8) ND = 4;
    }
    if (strcmp(argv[k], "-c")==0) {
      cup = 1;
    }
  }

  Load();
  
 if (cup) CupData(); 

  int count = 0; 
  printf("Unused ids:\n");
  for (int i=0; i<NC; i++) {
    if (used[i]==0) {
      printf("%d.%s\n", i, mnem[i]);
      count++;
    }
  }

  printf("Total: %d\n", count);
  return 0;
}
