#include <stdio.h>
#include <string.h>
#include <stdlib.h>


const char *luna[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

const char *fluna[] = {"January", "February", "March", "April", "May", "June",
                 "July", "August", "September", "October", "November", "December"};
char **club;
char **mnem;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64];
int  pts[64], pen[64], pdt[64], rank[64], round[64][64], res[64][64];
int  round2[64][64], res2[64][64];
char inputfile[64], outputfile[64];
int NC, n, ppv, tbr, p1, p2, r1, r2;
char desc[64][32];
int reset;
int df, fd, syntax;

void Load(const char *filename) {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("rsssfteams.dat", "rt");
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

void Dump(char *outfile) {
  FILE *f; 
  int i, j;
  char tempf[100];
  sprintf(tempf, "%s.old", outfile);
  rename(outfile, tempf);
  f = fopen(outfile, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, p1, p2, r1, r2);
  for (i=0; i<n; i++) {
    fprintf(f, "%4d %4d %4d %4d %4d %4d %4d %4d ",
      id[i], win[i]+drw[i]+los[i], win[i], drw[i], los[i], gsc[i], gre[i], ppv*win[i]+drw[i]);
    if (pen[i]!=0) fprintf(f, "%4d %4d", pen[i], pdt[i]);
    if (strlen(desc[i])>0) {
      if (pen[i]==0)  fprintf(f, "%4d%4d", pen[i], pdt[i]);
      fprintf(f, " %s", desc[i]);
    }
    fprintf(f, "\n");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) 
      fprintf(f, " %5d %5d", round[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) 
        fprintf(f, " %5d %5d", round2[i][j], res2[i][j]);
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

int Find(char* s) {
  if (!s) return -1;
  int found = 0;
  int i = 0;
  int multi = 0;
  int j;
    
  while (i < n && !found) {
//    if (strstr(mnem[id[i]], s)==mnem[id[i]]) return i;
    if (strcmp(mnem[id[i]], s)==0) return i;
    else i++;
  }
  return -1;
}

int Month(char *s) {
  if (s[0]>='a' && s[0]<='z') s[0] -= 32;
//  if (strlen(s)>3) s[4] = 0;
  for (int i=0; i<12; i++)
    if (strcmp(luna[i],s)==0) return i+1;
  for (int i=0; i<12; i++)
    if (strcmp(fluna[i],s)==0) return i+1;
  return -1000;
}

int IsScore(char *s) {
  if (!s) return 0;
  int l = strlen(s);
  if (l<3) return 0;
  int x = 0;
  int i = 0;
  while (i<l) {
    if (s[i]!='-' && (s[i]<'0' || s[i]>'9')) return 0; 
    if (x==0) {
      if (s[i]=='-') x = 1;
    }
    else if (x==1) {
      if (s[i]<'0' || s[i]>'9') return 0;
      else x = 2;
    }
    i++;
  }
 return 1;
}

int Fmonth(char *s) {
  if (s[0]>='a' && s[0]<='z') s[0] -= 32;
  for (int i=0; i<12; i++)
    if (strcmp(fluna[i],s)==0) return i+1;
  return -1;
}

void BumpDate(int &m, int &z) {
  m = (m + (z+6)/30 -1)%12 + 1;
  z = (z+6)%30 + 1;
}

int sup(int i, int j) {
  int p1 = ppv*win[i]+drw[i];
  int p2 = ppv*win[j]+drw[j];
  if (p1 > p2) return 1;
  if (p1 < p2) return 0;
  if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
  if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
  if (gsc[i] > gsc[j]) return 1;
  if (gsc[i] < gsc[j]) return 0;   
  if (i > j) return 0;
  return 1;
}
  
void Ranking() {
  // BubbleSort
  int i, j;
  for (i=0; i<n; i++) rank[i] = i;
  int sorted;
  do {
    sorted = 1;
    for (i=0; i<n-1; i++)
      if (sup(rank[i+1], rank[i])) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }   
  } while (!sorted);
}
  
void Listing() {
  printf("\nStandings:\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    int pts = ppv*win[x] + drw[x];
    printf("%2d.%-30s%2d%3d%3d%3d%4d-%2d%3d\n", i+1,
     club[id[x]], win[x]+drw[x]+los[x],
     win[x], drw[x], los[x], gsc[x], gre[x], pts);

     if (i==p1-1)
        printf("------------------------------------------------------\n");
     if (i==p1+p2-1 && p2>0)
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-(r1+r2)-1 && r2>0)
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-r1-1 && r1>0)
        printf("------------------------------------------------------\n");

  }
}
        

int AddResult(int a, int b, int r, int x, int y) {
  int second = 0;
  if (res[a][b] >= 0)  {
    if (res2[a][b] >= 0) {
      if (res[b][a] >= 0) {
        if (res2[b][a] >= 0) {
          printf("%s-%s already stored twice h&a.\n", club[id[a]], club[id[b]]); 
          return 1;
        }
        else {
          round2[b][a] = r;
          res2[b][a] = 100*y+x;
          second = 2;      
        }
      }
    }
    else second = 1;
  }
  if (second == 1) {
   if (tbr >= 10) {
    round2[a][b] = r;
    res2[a][b]   = 100*x+y;
   }
   else if (res[b][a] < 0) {
     round[b][a] = r;
     res[b][a] = 100*y+x;
   }
   else {
      printf("%s-%s already stored h&a.\n", club[id[a]], club[id[b]]);
   }
  }
  else if (second == 0) {
    if (round[a][b]<0) {
      round[a][b] = r;
      res[a][b] = 100*x+y;
    }
    else {
      round[b][a] = r;
      res[b][a] = 100*y+x;
    }
  }
  gsc[a] += x; gre[a] += y;
  gsc[b] += y; gre[b] += x;
  if (x>y) { win[a]++; los[b]++; }
    else if (x==y) { drw[a]++; drw[b]++; }
     else { los[a]++; win[b]++; }
  printf("R%2d %2d %s %-15s%d-%d %-15s\n", r/1000, r%50, luna[(r%1000)/50-1],
          mnem[id[a]], x, y, mnem[id[b]]);
  return 1;
}

int LoadFile(char *filename) {
  FILE *f; 
  int i, j, k, x, y, r, z;
//  printf("Reading from file %s...\n", filename);
  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }
  // Loading file
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &p1, &p2, &r1, &r2);
  char s[2000], *tok[20];
  for (i=0; i<n; i++) {
    fgets(s, 2000, f);
    tok[0] = strtok(s, " \t\n");
    for (int j=1; j<20; j++) tok[j] = strtok(NULL, " \t\n");
    id[i] = atoi(tok[0]);
    if (reset) {
      win[i] = drw[i] = los[i] = gsc[i] = gre[i] = pts[i] = pen[i] = pdt[i] = 0;
    }
    else {
      win[i] = atoi(tok[2]);
      drw[i] = atoi(tok[3]);
      los[i] = atoi(tok[4]);
      gsc[i] = atoi(tok[5]);
      gre[i] = atoi(tok[6]);
      pts[i] = atoi(tok[7]);
    }
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  if (!reset) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++) {
        fscanf(f, "%d %d", &r, &z);
        round[i][j]  = r;
        res[i][j]    = z;
        round2[i][j] = -1;
        res2[i][j]   = -1;
      }
      fscanf(f, "\n");
    }
  }
  fclose(f);
  return 1;
}

void trim(char *s) {
  if (s) {
  if (strlen(s) > 1) {
    int j = strlen(s)-1;
    while ((s[j]==32 || s[j]=='\t') && j>0) j--;
    int i = 0; while (s[i]==32 && i<strlen(s)-1) i++;
    if (i>0 || j<strlen(s)-1) {
      s[j+1] = 0;
      memmove(s, s+i, j-i+1);
      s[j-i+1]=0;
    }
  }
  }
}

int Extract(char *filename) {
  FILE *f;
  char *s, *tok[32], *tkm, *tkz;
  int i, j, r, m, z, x, y;
  
  if (reset) {
    for (int j=0; j<n; j++) {
      for (int k=0; k<n; k++) {
        round[j][k] = -1;
        res[j][k] = -1;
        round2[j][k] = -1;
        res2[j][k] = -1;
      }
      win[j] = drw[j] = los[j] = gsc[j] = gre[j] = pts[j] = 0;
    }
  }

  f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }

  s = new char[300];
  r = 0;
  m = fd/50;
  z = fd%50;
  
  while (!feof(f)) { 
    fgets(s, 300, f);
    if (strstr(s, "ound")!=NULL) {
      Ranking();
      Listing();
      printf("\n%s", s);
      tok[0] = strtok(s, " \t");
      tok[1] = strtok(NULL, " ([].\t\n");
      tok[2] = strtok(NULL, " ([-.]\t\n");
      tok[3] = strtok(NULL, " [-.])\t\n");
      tok[4] = strtok(NULL, " [-.])\t\n");
      if (tok[1] && strstr(tok[1], "ound")==NULL) {
        r = atoi(tok[1]);
        if (syntax < 4) {
         if (df==0) {
           if (tok[2]) m = Month(tok[2]);
           if (tok[3]) z = atoi(tok[3]);
         }
         else if (df==1) {
           if (tok[2]) z = atoi(tok[2]);
           if (tok[3]) m = Month(tok[3]);
         }
        }
        if (fd > 51 || tok[2]==NULL || tok[3]==NULL) {
          m = (m + (z+6)/30 -1)%12 + 1;
          z = (z+6)%30 + 1;
        }
      }
      else if (df==2) {
         r = atoi(tok[0]);
         if (tok[3][1]>='A' && tok[3][1]<='Z') tok[3][1] += 32;
         if (tok[3][2]>='A' && tok[3][2]<='Z') tok[3][2] += 32;
         if (tok[3]) m = Month(tok[3]);
         if (tok[2]) z = atoi(tok[2]);
         else BumpDate(m, z);
      }
      else if (df==3) {
         r = atoi(tok[0]);
         if (tok[3]) m = atoi(tok[3]);
         if (tok[4]) z = atoi(tok[4]);
         else BumpDate(m, z);
      }
      else if (strstr(tok[1], "ound")!=NULL) {
        r = atoi(tok[0]);
      }
    }
    else if (s[0]=='[') {
      tok[0] = strtok(s, " \t[.-]");
      tok[1] = strtok(NULL, " \t[.-]");
      if (df==0) {
        m = Month(tok[0]);
        z = atoi(tok[1]);
      }
      else if (df==1) {
        z = atoi(tok[0]);
        m = Month(tok[1]);
      }
    }
    else if (strchr(s, '/') && (s[0]>='1' && s[0]<='9' && s[1]>='0' && s[1]<='9')) {
      tok[0] = strtok(s, "/");
      tok[1] = strtok(NULL, "/");
      tok[2] = strtok(NULL, "/\n");
      if (tok[2] != NULL) {
        m = atoi(tok[1]);
        z = atoi(tok[2]);
      }
    }
    else if ( 
      strstr(s, "Monday")!=NULL ||
      strstr(s, "Tuesday")!=NULL ||
      strstr(s, "Wednesday")!=NULL ||
      strstr(s, "Thursday")!=NULL ||
      strstr(s, "Friday")!=NULL ||
      strstr(s, "Saturday")!=NULL ||
      strstr(s, "Sunday")!=NULL
    ) {
      tok[0] = strtok(s, " \t\n");
      tok[1] = strtok(NULL, " \t\n");
      tok[2] = strtok(NULL, " \t\n");
      if (tok[1]!=NULL && tok[2]!=NULL) {
        z = atoi(tok[1]);
        m = Fmonth(tok[2]);
      }
    }
   
    //-----------------------------------------
    // syntax  0: T1 x-y T2
    // syntax  1: T1 - T2 x-:y
    // syntax  2: T1 T2 x y
    // syntax  3: T1 x - y T2
    // syntax  4: T1 - T2 x1-y1 x2-y2
    // syntax  5: T1 - T2 x1-y1 x2-y2 x3-y3
    // syntax  6: T1 x T2 y
    // syntax  7: dd/mmm T1 x-y T2
    // syntax  8: dd/mm/yyy T1 a x b T2
    // syntax  9: dd.mm T1 x-y T2
    // syntax 10: T1 - T2 x - y
    // syntax 11: T1 x-y T2 attend,000
    // syntax 12: hh.mm T1 x-y T2
    //-----------------------------------------

    else if (strlen(s)>7) {
      // syntax 0: T1 x-y T2
      // syntax 11: T1 x-y T2 attend,000
      if (syntax==0 || syntax==11) {
        tok[0] = s;
        int w = 1;
        while (w<strlen(s) && !( (s[w]=='-' || s[w]==':') && s[w-1]>='0' && s[w-1]<='9' && s[w+1]>='0' && s[w+1]<='9')) w++;
        if (w>=strlen(s)) w=-1; 
        else { tok[9] = s+w+1; s[w]=0; w--; }
        while (w>=0 && tok[0][w]!=' ' && tok[0][w]!='\t') w--;
        if (w>=0) {
          tok[0][w] = 0;
          tok[1] = tok[0]+w+1;
          tok[2] = strtok(tok[9], " \t");
//          tok[3] = strtok(NULL, "([\t\n");
          tok[3] = strtok(NULL, "({[\t\n");
          if (syntax==11) {
            int ww = strlen(tok[3])-1;
            while (ww>0 && tok[3][ww]!=' ' && tok[3][ww]!='\t') ww--;
            if (ww>0) tok[3][ww] = 0;
          }
          if (s[0] == '{') {
            tok[8] = strtok(s, " \t");
            tok[0] = strtok(NULL, "|");
          }
          trim(tok[0]);
          trim(tok[3]);
          i = Find(tok[0]);
          j = Find(tok[3]);
          if (i>=0 && j>=0) {
            x = atoi(tok[1]);
            y = atoi(tok[2]);
            AddResult(i,j,1000*r+50*m+z,x,y);
          }
        }     
      } 
      // syntax T1 - T2 x-y
      else if (syntax==1) {
        char home[64], guest[64], aux[64];
        tok[0] = strtok(s, " \t\n");
        int h;
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        int k = 9; while (k>0 && tok[k]==NULL) k--;
        if (k<3) continue;
        if (tok[k][strlen(tok[k])-1]==']') {
          tkm = tok[k-1]+1;
          tkz = tok[k];
        }
        else tkm = NULL;
        if (tok[k][strlen(tok[k])-1]==')') k--;
        char *tkx = strtok(tok[k],"-:");
        char *tky = strtok(NULL,"\n");
        if (tkx==NULL || tky==NULL) continue;
        x = atoi(tkx);
        y = atoi(tky);

        sprintf(home, "%s %s", tok[0], tok[1]);
        i = Find(home);
        if (i<0) {
          strcpy(home, tok[0]);
          i = Find(home);
          h = 0;
          while (i<0 && h<k-1) {
            strcat(home, " ");
            h++;
            strcat(home, tok[h]);
            i = Find(home);
          }
          if (h>k-2) continue;
        }
        h=k-1;
        strcpy(guest, tok[h]);
        j = Find(guest);
        while (j<0 && h>1) {
          h--;
          sprintf(aux, "%s %s", tok[h], guest);
          strcpy(guest, aux);    
          j = Find(guest);
        }
        if (i>=0 && j>=0)
          AddResult(i,j,1000*r+50*m+z,x,y);
      } 
      // syntax T1 T2 x y
      else if (syntax==2) {
        char home[64], guest[64], aux[64];
        tok[0] = strtok(s, " \t\n");
        int h;
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        int k = 9; while (k>0 && tok[k]==NULL) k--;
        if (k<3) continue;
        y = atoi(tok[k]);
        x = atoi(tok[k-1]); 

        sprintf(home, "%s %s", tok[0], tok[1]);
        i = Find(home);
        if (i<0) {
          strcpy(home, tok[0]);
          i = Find(home);
          h = 0;
          while (i<0 && h<k-1) {
            strcat(home, " ");
            h++;
            strcat(home, tok[h]);
            i = Find(home);
          }
          if (h>k-2) continue;
        }
        h=k-2;
        strcpy(guest, tok[h]);
        j = Find(guest);
        while (j<0 && h>1) {
          h--;
          sprintf(aux, "%s %s", tok[h], guest);
          strcpy(guest, aux);    
          j = Find(guest);
        }
        if (i>=0 && j>=0) {
          if (tkm==NULL) AddResult(i,j,1000*r+50*m+z,x,y);
          else AddResult(i,j,1000*r+50*Month(tkm)+atoi(tkz),x,y);
        }
      } 
      // syntax 3: T1 x - y T2
      else if (syntax==3) {
        tok[0] = strtok(s, " \t\n");
        int h;
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        int k = 0; 
        while (tok[k]!=NULL && strcmp(tok[k],"-")!=0 && 
          !(k>0 && tok[k-1][0]>='0' && tok[k-1][0]<='9' && tok[k+1][0]>='0' && tok[k+1][0]<='9')) k++;
        if (tok[k]==NULL) continue;
        char home[64], guest[64];
        strcpy(home, tok[0]);
        h = 1;
        while (h < k-1) {
          strcat(home, " "); strcat(home, tok[h]);
          h++;
        }
        strcpy(guest, tok[k+2]);
        h = k+3;
        while (h<10 && tok[h]!=NULL) {
          strcat(guest, " "); strcat(guest, tok[h]);
          h++;
        }
        i = Find(home);
        j = Find(guest);
        if (i>=0 && j>=0) {
          x = atoi(tok[k-1]);
          y = atoi(tok[k+1]);
          AddResult(i,j,1000*r+50*m+z,x,y);
        }     
      } 
      // syntax 4 : T1 - T2 x1-y1 x2-y2
      else if (syntax==4) {
        char home[64], guest[64], aux[64];
        tok[0] = strtok(s, " \t\n");
        int h;
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        int k = 9; while (k>0 && tok[k]==NULL) k--;
        if (k<3) continue;

        char *tkx2 = strtok(tok[k],"-");
        char *tky2 = strtok(NULL,"\n");
        if (tkx2==NULL || tky2==NULL) continue;
        int x2 = atoi(tkx2);
        int y2 = atoi(tky2);
        char *tkx1 = strtok(tok[k-1],"-");
        char *tky1 = strtok(NULL,"\n");
        if (tkx1==NULL || tky1==NULL) continue;
        int x1 = atoi(tkx1);
        int y1 = atoi(tky1);

        sprintf(home, "%s %s", tok[0], tok[1]);
        i = Find(home);
        if (i<0) {
          strcpy(home, tok[0]);
          i = Find(home);
          h = 0;
          while (i<0 && h<k-1) {
            strcat(home, " ");
            h++;
            strcat(home, tok[h]);
            i = Find(home);
          }
          if (h>k-2) continue;
        }
        h=k-2;
        strcpy(guest, tok[h]);
        j = Find(guest);
        while (j<0 && h>1) {
          h--;
          sprintf(aux, "%s %s", tok[h], guest);
          strcpy(guest, aux);    
          j = Find(guest);
        }
        if (i>=0 && j>=0) {
          AddResult(i,j,1000*(r+ 0)+50*m+z+  0,x1,y1);
          AddResult(j,i,1000*(r+n-1)+50*m+z+200,y2,x2);
        }
      } 
      // syntax 5: T1 - T2 x1-y1 x2-y2 x3-y3
      else if (syntax==5) {
        char home[64], guest[64], aux[64];
        tok[0] = strtok(s, " \t\n");
        int h;
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        int k = 9; while (k>0 && tok[k]==NULL) k--;
        if (k<3) continue;
        char *tkx3 = strtok(tok[k],"-");
        char *tky3 = strtok(NULL,"\n");
        if (tkx3==NULL || tky3==NULL) continue;
        int x3 = atoi(tkx3);
        int y3 = atoi(tky3);
        char *tkx2 = strtok(tok[k-1],"-");
        char *tky2 = strtok(NULL,"\n");
        if (tkx2==NULL || tky2==NULL) continue;
        int x2 = atoi(tkx2);
        int y2 = atoi(tky2);
        char *tkx1 = strtok(tok[k-2],"-");
        char *tky1 = strtok(NULL,"\n");
        if (tkx1==NULL || tky1==NULL) continue;
        int x1 = atoi(tkx1);
        int y1 = atoi(tky1);

        sprintf(home, "%s %s", tok[0], tok[1]);
        i = Find(home);
        if (i<0) {
          strcpy(home, tok[0]);
          i = Find(home);
          h = 0;
          while (i<0 && h<k-1) {
            strcat(home, " ");
            h++;
            strcat(home, tok[h]);
            i = Find(home);
          }
          if (h>k-2) continue;
        }
        h=k-3;
        strcpy(guest, tok[h]);
        j = Find(guest);
        while (j<0 && h>1) {
          h--;
          sprintf(aux, "%s %s", tok[h], guest);
          strcpy(guest, aux);    
          j = Find(guest);
        }
        if (i>=0 && j>=0) {
          AddResult(i,j,1000*(r+ 0)+50*m+z+  0,x1,y1);
          AddResult(j,i,1000*(r+n-1)+50*m+z+150,y2,x2);
          AddResult(i,j,1000*(r+2*n-2)+50*m+z+300,x3,y3);
        }
      } 
      // syntax 6: T1 x T2 y
      else if (syntax==6) {
        tok[0] = strtok(s, " \t\n");
        int h;
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        int k1 = 0; 
        while (tok[k1]!=NULL && k1<10 && (strlen(tok[k1])>1 || tok[k1][0]<'0' || tok[k1][0]>'9')) k1++;
        int k2 = k1+1;
        while (tok[k2]!=NULL && k2<10 && (strlen(tok[k2])>1 || tok[k2][0]<'0' || tok[k2][0]>'9')) k2++;
        if (k2>=10 || tok[k2]==NULL) continue;
        char home[64], guest[64];
        strcpy(home, tok[0]);
        for (h = 1; h < k1; h++) {
          strcat(home, " "); strcat(home, tok[h]);
        }
        strcpy(guest, tok[k1+1]);
        for (h = k1+2; h < k2; h++) {
          strcat(guest, " "); strcat(guest, tok[h]);
        }
        i = Find(home);
        j = Find(guest);
        if (i>=0 && j>=0) {
          x = atoi(tok[k1]);
          y = atoi(tok[k2]);
          AddResult(i,j,1000*r+50*m+z,x,y);
        }     
      } 
      // syntax 7: dd/mmm T1 x-y T2
      else if (syntax==7) {
        int h;
        tok[0] = strtok(s, " /[\n");
        for (h=1; h<10; h++) tok[h] = strtok(NULL, " \t\n");
        z = atoi(tok[0]);
        if (tok[1][0]>='a' && tok[1][0]<='z') tok[1][0] -= 32;
        m = Month(tok[1]);
        if (m<=0) m = atoi(strtok(tok[1],"/"));

        char home[64], guest[64];
        int k1 = 3; 
        while (tok[k1]!=NULL && (tok[k1][0]<'0' || tok[k1][0] >'9')) k1++;
        if (tok[k1]==NULL) continue;
        int k2 = 9; while (tok[k2]==NULL && k2>0) k2--;
        if (k2==0) continue;
        char *tkx = strtok(tok[k1], "-");
        char *tky = strtok(NULL, "-");
        if (tkx==NULL || tky==NULL) continue;
        x = atoi(tkx);
        y = atoi(tky);

        strcpy(home, tok[2]);
        for (h = 3; h < k1; h++) {
          strcat(home, " "); strcat(home, tok[h]);
        }
        strcpy(guest, tok[k1+1]);
        for (h = k1+2; h <= k2; h++) {
          strcat(guest, " "); strcat(guest, tok[h]);
        }
          i = Find(home);
          j = Find(guest);
          if (i>=0 && j>=0) {
            AddResult(i,j,1000*r+50*m+z,x,y);
          }
      } 
      // syntax 8: dd/mm/yyy T1 a x b T2
      else if (syntax==8) {
        int h;
        tok[0] = strtok(s, " /[\n");
        tok[1] = strtok(NULL, " /[\n");
        for (h=2; h<20; h++) tok[h] = strtok(NULL, " \t\n");
        z = atoi(tok[0]);
        m = atoi(tok[1]);
        int k1 = 3; 
        while (tok[k1]!=NULL && strcmp(tok[k1], "x")!=0) k1++;
        if (tok[k1]==NULL) continue;
        int k2 = 19; while (tok[k2]==NULL && k2>0) k2--;
        if (k2==0) continue;
        x = atoi(tok[k1-1]);
        y = atoi(tok[k1+1]);

        char home[64], guest[64];
        strcpy(home, tok[3]);
        for (h = 4; h < k1-1; h++) {
          strcat(home, " "); strcat(home, tok[h]);
        }
        strcpy(guest, tok[k1+2]);
        for (h = k1+3; h <= k2; h++) {
          strcat(guest, " "); strcat(guest, tok[h]);
        }
        i = Find(home);
        j = Find(guest);
        if (i>=0 && j>=0) {
          AddResult(i,j,1000*r+50*m+z,x,y);
        }
      } 
      else if (syntax==9 || syntax==12) {
        // dd.mm T1 x-y T2
        // hh.mm T1 x-y T2
        strtok(s, "[");
        tok[0] = strtok(s, " \t\n");
        for (int k=2; k<10; k++) tok[k] = strtok(NULL, " \t\n");
        if (tok[0]) {
          strtok(tok[0], ".");
          tok[1] = strtok(NULL, ".");
          if (syntax==9) {
            z = atoi(tok[0]);
            if (tok[1]) m = atoi(tok[1]);
          }
        } 
        else continue;

        int w = 3;
        while (tok[w] && !IsScore(tok[w])) w++;
        if (tok[w]==NULL) continue;
        char home[64], guest[64], aux[64];

        strcpy(home, tok[2]);
        for (int k=3; k<w; k++) {
          strcat(home, " ");
          strcat(home, tok[k]);
        }        

        strcpy(guest, tok[w+1]);
        for (int k=w+2; tok[k]!=NULL; k++) {
          strcat(guest, " ");
          strcat(guest, tok[k]);
        }        

        i = Find(home);
        j = Find(guest);
        strtok(tok[w], "-");
        tok[10] = strtok(NULL, "-");
        if (i>=0 && j>=0) {
          x = atoi(tok[w]);
          y = atoi(tok[10]);
          AddResult(i,j,1000*r+50*m+z,x,y);
        }
      } // syntax
    } // strlen>7
  } // feof

  fclose(f);
  printf("Input file closed.\n");
  return 1;
}


int main(int argc, char* argv[]) {
  printf("Usage: rsssf <a.2000> -syntax 1 <a.2000.html>\nTeams in 'rsssfteams.dat'\n");
  if (argc < 2) { printf("No input file specified.\n"); return 0; }
  if (argc > 2) strcpy(inputfile, argv[argc-1]);
  else sprintf(inputfile, "html/%s", argv[1]);
  reset = 1;
  df = 0;
  fd = 51;
  syntax = 0;
  for (int j=2; j<argc-1; j++) {
    if (strstr(argv[j], "-nor")!=NULL) reset = 0;
    else if (strcmp(argv[j], "-df")==0) {
      if (j+1<argc && strcmp(argv[j+1],"md")==0) df = 0;
      if (j+1<argc && strcmp(argv[j+1],"dm")==0) df = 1;
      if (j+1<argc && strcmp(argv[j+1],"cro")==0) df = 2;
      if (j+1<argc && strcmp(argv[j+1],"cro2")==0) df = 3;
    }
    else if (strcmp(argv[j], "-fd")==0) {
      if (j+1<argc) fd = atoi(argv[j+1]);
    }
    else if (strstr(argv[j], "-s")==argv[j]) {
      if (j+1<argc) syntax = atoi(argv[j+1]);
    }
  }
  Load("rsssfteams.dat");
  if (!LoadFile(argv[1])) return 0;
  Extract(inputfile);
  Ranking();
  Listing();
  Dump(argv[1]);
  return 1;
}
