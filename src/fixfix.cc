#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAD_TO_HEAD 0

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char **club;
char **mnem;
int  NC;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64];
int  pts[64], pen[64], pdt[64], rank[64], _round[64][64], res[64][64], allres[64][64];
int  _round2[64][64], res2[64][64], allres2[64][64];
int  day[12], home[12], 
guest[12];
int  n, r, lastr, tbr, pr1, pr2, rel1, rel2, ppv, year;
  int _vis[64][64];	// true-false: team i played in this rnd
  int numg[650];	// number of games played this day
  int rnd[64][64];	// games played by team i in round r
  int opp[64][64];	// opponent of team i in round r
  int cur_ng[64];
  int isr[650];

  int all_p[64][64];	// new method
  int cur_p[64];
  int new_p[64];
  int p_count[64];
  int new_count[64];
  int n_p, new_n_p;
  int cur_rnd;
char desc[64][64];
int fd;
  
int precz(int d) {
  if (d%50!=1) return d-1;
  if (d==1) return 631;
  int m = d/50;
  if (m==3) return 128;
  if (m==1 || m==2 || m==4 || m==6 || m==8 || m==9 || m==11) return (m-1)*50+31;
  return (m-1)*50+30;
}

int succz(int d) {
  if (d==81) return 101; 
  if (d==128 || d==129) return 151;
  if (d==181) return 201;
  if (d==230) return 251;
  if (d==281) return 301;
  if (d==330) return 351;
  if (d==381) return 401;
  if (d==431) return 451;
  if (d==480) return 501;
  if (d==531) return 551;
  if (d==580) return 601;
  if (d==631) return 51;
  else return d+1;
}

int NumGamesAroundDate(int d) {
  int pd1 = precz(d);
  int pd2 = precz(pd1);
  int sd1 = succz(d);
  int sd2 = succz(sd1);
  int npd2 = numg[pd2];
  int npd1 = numg[pd1];
  int nd = numg[d];
  int nnd1 = numg[sd1];
  int nnd2 = numg[sd2];
  return pd1+pd2+sd1+sd2;
}


//--------------------------------------

void Load() {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    mnem[i] = new char[16];
    club[i] = new char[64];
    memmove(mnem[i], s, 15); mnem[i][15] = 0;
    for (int j=0; j<30; j++) club[i][j] = 32;
    memmove(club[i], s+15, 30);
  }
  fclose(f);
}

   
int LoadFile(char *filename) {
  FILE *f;
  int i, j, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  char s[200], *tok[12];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i] = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      _round[i][j] = r;
      res[i][j] = z;
      _round2[i][j] = -1;
      res2[i][j] = -1;
    }
    fscanf(f, "\n");
  }
  fclose(f);

  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        _round2[i][j] = r;
        res2[i][j] = z;
      }
      fscanf(f, "\n");
    }
  }
}

int Collect() {
  // initialiaze
  for (int i=0; i<64; i++) for (int j=0; j<64; j++) { _vis[i][r] = 0; opp[i][r]=-1; rnd[i][r] = 0; }
  for (int i=0; i<650; i++) { numg[i] = 0; }

  // collect
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (res[i][j]>=0) {
        int r = _round[i][j]/1000;
        int z = _round[i][j]%1000;
        if (r>0 && r<64) {
          _vis[i][r] = 1;
          _vis[j][r] = 1;
          opp[i][r] = j;
          opp[j][r] = i;
          if (r+1<64) { rnd[i][r+1]++; rnd[j][r+1]++; _vis[i][r]++; _vis[j][r]++;}
        }
        else if (r==0) { rnd[i][1]++; rnd[j][1]++; }
        else if (r<0)  { rnd[i][0]++; rnd[j][0]++; }
        if (z>0) {
          numg[z]++; 
        }
      }
      if (res2[i][j]>=0) {
        int r = _round2[i][j]/1000;
        int z = _round2[i][j]%1000;
        if (r>0 && r<64) {
          _vis[i][r] = 1;
          _vis[j][r] = 1;
          opp[i][r] = j;
          opp[j][r] = i;
          if (r+1<64) { rnd[i][r+1]++; rnd[j][r+1]++; _vis[i][r]++; _vis[j][r]++; }
        }
        else if (r==0) { rnd[i][1]++; rnd[j][1]++; }
        else if (r<0)  { rnd[i][0]++; rnd[j][0]++; }
        if (z>0) {
          numg[z]++; 
        }
      }
    }
  }

}

int CheckFixtures(bool v) {

  Collect();

  int err = 0;
  // check

  for (int i=0; i<n; i++) {
    printf("%-16s:", mnem[id[i]]);
    for (int j=0; j<64; j++) {
      printf("%2d ", rnd[i][j]);
    }
    printf("\n");
  }

  for (int i=0; i<n; i++) {
    for (int r=1; r<=2*(n-1); r++) 
      if (opp[i][r]<0) {
        if (v) printf("ERROR: Opponent of %s in round %d unknown.\n", mnem[id[i]], r);
        err++;
      }
    for (int j=0; j<n; j++) 
      if (i!=j)
      if (_round[i][j]/1000<1 || _round[i][j]/1000>2*(n-1)) {
        if (v) printf("ERROR: Incorrect round for %s versus %s: %d.\n", mnem[id[i]], mnem[id[j]], _round[i][j]/1000);
        err++;
      }
  }

}

/*
int isRound() {
  int count = 0;
  int z, z1, z2;
  z = 50;
  z1 = succz(z);
  z2 = succz(z1);
  for (int i=0; i<650; i++) isr[i] = 0;
  printf("\n===================\nRound dates detected:\n");
  do {
    if (numg[z]>0) {
      if (numg[z]>=n/3) {
        isr[z] = 1; count++; 
        printf("%3d ", z);
        if (z>=630) z1 = 0;
        z = z2;
      }
      else if (numg[z]+numg[z1]+numg[z2]>n/3 && !Conflicts(z, z1, z2)) { 
        isr[z] = 1; count++; 
        printf("%3d ", z);
        if (z>=630) z1 = 0;
        else if (numg[z]+numg[z1]<=n/3) z = succz(z2);
        else if (numg[z]<=n/3 || numg[z1]>0) z = z2;
        else z = z1;
      }
      else z = z1;
    }
    else {
      if (z==631) z1 = 0;
      else z = z1;
    }
    if (z1>0) z1 = succz(z);
    if (z1>0) z2 = succz(z1);
  } while (z1>0);
  printf("\n==================\nCounted %d rounds...\n", count);
}
*/

int FindRnd(int i, int j, int r) {
  int avail[64];
  for (int h=0; h<64; h++) avail[h] = 1;
  for (int k=1; k<=r+1; k++) {
    if (_vis[i][k]>0) avail[k] = 0;
    if (_vis[j][k]>0) avail[k] = 0;
  }
  int r1;
  for (int k=r+1; k>0; k--) {
    if (avail[k]) r1 = k;
  }
  return r1;
}

int PrintNg() {
  printf("\n");
  for (int i=0; i<n; i++)
   printf("%d ", cur_ng[i]);
  printf("\n"); 
}


int GetGames(int z) {
  int k = 0;
  for (int i=0; i<64; i++) { new_count[i] = 0; }
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      if (res[i][j]>=0 && _round[i][j]%1000==z) {
        new_p[k] = i;
        new_p[k+1] = j;
        new_count[i]++;
        new_count[j]++;
        k+=2;
      }
      if (res2[i][j]>=0 && _round2[i][j]%1000==z) {
        cur_p[k] = i;
        cur_p[k+1] = j;
        new_count[i]++;
        new_count[j]++;
        k+=2;
      }
    }
  }
  return k;
}


void AddToRound() {
  for (int k=0; k<new_n_p; k++) {
    int x = new_p[k];
    cur_p[n_p+k] = x;
    p_count[x] += new_count[x];
  }  
  n_p += new_n_p;
}

void SubFromRound() {
  for (int k=0; k<new_n_p; k++) {
    int x = new_p[k];
    p_count[x] -= new_count[x];
  }  
  n_p -= new_n_p;
}

int Conflicts() {
  for (int i=0; i<n_p; i++) 
    if (p_count[i]>1) return 1;
  return 0;
}

void DumpNewRound() {
  cur_rnd++;
  for (int k=0; k<new_n_p; k++) {
    int x = new_p[k];
    all_p[cur_rnd][k] = x;
    _vis[x][cur_rnd]++;
    cur_ng[x]++;
  }
  for (int k=0; k<new_n_p; k+=2) {
    int x = new_p[k];
    int y = new_p[k+1];
    int r = res[x][y];
    int z = _round[x][y]%1000;
    printf("[R%2d, %3d] %-16s-%-16s %d-%d\n", 
     cur_rnd, z, mnem[id[x]], mnem[id[y]], r/100, r%100);
     _round[x][y] = 1000*cur_rnd+z;
  }
  for (int k=0; k<64; k++) { p_count[k] = 0; }
  n_p = 0;
  PrintNg();
}

void DumpPartial() {
  int r = 0;
  if (n_p > n/2) r = ++cur_rnd;
  for (int k=0; k<n_p; k+=2) {
    int x = cur_p[k];
    int y = cur_p[k+1];
    int s = res[x][y];
    int z = _round[x][y]%1000;
    if (n_p <= n/2)  r = FindRnd(x, y, cur_rnd);
    if (r>0) {
      printf("[R%2d, %3d] %-16s-%-16s %d-%d\n", 
       r, z, mnem[id[x]], mnem[id[y]], s/100, s%100);
      cur_ng[x]++;
      cur_ng[y]++;
      _vis[x][r]++;
      _vis[y][r]++;
      _round[x][y] = 1000*r+z;
    }
    else {
      printf("[R??, %3d] %-16s-%-16s %d-%d not determined\n", 
       z, mnem[id[x]], mnem[id[y]], s/100, s%100);
    }
  }
  for (int k=0; k<n_p; k++) {
    p_count[cur_p[k]]--;
  }
  n_p = 0;
  PrintNg();
}

void SuggestFixtures() {
  for (int i=0; i<64; i++) {
    p_count[i] = 0; cur_p[i] = 0; 
    cur_ng[i] = 0;
    for (int j=0; j<64; j++) { 
      _vis[i][j] = 0;
      all_p[j][i] = 0;
    }
  }
  n_p = 0;
  cur_rnd = 0;
  for (int d=fd; d<fd+649; d++) {
    int z = d%650;
    new_n_p = GetGames(z);
    if (new_n_p == 0) continue;
    if (new_n_p==2*(n/2)) { // full round
      if (n_p >0) DumpPartial();
      DumpNewRound();
    }
    else {
      AddToRound();
      if (Conflicts()) {
        SubFromRound();
        DumpPartial();
        AddToRound();
      }
      else if (n_p==2*(n/2)) {
        DumpPartial();
     }
    }
  }


  for (int i=1; i<=2*(n-1); i++) {
    int nv = 0;
    int xx1, xx2;
    for (int j=0; j<n; j++) {
      if (_vis[j][i]==0) {
        printf("ERROR: %s has not played in round %d.\n", mnem[id[j]], i);
        nv++;
        if (nv==1) xx1 = j;
        if (nv==2) xx2 = j;
      }
    }
    if (nv == 2) {
      int r1 = -1;
      int r2 = -1;
      if (_round[xx1][xx2]>=0) r1 = _round[xx1][xx2]/1000;
      if (_round[xx2][xx1]>=0) r2 = _round[xx2][xx1]/1000;
      int z1 = -1;
      int z2 = -1;
      if (_round[xx1][xx2]>=0) z1 = _round[xx1][xx2]%1000;
      if (_round[xx2][xx1]>=0) z2 = _round[xx2][xx1]%1000;
      if (numg[z1]==1) {
        _round[xx1][xx2] = 1000*i+z1;
        printf("EASY FIX in round %d: only teams not scheduled:  %s and %s.\n", i, club[id[xx1]], club[id[xx2]]);
        printf("  moved from round %d.\n", r1);
        rnd[xx1][i+1]++;
        rnd[xx1][r1+1]--;
        rnd[xx2][i+1]++;
        rnd[xx2][r1+1]--;
      }
      else if (numg[z2]==1) {
        _round[xx2][xx1] = 1000*i+z2;
        printf("EASY FIX in round %d: only teams not scheduled:  %s and %s.\n", i, club[id[xx2]], club[id[xx1]]);
        printf("  moved from round %d.\n", r2);
        rnd[xx2][i+1]++;
        rnd[xx2][r2+1]--;
        rnd[xx1][i+1]++;
        rnd[xx1][r2+1]--;
      }
    }
  }

}

void Dump(char *inputfile) {
  FILE *f;
  int i, j;
  char outputfile[100];
  strcpy(outputfile, inputfile);
  strcat(outputfile, ".old");
  rename(inputfile, outputfile);
  f = fopen(inputfile, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, pr1, pr2, rel1, rel2);
  for (i=0; i<n; i++) {
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
    if (pen[i]!=0) fprintf(f, "%4d%4d", pen[i], pdt[i]);
    if (strlen(desc[i])>0) {
      if (pen[i]==0)  fprintf(f, "%4d%4d", pen[i], pdt[i]);
      fprintf(f, " %s", desc[i]);
    }
    fprintf(f, "\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", _round[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++)
        fprintf(f, "%6d%5d", _round2[i][j], res2[i][j]);
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

//---------------------------------------------

int main(int argc, char* argv[]) {
  Load();  
  if (argc < 2) { printf("No input file specified.\n"); return 1; }
  fd = 365;
  for (int i=2; i<argc; i++)
    if (strcmp(argv[i], "-fd")==0 && i+1<argc) fd = atoi(argv[i+1]);
  LoadFile(argv[1]);
  CheckFixtures(false);
  SuggestFixtures();
  int err = CheckFixtures(true);
  SuggestFixtures();
//  Dump(argv[1]);
  return 0;
}
