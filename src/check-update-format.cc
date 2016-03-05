#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>

char *shortmonth[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char *month[] = {"", "January", "February", "March", "April", "May", "June",
                     "July", "August", "September", "October", "November", "December"};

char *dow[] = {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char **club;
char **mnem;
int  NC, NG;
int  n, ppv, tbr, pr1, pr2, rel1, rel2, option, _rnd, year;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pen[64], pdt[64];
int  pts[64], rank[64], rnd[64][64], res[64][64];
int  rnd2[64][64], res2[64][64];
char desc[64][32];

int Month(char *m) {
  for (int i=1; i<=12; i++)
   if (strstr(m, month[i])==m) return i;
  return 0;
}

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
  f = fopen("riku.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];

  for (int i=0; i<NC; i++) {
    fgets(s, 200, f);
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

int Find(char* s) {
  if (!s) return -1;
  int l = strlen(s);
  while (l>0 && (s[l-1]==' ' || s[l-1]=='\t')) l--;
  s[l] = 0;
  for (int i=0; i<n; i++)
    if (strcmp(mnem[id[i]], s)==0) return i;
  return -1;
}

int LoadFile(char *filename) {
  printf("\n\033[33;1mReading from file %s \033[0m...\n", filename);
  FILE *f = fopen(filename, "rt");
  if (NULL == f) { printf("File %s not found.\n", filename); return 0; }

  // Loading file
  char fncopy[64]; 
  strcpy(fncopy, filename);
  char *ystr = strtok(fncopy, ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);
  int i, j, x, y, zi;
  char s[200], *tok[20];
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
      fscanf(f, "%d %d", &x, &y);
      rnd[i][j] = x;
      res[i][j]   = y;
      rnd2[i][j] = res2[i][j] = -1;
    }
    fscanf(f, "\n");
  }
  if (tbr>=10) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &x, &y);
      rnd2[i][j] = x;
      res2[i][j]   = y;
    }
    fscanf(f, "\n");
  }
  fclose(f);
  }
}

int GetFormat(char *filename) {
  FILE *f = fopen(filename, "rt");
  if (f==NULL) return 0;
  char s[50000], u[1024], v[128];
  int a, b, x, y, m, m1, z, z1, r, yr;
  char *t, *gm, *tk, *tkw, *tkm, *tkd, *tkz, *tkj, *tkyr, *tks, *t1, *t2, *tkx, *tky;
  char home[64], guest[64], score[64], scr[64], dow[64];

  r = 0;
  while (!feof(f)) {
    fgets(s, 1024, f);
    if (strstr(s, "  match")!=NULL) {
      fgets(s, 1024, f); // dow
      tk = strstr(s, "day");
      if (tk!=NULL) {
        tkw = tk+5;
        tk = strtok(tkw, "<");
        strcpy(dow, tkw);
      }
      do { 
       fgets(s, 1024, f);
        tk = strstr(s, "date");
      } while (!feof(f) && tk==NULL);
      tkj  = strstr(tk, "<span");
      tkd  = strstr(tkj, ">");
      tkz  = strtok(tkd+1, "/");
      tkm  = strtok(NULL, ",/<\n");
      if (tkz && strlen(tkz)>0) z1  = atoi(tkz); else z1 = 0;
      if (z1>0) z = z1;
      if (tkm && strlen(tkm)>0)  m1 = atoi(tkm); else m1 = 0;
      if (m1>0) m = m1;
      do { 
        fgets(s, 1024, f); 
        tk = strstr(s, "team-a");
      } while (tk==NULL && !feof(f));
      if (tk!=NULL) {
        for (int ii=0; ii<3; ii++) fgets(s, 1024, f);
        s[strlen(s)-1] = 0;
        t2 = s; while (t2[0]==' ') t2++;
        strcpy(home, t2);
      }
      a = Find(home);
      if (a<0) printf("\033[31;1mCannot find team %s\033[0m...\n", home);

      for (int ii=0; ii<10; ii++)
        fgets(s, 1024, f);
      if (strstr(s, "PSTP")!=NULL) continue;
      if (strstr(s, "CANC")!=NULL) continue;
      if (strstr(s, "SUSP")!=NULL) continue;
      if (strchr(s, ':')!=NULL) continue;
      tkx = strtok(s, " -");
      tky = strtok(NULL, " -\n");
      if (tkx==NULL || tky==NULL) continue;
      x = atoi(tkx);
      y = atoi(tky);

      do { 
        fgets(s, 1024, f); 
        tk = strstr(s, "team-b");
      } while (tk==NULL && !feof(f));
      if (tk!=NULL) {
        for (int ii=0; ii<3; ii++) fgets(s, 1024, f);
        s[strlen(s)-1] = 0;
        t2 = s; while (t2[0]==' ') t2++;
        strcpy(guest, t2);
      }
      b = Find(guest);
      if (b<0) printf("\033[31;1mCannot find team %s\033[0m...\n", guest);
      if (a>=0 && b>=0) { fclose(f); return 2009; }
    }
  }

  fclose(f);
  f = fopen(filename, "rt");
  if (f==NULL) return 0;
  while (!feof(f)) {
    fgets(s, 50000, f);
    if (strstr(s, " score")!=NULL) {
      t = strstr(s, "tbody");
      if (t==NULL) continue;
      while ( (gm = strstr(t, "<tr class")) != NULL) {
        tk = strstr(gm, "<td class");
        if (tk==NULL) { t = gm + 3; continue; }
        t1 = strstr(tk, "span");
        if (t1==NULL) { t = gm + 3; continue; };
        t2 = strstr(t1, ">");
        if (t2==NULL) { t = gm + 3; continue; };
        strncpy(u, t2+1, 1024);
        tkw = strtok(u, "<");

        gm = tk + 3;    // advance
        tk = strstr(gm, "<td class");
        if (tk==NULL) { t = gm + 3; continue; };
        t1 = strstr(tk, "span");
        if (t1==NULL) { t = gm + 3; continue; };
        t2 = strstr(t1, ">");
        if (t2==NULL) { t = gm + 3; continue; };
        strncpy(u, t2+1, 1024);
        tkd = strtok(u, "<");
        if (tkd==NULL) { t = gm + 3; continue; };
        tkz = strtok(tkd, "/");
        tkm = strtok(NULL, ",/<\n");
        if (tkm && strlen(tkm)>1)  {
           m1 = atoi(tkm);
        }
        if (m1>0) m = m1;
        if (m1>0 && tkz && strlen(tkz)>0) {
           z1  = atoi(tkz); 
        }
        else {
          z1 = 0;
        }
        if (z1>0) z = z1;

        gm = tk + 3;    // advance
        tk = strstr(gm, "<td class");
        if (tk==NULL) { t = gm + 3; continue; };
        t1 = strstr(tk, "<a href");
        if (t1==NULL) { t = gm + 3; continue; };
        t2 = strstr(t1, ">");
        if (t2==NULL) { t = gm + 3; continue; };
        strncpy(u, t2+1, 1024);
        tkd = strtok(u, "<");
        if (tkd==NULL) { t = gm + 3; continue; };
        strcpy(home, tkd);
        a = Find(home);
        if (a<0) printf("\033[31;1mCannot find team %s\033[0m...\n", home);

        gm = tk + 3;    // advance
        tk = strstr(gm, "<td class");
        t = tk + 3;
        if (tk==NULL) { t = gm + 3; continue; };
        t1 = strstr(tk, "<a href");
        if (t1==NULL) { t = gm + 3; continue; };
        if (strstr(t1-7, "score") != t1-7) { t = gm + 3; continue; };
        t2 = strstr(t1, ">");
        if (t2==NULL) { t = gm + 3; continue; };
        strncpy(u, t2+1, 1024);
        tkd = strtok(u, "<");
        if (tkd==NULL) { t = gm + 3; continue; };
        strncpy(v, tkd, 16);
        if (strstr(v, "PSTP")!=NULL) { t = gm + 3; continue; };
        if (strstr(v, "CANC")!=NULL) { t = gm + 3; continue; };
        if (strstr(v, "SUSP")!=NULL) { t = gm + 3; continue; };
        if (strstr(v, ":")!=NULL) { t = gm + 3; continue; };
        tkx = strtok(v, " -");
        tky = strtok(NULL, " -\n");
        if (tkx==NULL || tky==NULL) { t = gm + 3; continue; };
        x = atoi(tkx);
        y = atoi(tky);

        gm = tk + 3;    // advance
        tk = strstr(gm, "<td class");
        if (tk==NULL) { t = gm + 3; continue; };
        t1 = strstr(tk, "<a href");
        if (t1==NULL) { t = gm + 3; continue; };
        t2 = strstr(t1, ">");
        if (t2==NULL) { t = gm + 3; continue; };
        strncpy(u, t2+1, 1024);
        tkd = strtok(u, "<");
        if (tkd==NULL) { t = gm + 3; continue; };
        strcpy(guest, tkd);
        b = Find(guest);
        if (b<0) printf("\033[31;1mCannot find team %s\033[0m...\n", home);

        if (a>=0 && b>=0) {
          fclose(f); return 2010;
        }

        t = gm + 3;
      }
    }
  }

  fclose(f);
  return 0;
}

int main(int argc, char **argv) {
  char inputfile[64], outputfile[64], updatesfile[64];

  Load();
  if (argc < 3) { 
    printf("Not all input files specified.\n"); 
    printf("Usage: autoupdate a.2007 first.html\n");
    return 0; 
  }

  FILE *f;
  strcpy(inputfile, argv[1]);
  strcpy(updatesfile, argv[2]);
  LoadFile(inputfile);
  int fmt = GetFormat(updatesfile);
  printf("Format = %d.\n", fmt);
  return 1;  
}
