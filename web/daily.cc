#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *normal  = "\033[0m";
const char *yellow  = "\033[33m";
const char *hiyellow  = "\033[33;1m";
const char *hicyan    = "\033[36;1m";

int NC;
int NL[300];
int wint[300];
char **country;
char **script;
char **dir;
char ***league;
char ***division;
char ***mnem;
char *fc, *fl, *fd;
char param[128];
int z, m, y;
int pt, pl;
int loaded[300];
int updated[300];
int cdired[300];
int do_upd;
int do_get;

int Load() {
  FILE *f = fopen("/home/radu/rsssf/sat/web/league-names.dat", "rt");
  if (!f) {
    printf("ERROR: league names not found.\n");
    return 0;
  }
  fscanf(f, "%d\n", &NC);
  country  = new char*[NC+1];
  script   = new char*[NC+1];
  dir      = new char*[NC+1];
  league   = new char**[NC+1];
  division = new char**[NC+1];
  mnem     = new char**[NC+1];
  char s[256], *w;
  char buf[30][100];
  int t = -1;
  int l = 0;
  while (!feof(f)) {
    fgets(s, 128, f);
    if (feof(f)) continue;
    strtok(s, "\n");
    if (s[0]=='#' || s[0]=='$') {
      t++;
      w = strtok(s+1, "|");
      country[t] = strdup(w);
      w = strtok(NULL, "|");
      script[t] = strdup(w);
      w = strtok(NULL, "\n");
      dir[t] = strdup(w);
      wint[t] = (s[0]=='$');
      if (t>0) {
        mnem[t-1]     = new char*[l+1];
        league[t-1]   = new char*[l+1];
        division[t-1] = new char*[l+1];
//        printf("### %s - %d leagues\n", country[t-1], l+1);
        NL[t-1] = l+1;
        for (int i=0; i<=l; i++) {
          mnem[t-1][i] = strdup(strtok(buf[i],":"));
          league[t-1][i] = strdup(strtok(NULL, "|"));
          char *dvn = strtok(NULL, "|");
          if (dvn==NULL) division[t-1][i]=NULL;
          else division[t-1][i] = strdup(dvn);
//          printf(" - %s\n", league[t-1][i]);
        }
      }
      l = -1;
    }
    else {
      l++;
      strcpy(buf[l], strtok(s, "\n"));
    }
  }
  fclose(f);
  return 1;
}

int Find(char *ctn, char *lgn, char *dvn, int &t, int &l) {
  for (int i = 0; i<NC; i++)  {
    if (strcmp(country[i], ctn)==0)  {
      t = i;
      int j = 0;
      while (j<NL[t] && strcmp(league[t][j], lgn)!=0) j++;
      if (j>=NL[t]) continue;
      else if (dvn!=NULL) {
        while (j<NL[t] && division[t][j]!=NULL && strcmp(division[t][j], dvn)!=0) j++;
        if (j>=NL[t]) continue;
      }
      l = j;
      return 1;
    }
  }
  return 0;
}

int Extract(char *filename) {
  FILE *f = fopen(filename, "rt");
  if (f==NULL) return 2;

  char s[1024];
  char *tk, *tks, *t1, *t2, *tkx, *tky, *tok[10];
  char home[64], guest[64], score[64], scr[64];
  char outfile[128];
  for (int ii=0; ii<NC; ii++) cdired[ii] = loaded[ii] = updated[ii] = 0;
  int t, l, k;
  FILE *g = fopen("today", "wt");
  if (!g) return 3;
  FILE *r = fopen("res-today", "wt");
  if (!g) return 3;
  if (!r) return 3;
  fprintf(g, "#!/bin/csh\n\n");
  fprintf(r, "#!/bin/csh\n\n");

  FILE *h;
  strcpy(outfile, "");
  h = NULL;
  while (!feof(f)) {
    fgets(s, 1000, f);
    if (strstr(s, "group-head")!=NULL) {
    if (strstr(s, "clickable")!=NULL || strstr(s, "loaded")!=NULL) {
      do {
        fgets(s, 1000, f);
        tok[0] = strstr(s, "span class");
      } while (!feof(f) && tok[0]==NULL);
      if (tok[0] == NULL) continue;
      tok[1] = strchr(tok[0], '>');
      if (tok[1] == NULL) continue;
      if (h) fclose(h);
      tok[1]++;
      char *bosnia =  strstr(tok[1], "Bosnia");
      if (bosnia) bosnia[6] = ' ';
      tok[2] = strtok(tok[1], "<");
      tok[3] = strtok(tok[2], "-"); 
      tok[4] = strtok(NULL, ">");
      tok[5] = strtok(NULL, ">");   
      if (tok[3]) { tok[3][-2] = 0; }
      if (tok[4]) { tok[4][-2] = 0; tok[4]++; }
      if (tok[5]) { tok[5][-2] = 0; tok[5]++; }
//      printf("*** %s/%s", tok[3], tok[4]);
      if (Find(tok[3], tok[4], tok[5], t, l)) {
        printf("*** | %-16s| %-20s", country[t], league[t][l]);
        printf("| %-12s", (tok[5]?division[t][l]:" "));
        printf("| [%s(%s)]\n", script[t], mnem[t][l]);

        int delta = 0;
        if (wint[t]==0 && m>6) delta = 1;
        if (do_get && !loaded[t]) {
            fprintf(g, "echo \"%s\"\n", hicyan);
            fprintf(g, "echo \"==================================\"\n");
            fprintf(g, "echo \"\t%s\"\n", country[t]);
            fprintf(g, "echo \"==================================\"\n");
            fprintf(g, "echo \"%s\"\n", normal);
          fprintf(g, "\n../web/get-%s\n", script[t]);
          loaded[t] = 1;
        }
        if (do_upd) {
          if (!cdired[t]) {
            fprintf(g, "cd /home/radu/rsssf/sat/%s\n", dir[t]);
            cdired[t] = 1;
          }
//          fprintf(g, "../update %s.%d ../web/%s/%s.html\n", mnem[t][l], y+delta, dir[t], mnem[t][l]);
          if (!updated[t]) {
            fprintf(g, "mv alias.dat alias.saved\n");
            fprintf(g, "i2u alias.saved > alias.dat\n");
            fprintf(g, "../web/u-%s\n", script[t]);
            fprintf(g, "mv -f alias.saved alias.dat\n");
             updated[t] = 1;
          }
        }

        sprintf(outfile, "%s/%s.html", dir[t], mnem[t][l]);
        if (pt<0 || (t==pt && l==pl))
          h = fopen(outfile, "wt");
        else 
          h = NULL; 
      }
      else h = NULL; 
    }
    }
    else if (h!=NULL) {
      if (strstr(s, "  matc")!=NULL) {
         fprintf(h, "%s", s);
         fprintf(h, "\tfake dow\n");
         fprintf(h, "\t<td class=\"full-date\">%02d-%02d-%02d</td>\n", z, m, y);
      }
      else if (strstr(s, "team-b")!=NULL) fprintf(h, "%s", s);
      else if (strstr(s, "team-a")!=NULL) {
        for (int ii=0; ii<5; ii++) {
          fprintf(h, "%s", s);
          fgets(s, 1000, f);
        }
      }
    }
  }
  fclose(f);
  fclose(g);
  fclose(r);
  if (h) fclose(h);
  return 1;
}

int main(int argc, char **argv) {
  if (!Load()) return 1;
  do_upd = 1;
  do_get = 1;

  if (argc < 2) { 
    printf("ERROR: no input file specified.\n");
    printf("Usage: daily index-2.html\n");
    return 1; 
  }
 
  for (int h=2; h<argc; h++) {
    if (strcmp(argv[h], "-g")==0) do_get = 0;
    if (strcmp(argv[h], "+g")==0) do_get = 1;
    if (strcmp(argv[h], "-u")==0) do_upd = 0;
    if (strcmp(argv[h], "+u")==0) do_upd = 1;
  }

  char sdate[100];
  strcpy(sdate, argv[1]);
  char *tk1 = strtok(sdate, "/");
  char *tky = strtok(NULL, "/");
  char *tkm = strtok(NULL, "/");
  char *tkz = strtok(NULL, ".");
  if (tkz) z = atoi(tkz); else z = 0;
  if (tkm) m = atoi(tkm); else m = 0;
  if (tky) y = atoi(tky); else y = 0;

  fc = fl = fd = NULL;
  pt = pl = -1;
  if (argc > 2) {
    strcpy(param, argv[2]+1);
    fc = strtok(param, "/:|");
    fl = strtok(NULL, "/:|");
    fd = strtok(NULL, "/:|");
    if (!Find(fc, fl, fd, pt, pl)) {
      printf("ERROR: %s is not registered.\n", argv[2]);
    }
  }

  Extract(argv[1]);

  return 0;
}
