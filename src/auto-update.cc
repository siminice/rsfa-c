#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>

#define HEAD_TO_HEAD 1

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *dow[] = {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char **club;
char **mnem;
int  NC;
int  n, ppv, tbr, pr1, pr2, rel1, rel2, option, _rnd, year;
int  id[64], win[64], drw[64], los[64], gsc[64], gre[64], pen[64], pdt[64];
int  pts[64], rank[64], rnd[64][64], res[64][64];
int  rnd2[64][64], res2[64][64];
int  tbwin[64], tbdrw[64], tblos[64], tbgsc[64], tbgre[64], tbrk[64];
int lastrng, lastrnd, lastr, lastrh[16], lastrg[16], lastrd[16], lastrs[16];
int updates, updatez, updater;
char desc[64][32];

char inputfile[64], outputfile[64], updatesfile[64];

int Month(char *m) {
  for (int i=1; i<=12; i++)
   if (strstr(m, month[i])==m) return i;
  return 0;
}



struct alias {
  int   year;
  char *name;
  char *nick;
  alias(int, char*, char*);
  ~alias();
};

struct node {
  alias *data;
  node  *next;
  node(alias*, node*);
  ~node();
};

struct Aliases {
  node *list;
  Aliases();
  ~Aliases();
  void Append(alias *a);
  char* GetName(int y);
  char* GetNick(int y);
};

//-------------------------------------

alias::alias(int y, char *s, char *n) {
  year = y;
  name = (char*) malloc(strlen(s)+1);
  strcpy(name, s);
  if (n != NULL) {
    nick = (char*) malloc(strlen(n)+1);
    strcpy(nick, n);
  }
  else nick = NULL;
};

alias::~alias() {
  if (name) delete name;
  if (nick) delete nick;
};

node::node(alias *a, node *n) {
  data = a;
  next = n;
};

node::~node() {
  if (next) delete next;
  delete data;
};

Aliases::Aliases() {
  list = NULL;
};

Aliases::~Aliases() {
 delete list;
}

void Aliases::Append(alias *a) {
  node *n = (node*) malloc(sizeof(node));
  n->data = a;
  n->next = list;
  list = n;
};

char* Aliases::GetName(int y) {
  if (!list) return NULL;
  node *n = list;
  char *s = n->data->name;
  int x = n->data->year;
  while (y < x && n->next != NULL) {
    n = n->next;
    s = n->data->name;
    x = n->data->year;
  }
  return s;
}

char* Aliases::GetNick(int y) {
  if (!list) return NULL;
  node *n = list;
  char *s = n->data->nick;
  int x = n->data->year;
  while (y < x && n->next != NULL) {
    n = n->next;
    s = n->data->nick;
    x = n->data->year;
  }
  return s;
}

Aliases **L;

//--------------------------------------------------

void Load() {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
  f = fopen("riku.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;

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


  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    fgets(s, 500, f);
    if (!s) continue;
    if (s[0]==0) continue;
    s[strlen(s)-1] = 0;
    tok[0] = strtok(s, "*");
    for (int j=1; j<20; j++)
      tok[j] = strtok(NULL, "*");
    int k=0;
    while(tok[k]) {
      ystr = strtok(tok[k], " ");
      name = strtok(NULL, "~");
      nick = strtok(NULL, "@");
      int y = atoi(ystr);
      alias *a = new alias(y, name, nick);
      L[i]->Append(a);
//      printf("Alias (%d,%s~%s) for %d.%s...\n", y, name, (nick!=NULL?nick:""), i, club[i]);
      k++;
    }
    s[0] = 0;
  }
  fclose(f);
}


char *NameOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetName(y);
  if (!s) return club[t];
  return s;
}

char *NickOf(Aliases **L, int t, int y) {
  char *s = L[t]->GetNick(y);
  if (!s) return mnem[t];
  return s;
}

void Dump() {
  FILE *f;
  int i, j;
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
      fprintf(f, "%6d%5d", rnd[i][j], res[i][j]);
    fprintf(f, "\n");
  }
  if (tbr>=10) {
    for (i=0; i<n; i++) {
      for (j=0; j<n; j++)
        fprintf(f, "%6d%5d", rnd2[i][j], res2[i][j]);
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

int sup(int i, int j, int tbr=0) {
  if (tbr%10==1) {
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    return sup(i, j, 0);
  }
  else {
    int p1 = pts[i] - pen[i];
    int p2 = pts[j] - pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (i > j) return 0;
    return 1;
  }
}

void Tiebreak(int h, int k) {
  for (int i=h; i<=k; i++) {
    int j = rank[i];
    tbwin[j] = tbdrw[j] = tblos[j] = tbgsc[j] = tbgre[j] = 0;
    tbrk[i] = rank[i];
  }
  int gm = 0;
  for (int i=h; i<=k; i++) {
    for (int j=h; j<=k; j++) {
      int t1 = rank[i];
      int t2 = rank[j];
      if (res[t1][t2] >= 0) {
        gm++;
        int x = res[t1][t2] / 100;
        int y = res[t1][t2] % 100;
        tbgsc[t1] += x; tbgre[t2] += x;
        tbgsc[t2] += y; tbgre[t1] += y;
        if (x>y) {tbwin[t1]++; tblos[t2]++;}
        else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
        else {tbwin[t2]++; tblos[t1]++;}
      }
    }
  }

  int sorted;
  do {
    sorted = 1;
    for (int i=h; i<k; i++)
      if (sup(tbrk[i+1], tbrk[i], (gm>=(k-h+1)*(k-h)) )) {
        sorted = 0;
        int aux = tbrk[i];
        tbrk[i] = tbrk[i+1];
        tbrk[i+1] = aux;
      }
  } while (!sorted);
  for (int i=h; i<=k; i++)
    rank[i] = tbrk[i];
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
  if (tbr) {
   i = 0;
    while (i<n-1) {
      j = i;
      while (j+1<n && (pts[rank[j+1]]-pen[rank[j+1]] == pts[rank[i]]-pen[rank[i]])) j++;
      if (j>i) Tiebreak(i,j);
      i = j+1;
    }
  }
}

void Listing() {
  printf("\nStandings:\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    printf("%2d.%-30s%2d%3d%3d%3d%4d-%2d", i+1,
     NameOf(L,id[x],year), win[x]+drw[x]+los[x],
     win[x], drw[x], los[x], gsc[x], gre[x]);
     if (pen[x]>0) printf("%3d (-%dp pen)" , pts[x]-pen[x], pen[x]);
     else if (pen[x]<0) printf("%3d (+%dp bonus)" , pts[x]-pen[x], -pen[x]);
     else printf("%3d", pts[x]);
     printf(" %s\033[0m\n", desc[x]);
     if (i==pr1-1)
        printf("------------------------------------------------------\n");
     if (i==pr1+pr2-1 && pr2>0)
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-(rel1+rel2)-1 && rel2>0)
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-rel1-1 && rel1>0)
        printf("------------------------------------------------------\n");
  }
}


int RankOf(int t) {
  for (int i=0; i<n; i++)
    if (rank[i]==t) return i;
  return -1;
}

int Find(char* s) {
  for (int i=0; i<n; i++)
    if (NULL != strstr(mnem[id[i]], s)) return i;
  return -1;
}

int LoadFile(char *filename) {
  printf("\n\033[33;1mReading from file %s \033[0m...\n", inputfile);
  FILE *f = fopen(inputfile, "rt");
  if (NULL == f) { printf("File %s not found.\n", inputfile); return 0; }

  // Loading file
  char fncopy[64]; 
  strcpy(fncopy, filename);
  char *ystr = strtok(fncopy, ".");
  ystr = strtok(NULL, " ");
  year = atoi(ystr);
  int i, j, x, y, zi;
  char s[200], *tok[20];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &pr1, &pr2, &rel1, &rel2);
  lastrng = 0;
  time_t  tt;
  struct  tm* t = new tm;
  time(&tt);
  t = localtime(&tt);
  int today = (t->tm_mon +1)*50 + t->tm_mday;
  lastrnd = -1;
  for (i=0; i<n; i++) {
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    win[i] = drw[i] = los[i] = gsc[i] = gre[i] = pts[i] = pen[i] = pdt[i] = 0;
    id[i] = atoi(tok[0]);
/*
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
*/
    if (tok[10]) strcpy(desc[i], tok[10]); else strcpy(desc[i],"");
  }
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
/*
      fscanf(f, "%d %d", &x, &y);
      rnd[i][j] = x;
      res[i][j]   = y;
      rnd2[i][j] = res2[i][j] = -1;
      if (rnd[i][j]>50) {
        int thisd = rnd[i][j]%1000;
        int storethis = 0;
        int dl = abs(today-lastrnd);
        int dc = abs(today-thisd);
        if (dc < dl) {
          lastrng = 0;
          storethis = 1;
          lastrnd = thisd;
          lastr = rnd[i][j]/1000;
        }
        else if (dc == dl) { storethis = 1; }
        if (storethis) {
          lastrh[lastrng] = i;
          lastrg[lastrng] = j;
          lastrd[lastrng] = thisd;
          lastrs[lastrng] = res[i][j];
          lastrng++;
          lastr = rnd[i][j]/1000;
        }
      }
*/
      res[i][j] = -1;
    }
//    fscanf(f, "\n");
  }
  if (tbr>=10) {
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
/*
      fscanf(f, "%d %d", &x, &y);
      rnd2[i][j] = x;
      res2[i][j]   = y;
      if (rnd2[i][j]>50) {
        int thisd = rnd2[i][j]%1000;
        int storethis = 0;
        int dl = abs(today-lastrnd);
        int dc = abs(today-thisd);
        if (dc < dl) {
          lastrng = 0;
          storethis = 1;
          lastrnd = thisd;
          lastr = rnd2[i][j]/1000;
        }
        else if (dc == dl) storethis = 1;
        if (storethis) {
          lastrh[lastrng] = i;
          lastrg[lastrng] = j;
          lastrd[lastrng] = thisd;
          lastrs[lastrng] = res2[i][j];
          lastrng++;
          lastr = rnd2[i][j]/1000;
        }
      }
*/
      res2[i][j] = -1;
    }
//    fscanf(f, "\n");
  }
  fclose(f);
  }

  printf("\nLatest results:\n");
  for (int i=0; i<lastrng; i++) {
    printf("R%d, %2d %3s: %-16s %d-%d %s\n", lastr,
            lastrd[i]%50, month[lastrd[i]/50], NickOf(L, id[lastrh[i]], year), lastrs[i]/100,
            lastrs[i]%100, NickOf(L, id[lastrg[i]], year) );
  }

  Ranking();
  Listing();

}

int AddResult(int a, int b, int x, int y, int z) {
  int more = 0;
  int i, oldr, ox, oy;

  if (updates==0) {
    if (lastrng>0) {
      if (z>(lastrd[0]%1000)+2) _rnd = 1000*(lastr+1)+z;
      else _rnd = 1000*lastr+z;
    }
  }

  if (tbr<10) {
    if (res[a][b] >= 0)  {
      int zi = rnd[a][b] % 1000;
      printf("[R%-2d] %2d %s: %s [%d-%d] %s   ... already exists\n", 
               rnd[a][b]/1000, zi%50, month[zi/50], 
               mnem[id[a]], res[a][b]/100, res[a][b]%100, mnem[id[b]]);
    }
  }
  else {
    oldr = 0;
    if (res[a][b] >= 0)  {
      oldr += 1;
    }
    if (res2[a][b] >= 0)  {
      int zi = rnd2[a][b] % 1000;
      printf("[R%-2d] %2d %s: %s [%d-%d] %s   ... already exists\n", 
               rnd2[a][b]/1000, zi%50, month[zi/50], 
               mnem[id[a]], res2[a][b]/100, res2[a][b]%100, mnem[id[b]]);
      oldr += 2;
    }
  }


  printf("[R%-2d] %2d %s: %s [%d-%d] %s \n", _rnd/1000, z%50, month[z/50], 
           mnem[id[a]], x, y, mnem[id[b]]);
  if (tbr<10 || res[a][b] < 0) {
    rnd[a][b] = 1000*(_rnd/1000)+z;
    res[a][b] = 100*x+y;
  }
  else if (res2[a][b] < 0) {
    rnd2[a][b] = 1000*(_rnd/1000)+z;
    res2[a][b] = 100*x+y;
  }

  if (100*x+y >= 0) {
    gsc[a] += x; gre[a] += y;
    gsc[b] += y; gre[b] += x;
    if (x>y) { win[a]++; los[b]++; pts[a] += ppv; }
      else if (x==y) { drw[a]++; drw[b]++; pts[a]++; pts[b]++; }
       else { los[a]++; win[b]++; pts[b] += ppv; }
//    printf("%s - %s  [%d-%d]  added  (round %2d, %s %2d)\n\n",
//           NameOf(L,id[a],year), NameOf(L,id[b],year),
//          x, y, _rnd/1000, month[z/50], z%50);
    updates++;
  }
  return 1;
}




int Extract(char *filename) {
  FILE *f = fopen(filename, "rt");
  if (f==NULL) return 2;
  char s[1024];
  int a, b, x, y, m, m1, z, z1, r, yr;
  char *tk, *tkw, *tkm, *tkd, *tkyr, *tks, *t1, *t2, *tkx, *tky;
  char home[64], guest[64], score[64], scr[64], dow[64];

  updates = 0;
  r = 0;
  while (!feof(f)) {
    fgets(s, 1024, f);
    if (strstr(s, "dynres")!=NULL) {
      fgets(s, 1024, f); // dow
      tk = strstr(s, "_day");
      if (tk==NULL) continue;
      else {
        tkw = tk+6;
        tk = strtok(tkw, "<");
        strcpy(dow, tkw);
      }

      fgets(s, 1024, f);
      if (strstr(s, "_date2")==NULL) continue;
      tk = strchr(s, '>');
      if (tk==NULL) continue;
      tk++;
      tkm  = strtok(tk, " ,\n");
      tkd  = strtok(NULL, ",<\n");
      tkyr = strtok(NULL, "<\n");
      if (tkm && strlen(tkm)>2)  m1 = Month(tkm); else m1 = 0;
      if (m1>0) m = m1;
      if (m1>0 && tkd && strlen(tkd)>0) z1  = atoi(tkd); else z1 = 0;
      if (z1>0) z = z1;
      if (m1==0 && tkm && tkd) {
        // reverse syntax
        m1 = Month(tkd);
        z1 = atoi(tkm);
        if (m1>0 && z1>0) {
          m = m1;
          z = z1;
        }
      }
      
      if (tkyr) yr = atoi(tkyr);

      char foo[4]; sprintf(foo, "%s", "/ >"); foo[1]='"';
      fgets(s, 1024, f);
      t1 = strstr(s, "/'>");
      if (t1==NULL) t1 = strstr(s, foo);
      if (t1==NULL) continue;
      t1 += 3;
      tk = strchr(t1, '<');
      if (tk==NULL) continue;
      tk[0] = 0;
      strcpy(home, t1);
      a = Find(home);
      if (a<0) printf("\033[31;1mCannot find team %s\033[0m\n", home);

      fgets(s, 1024, f);
      tk = strstr(s, "orange");
      if (tk!=NULL) continue;
      tk = strstr(s, "_score");
      if (tk==NULL) continue;      
      tks = strchr(tk, '>');
      if (tks==NULL) continue;
      tks++;
      tk = strchr(tks, '<');
      if (tk==NULL) continue;
      tk[0] = 0;
      strcpy(score, tks);
      strcpy(scr, tks);
      if (strcmp(score, "PSTP")==0) continue;
      if (strchr(score, ':')!=NULL) continue;
      tkx = strtok(scr, " -");
      tky = strtok(NULL, " -\n");
      if (tkx==NULL || tky==NULL) continue;
      x = atoi(tkx);
      y = atoi(tky);

      fgets(s, 1024, f);
      t2 = strstr(s, "/'>");
      if (t2==NULL) continue;
      t2 += 3;
      tk = strchr(t2, '<');
      if (tk==NULL) continue;
      tk[0] = 0;
      strcpy(guest, t2);
      b = Find(guest);
      if (b<0) printf("\033[31;1mCannot find team %s\033[0m\n", guest);
      if (a<0 || b<0) continue;

//      printf("\n[R%-2d] %2d %s, %s [%s] %s\n", _rnd/1000, z, month[m], home, score, guest);
//      printf("------------------------------------------------------------------\n");
      AddResult(a, b, x, y, 50*m+z);
    }
  }
}

int main(int argc, char **argv) {
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
  Extract(updatesfile);
  Ranking();
  Listing();
  Dump();
  return 1;  
}
