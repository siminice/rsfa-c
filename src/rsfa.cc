#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include "rsfa.hh"

char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


// Linux terminal colors
const char *normal  = "\033[0m";
const char *black   = "\033[30m";
const char *red     = "\033[31m";
const char *green   = "\033[32m";
const char *yellow  = "\033[33m";
const char *blue    = "\033[34m";
const char *magenta = "\033[35m";
const char *cyan    = "\033[36m";
const char *white   = "\033[37m";
const char *loblack   = "\033[30;0m";
const char *lored     = "\033[31;0m";
const char *logreen   = "\033[32;0m";
const char *loyellow  = "\033[33;0m";
const char *loblue    = "\033[34;0m";
const char *lomagenta = "\033[35;0m";
const char *locyan    = "\033[36;0m";
const char *lowhite   = "\033[37;0m";
const char *hiblack   = "\033[30;1m";
const char *hired     = "\033[31;1m";
const char *higreen   = "\033[32;1m";
const char *hiyellow  = "\033[33;1m";
const char *hiblue    = "\033[34;1m";
const char *himagenta = "\033[35;1m";
const char *hicyan    = "\033[36;1m";
const char *hiwhite   = "\033[37;1m";

const char *colors[] = {black, red, green, yellow, blue, magenta, cyan, white};
const char *locols[] = {loblack, lored, logreen, loyellow, loblue, lomagenta, locyan, lowhite};
const char *hicols[] = {hiblack, hired, higreen, hiyellow, hiblue, himagenta, hicyan, hiwhite};

/*****************************************************************
*****               Aliases class                            *****
******************************************************************/


alias_data::alias_data(int y, char *s, char *n) {
  year = y;  
  name = (char*) malloc(strlen(s)+1);
  strcpy(name, s);
  if (n != NULL) {
    nick = (char*) malloc(strlen(n)+1);
    strcpy(nick, n);
  }
  else nick = NULL;
};

alias_data::~alias_data() {
  if (name) delete name;
  if (nick) delete nick;
};
  
alias_node::alias_node(alias_data *a, alias_node *n) {
  data = a;
  next = n;
};
  
alias_node::~alias_node() {
  if (next) delete next;
  delete data;
};
  

//-------------------------------------

Aliases::Aliases() {
  list = NULL;
};
  
Aliases::~Aliases() {
 delete list;
} 

void Aliases::Append(alias_data *a) {
  alias_node *n = (alias_node*) malloc(sizeof(alias_node));
  n->data = a;
  n->next = list;
  list = n;
};
  
char* Aliases::GetName(int y) {
  if (!list) return NULL;
  alias_node *n = list;
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
  alias_node *n = list;
  char *s = n->data->nick;
  int x = n->data->year;
  while (y < x && n->next != NULL) {
    n = n->next;
    s = n->data->nick;
    x = n->data->year;
  }
  return s;
} 

/*****************************************************************
*****                   Fed class                            *****
******************************************************************/

int Fed::Load(char *filename, int style) {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen(filename, "rt");
  if (f==NULL) return 0;

  fscanf(f, "%d\n", &NC);
  club = new char*[NC];;
  mnem = new char*[NC];

  for (int i=0; i<NC; i++) {
    fgets(s, 60, f);
    s[strlen(s)-1] = 0;
    if (style==FIXED) {
      mnem[i] = new char[16];
      club[i] = new char[32];
      memmove(mnem[i], s, 15); mnem[i][15] = 0;
      for (int j=0; j<30; j++) club[i][j] = 32;
      memmove(club[i], s+15, 30);
    }
    else {
      tok[0] = strtok(s, ",");
      tok[1] = strtok(NULL, "\n,");
      int l1 = strlen(tok[0]);
      int l2 = strlen(tok[1]);
      mnem[i] = new char[l1+1];
      club[i] = new char[l2+1];
      strcpy(mnem[i], tok[0]);
      strcpy(club[i], tok[1]);
    }
  }
  fclose(f);

  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  f = fopen("alias.dat", "rt");
  if (!f) return 0;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    if (!s) continue; 
    if (strlen(s) < 3) continue;
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
      alias_data *a = new alias_data(y, name, nick); 
      L[i]->Append(a);
      k++;
    }
    s[0] = 0;
  }
  fclose(f);
  return 1;
}

   
//---------------------------------------------
char* Fed::NameOf(int t, int y) {
  char *s = L[t]->GetName(y);
  if (!s) return club[t];   
  return s;
}

char* Fed::NickOf(int t, int y) {
  char *s = L[t]->GetNick(y);
  if (!s) return mnem[t];   
  return s;
}

//---------------------------------------------
int Fed::Find(char *s) {
  int found = 0;
  int multi = 0;
  int j;
  int l = strlen(s);

  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  for (int i=0; i<l-1; i++)
    if ((s[i]==32 || s[i]=='.') && s[i+1]>96) s[i+1] -= 32;

  // Exact match
  int i = 0;
  while (i < NC) {
    if (strcmp(mnem[i], s)==0) return i;
    i++;
  }

  // Substring
  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
    else i++;
  }
  if (found) return i;

  // try uppercase all
  for (int j=0; j<l; j++)
    if (s[j]>='a' && s[j]<='z') s[j] -= 32;
  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], s)) found = 1;
    else i++;
  }
  if (found) return i;
  return UNKNOWN;
}

int Fed::GetUnique(char *prompt) {
  char name[30];
  int res;
  do {
   printf("%s", prompt);
   do { gets(name); } while (!strlen(name));
   res = Find(name);
  } while (res < 0);
  return res;
}


/*****************************************************************
*****               History class                            *****
******************************************************************/

History::History(Fed *f) {
  F = f;
  part = NULL;
  med = NULL;
}

void History::Collect() {
  char s[100], *dv, *pl, *yr, *suf;

  FY   = new int[MAX_LEVELS];
  LY   = new int[MAX_LEVELS];
  MAX  = new int [MAX_LEVELS];
  part = new int***[MAX_LEVELS];

  ND = 0;
  for (int d=0; d<MAX_LEVELS; d++) {
    MAX[d] = 0;
    FY[d] = 3000;
    LY[d] = 1000;
  }

  int t, d, p, y;
  DIR *dp;
  struct dirent *ep;
  dp = opendir("./");
  if (dp != NULL) {
      while (ep = readdir (dp)) {
        strcpy(s, ep->d_name);
        dv  = strtok(s, ".");
        yr  = strtok(NULL, ".");
        suf = strtok(NULL, ".");
        if (dv!=NULL && yr!=NULL && suf==NULL) {
          int l = strlen(dv);
          if (l==0 || l>3) d=-1;
          d = ((int) s[0]) - 97;
          if (d<0 || d>=MAX_LEVELS) d=-1;
          if (d+1>ND) ND = d+1;
          if (l>1) p = atoi(dv+1); else p = 1;
          y = atoi(yr);
          if (d>=0 && y>1888 && y<2100) {
            if (p>MAX[d]) MAX[d] = p;
            if (y<FY[d])  FY[d]  = y;
            if (y>LY[d])  LY[d]  = y;
          }
        }
      }
      closedir(dp);
      ND = MAX_LEVELS;
      while (ND>0 && MAX[ND-1]==0) ND--;
  }
  else
   printf("ERROR: Couldn't open the directory.\n");

  // Allocate
  for (int d=0; d<ND; d++) {
    if (MAX[d]>0) {
      part[d] = new int**[LY[d]-FY[d]+1];
      for (int i=FY[d]; i<=LY[d]; i++) {
        part[d][i-FY[d]] = new int*[MAX[d]+1];
        for (int j=0; j<=MAX[d]; j++) {
          part[d][i-FY[d]][j] = new int[MAXLSIZE];
        }
      }
    }
  }
}


//---------------------------------------------
void History::Build() {
  FILE *f, *h;
  char *filename = new char[15];
  char *outfile  = new char[15];

  if (part==NULL) Collect();
  for (int d=0; d<ND; d++)
    printf("%c: %d - %d (max: %d)\n", (char) (d+65), FY[d], LY[d], MAX[d]);

  for (int d=0; d<ND; d++) {
    printf("Compiling level %c...\n", (char) (d+65));
    if (MAX[d]>-100) {
    sprintf(outfile,  "part.%c", (char)(d+97));
    f = fopen(outfile, "wt");
    for (int y=FY[d]; y<=LY[d]; y++) {
      sprintf(filename, "%c.%d", (char) (d+97), y);
      h = fopen(filename, "rt");
      if (h==NULL) {
         fprintf(f, "%d 0\n", y);
         continue;
      }
      fclose(h);

      League *L = new League(F);
      L->Load(filename);
      L->Ranking();
      fprintf(f, "%d %d ", y, L->n);
      for (int j=0; j<L->n; j++) fprintf(f, "%3d ", L->id[L->rank[j]]);
      fprintf(f, "\n");
      delete L; 
    }
    fclose(f);

    for (int i=1; i<=MAX[d]; i++) {
      sprintf(outfile, "part.%c%d", (char) (d+97), i);
      f = fopen(outfile, "wt");
      for (int y=FY[d]; y<=LY[d]; y++) {
        sprintf(filename, "%c%d.%d", (char) (d+97), i, y);
        h = fopen(filename, "rt");
        if (h==NULL) {
          fprintf(f, "%d 0\n", y);
          continue;
        }
        fclose(h);

        League *L = new League(F);
        L->Load(filename);
        L->Ranking();
        fprintf(f, "%d %d ", y, L->n);
        for (int j=0; j<L->n; j++) fprintf(f, "%3d ", L->id[L->rank[j]]);
        fprintf(f, "\n");
        delete L;
      }
      fclose(f);
    }
  }
 }
 delete[] filename;
 delete[] outfile;  
}

//---------------------------------------------
void History::Load() {
  char *filename = new char[20];
  char s[250];
  FILE *f;
  int dummy, n, t;

  if (part==NULL) Collect();  

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
          }
          fgets(s, 200, f);
        }
      }
      if (f) fclose(f);
    }
  }
  
}


//---------------------------------------------
void History::CountMedals(int nmed, int count_last) {
  int num;
  printf("\n");
  med = new int*[F->NC];
  for (int t=0; t<F->NC; t++) {
    med[t] = new int[nmed];
    for (int j=0; j<nmed; j++) med[t][j] = 0;
  }
  for (int y=FY[0]; y<=LY[0]+count_last-1; y++) {
    num = part[0][y-FY[0]][0][1];
    if (num>=3) {
      printf("%4d: %-15s %-15s %-15s\n", y,
        F->NickOf(part[0][y-FY[0]][0][2],y),
        F->NickOf(part[0][y-FY[0]][0][3],y),
        F->NickOf(part[0][y-FY[0]][0][4],y)
      );
    }
    else if (num>=2) {
      printf("%4d: %-15s %-15s\n", y,
        F->NickOf(part[0][y-FY[0]][0][2],y),
        F->NickOf(part[0][y-FY[0]][0][3],y)
      );
    }
    for (int j=0; j<(nmed>num?num:nmed); j++)
      med[part[0][y-FY[0]][0][j+2]][j]++;
  }
}
  
//---------------------------------------------
int History::MedSup(int x, int y, int nmed) {
  for (int i=0; i<nmed; i++) {
    if (med[x][i] > med[y][i]) return 1;
    if (med[x][i] < med[y][i]) return 0;
  }
  return x < y;
}

void History::ShowMedals(int nmed) {
  // medal ranking
  int *rank = new int[F->NC];
  for (int i=0; i<F->NC; i++) rank[i] = i;
  int sorted = 1;
  do {
    sorted = 1;
    for (int i=0; i<F->NC-1; i++)
      if (MedSup(rank[i+1], rank[i], nmed)) {
        sorted = 0;
        int aux = rank[i]; rank[i] = rank[i+1]; rank[i+1] = aux;
      }
  } while (!sorted);

  // medal listing
  printf("\n");
  for (int i=0; i<F->NC; i++) {
    int x = rank[i];
    int sum = 0;
    for (int j=0; j<nmed; j++) sum += med[x][j];
    if (sum) {
      printf("%2d.%-30s", i+1, F->NameOf(x,2100));
      for (int j=0; j<nmed; j++) printf(" %2d%s",  med[x][j], j==2?"  ":"");
      printf("\n");
    }
  }  
  if (rank) delete[] rank;
}

//---------------------------------------------
int History::In(int y, int d, int p, int t) {
  if (MAX[d]==0) return 0;
  if (y<FY[d] || y>LY[d]) return 0;
  int m = part[d][y-FY[d]][p][1];
  for (int j=2; j<m+2; j++)
    if (part[d][y-FY[d]][p][j] == t) return j-1;
  return -1;
}

void strdiv(int d, char *s) {
  if (d>=0) {
    if (d%100>0) sprintf(s, "%s%c%2d", hicols[7-d/100], (char) (d/100+65), d%100);
    else sprintf(s, "%s%c  ", hicols[7-d/100], (char) (d/100+65));
  }
  else sprintf(s, " - ");
}

void History::Head2Head(int a, int b) {
  
}

History::~History() {
  if (med)  delete[] med;
}

/*****************************************************************
*****                League class                            *****
******************************************************************/

League::League(Fed *f) {
  F = f;
}

//---------------------------------------------
void League::Reset() {
  for (int i=0; i<n; i++) {
    win[i] = drw[i] = los[i] = 0;
    gsc[i] = gre[i] = pts[i] = 0;
    pen[i] = pdt[i] = 0;
    for (int k=0; k<numr; k++) {
      for (int j=0; j<n; j++) {
        res[k][i][j] = rnd[k][i][j] = UNKNOWN;
      }
    }
  }
  hla = hlb = UNKNOWN;
  fd = ld = UNKNOWN;
  lastr = 650;
}

//---------------------------------------------
void League::Clear() {
  Reset();
  for (int k=0; k<numr; k++) {
    for (int i=0; i<n; i++) {
      if (res[k][i]) delete[] res[k][i];
      if (rnd[k][i]) delete[] rnd[k][i];
    }
    if (res[k]) delete[] res[k];
    if (rnd[k]) delete[] rnd[k];
  }
  for (int i=0; i<n; i++) if (desc[i]) delete[] desc[i];

  if (infile) delete[] infile;
  if (T)      delete[] T;
  if (tbwin)  delete[] tbwin;
  if (tbdrw)  delete[] tbdrw;
  if (tblos)  delete[] tblos;
  if (tbgre)  delete[] tbgsc;
  if (tbgsc)  delete[] tbgre;
  if (tbrk)   delete[] tbrk;
  if (res)    delete[] res;
  if (rnd)    delete[] rnd;
  if (rank)   delete[] rank;
 
  infile = NULL;
  tbwin = tbdrw = tblos = tbgsc = tbgre = tbrk = NULL;
  res = rnd = NULL;
  rank = NULL;
}

//---------------------------------------------
int League::Find(char* s) {
  int found = 0;
  int l;

  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  l = strlen(s);
  for (int j=0; j<l-1; j++)
    if ((s[j]==32 || s[j]=='.') && s[j+1]>96) s[j+1] -= 32;

  // Substring
  int i = 0;
  while (i < n && !found) {
    if (NULL != strstr(F->mnem[id[i]], s)) found = 1;
    else i++;
  }
  if (found) return i;

  // try uppercase
  for (int j=0; j<l; j++)
    if (s[j]>='a' && s[j]<='z') s[j] -= 32;
  i = 0;
  while (i < n && !found) {
    if (NULL != strstr(F->mnem[id[i]], s)) found = 1;
    else i++;
  }
  if (found) return i;
  return -1;
}

//---------------------------------------------
int League::GetUnique(char *prompt) {
  char name[30];
  int res;
  do {
   printf("%s", prompt);
   do { gets(name); } while (!strlen(name));
   res = Find(name);
  } while (res < 0);
  return res;
}

//---------------------------------------------
int League::sup(int i, int j, int tb=0) {
  if (tb%NUMORD==HEAD_TO_HEAD) {  
    int p1 = ppv*tbwin[i]+tbdrw[i];
    int p2 = ppv*tbwin[j]+tbdrw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (tbgsc[i] - tbgre[i] > tbgsc[j] - tbgre[j]) return 1;
    if (tbgsc[i] - tbgre[i] < tbgsc[j] - tbgre[j]) return 0;
    if (tbgsc[i] > tbgsc[j]) return 1;
    if (tbgsc[i] < tbgsc[j]) return 0;
    if (tbwin[i] > tbwin[j]) return 1;
    if (tbwin[i] < tbwin[j]) return 0;
    return sup(i, j, DEFAULT);
  }
  else if (tb%NUMORD==RATIO) {  
    int p1 = ppv*win[i]+drw[i];
    int p2 = ppv*win[j]+drw[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    double gav1 = 0.0;
    if (gre[i]>0) gav1 = (double) gsc[i]/ (double) gre[i];
    double gav2 = 0.0;
    if (gre[j]>0) gav2 = (double) gsc[j]/ (double) gre[j];
    if (gav1>gav2) return 1;
    if (gav1<gav2) return 0;
    if (gsc[i] > gsc[j]) return 1;
    if (gsc[i] < gsc[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (i > j) return 0;
  }
  else {
    int p1 = pts[i];
    int p2 = pts[j];
    if (pen[i]!=0 && after(pdt[i],lastr)) p1 -= pen[i];
    if (pen[j]!=0 && after(pdt[j],lastr)) p2 -= pen[j];
    if (p1 > p2) return 1;
    if (p1 < p2) return 0;
    if (gsc[i] - gre[i] > gsc[j] - gre[j]) return 1;
    if (gsc[i] - gre[i] < gsc[j] - gre[j]) return 0;
    if (gsc[i] > gsc[j]) return 1;   
    if (gsc[i] < gsc[j]) return 0;
    if (win[i] > win[j]) return 1;
    if (win[i] < win[j]) return 0;
    if (i > j) return 0;
    return 1;
  }
}
  
//---------------------------------------------
void League::Tiebreak(int h, int k) {
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
      for (int r=0; r<numr; r++) {
        if (res[r][t1][t2] >= 0) {
          gm++;
          int x = res[r][t1][t2] / 100;
          int y = res[r][t1][t2] % 100;
          tbgsc[t1] += x; tbgre[t2] += x;
          tbgsc[t2] += y; tbgre[t1] += y;
          if (x>y) {tbwin[t1]++; tblos[t2]++;}
          else if (x==y) {tbdrw[t1]++; tbdrw[t2]++;}
          else {tbwin[t2]++; tblos[t1]++;}
        }
      }
    }
  }

  int sorted; 
  do {
    sorted = 1;
    for (int i=h; i<k; i++)
      if (sup(tbrk[i+1], tbrk[i], (tbr%NUMORD!=RATIO ? gm>=(k-h+1)*(k-h) : RATIO))) {  
        sorted = 0;
        int aux = tbrk[i];
        tbrk[i] = tbrk[i+1];
        tbrk[i+1] = aux;
      }
  } while (!sorted);
  for (int i=h; i<=k; i++)
    rank[i] = tbrk[i];
}
    

//---------------------------------------------
void League::Ranking() {
  // BubbleSort
  int i, j;
  for (i=0; i<n; i++) rank[i] = i;
  int sorted;

  if (ngr>0) {
    int nn = (n+ngr-1)/ngr;
    do {
      sorted = 1;
      for (int g=0; g<ngr; g++) {
        for (i=0; i<(g==ngr-1?n-g*nn-1:nn-1); i++) {
          if (sup(rank[g*nn+i+1], rank[g*nn+i], (tbr%NUMORD!=RATIO? 0 : tbr))) {
            sorted = 0;
            int aux = rank[g*nn+i];
            rank[g*nn+i] = rank[g*nn+i+1];
            rank[g*nn+i+1] = aux;
          }
        }
      }
    } while (!sorted);
    return;
  }

  do {
    sorted = 1;
    for (i=0; i<n-1; i++)
      if (sup(rank[i+1], rank[i], (tbr%NUMORD==RATIO? RATIO : DEFAULT))) {
        sorted = 0;
        int aux = rank[i];
        rank[i] = rank[i+1];
        rank[i+1] = aux;
      }
  } while (!sorted);
  if (tbr%NUMORD==HEAD_TO_HEAD) {
   i = 0;
    while (i<n-1) {
      j = i;
      while (j+1<n && (pts[rank[j+1]]-pen[rank[j+1]] == pts[rank[i]]-pen[rank[i]])) j++;
      if (j>i) Tiebreak(i,j);  
      i = j+1;
    }
  }
}

//---------------------------------------------
void League::Listing() {
//  printf("\nStandings:\n");
  printf("\n");
  for (int i=0; i<n; i++) {
    int x = rank[i];
    if (x==hla || x==hlb) printf("\033[1m");
    int grnk = i+1;
    if (ngr>1) grnk = i%((n+ngr-1)/ngr)+1;
    if (win[x]+drw[x]+los[x]==0 && win[rank[0]]+drw[rank[0]]+los[rank[0]]>0) 
      printf("%2d.%-30s\n", i+1, F->NameOf(id[x],year));
    else {
     printf("%2d.%-30s%2d%3d%3d%3d%4d%c%2d", grnk,
             F->NameOf(id[x],year), win[x]+drw[x]+los[x], 
             win[x], drw[x], los[x], gsc[x], (tbr%NUMORD==RATIO?':':'-'), gre[x]);
     if (tbr%NUMORD==RATIO) printf(" %4.3f ", (gre[x]>0?((double)gsc[x])/((double)gre[x]):gsc[x]));
     if (pen[x]>0 && after(pdt[x],lastr)) printf("%3d (-%dp pen)\n" , pts[x]-pen[x], pen[x]);
     else if (pen[x]<0 && after(pdt[x],lastr)) printf("%3d (+%dp bonus)\n" , pts[x]-pen[x], -pen[x]);
     else printf("%3d", pts[x]);
     printf(" %s\n", desc[x]);
    }
    if (ngr==0) {
     if (i==promo1-1)
        printf("------------------------------------------------------\n");
     if (i==promo1+promo2-1 && promo2>0) 
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-(releg1+releg2)-1 && releg2>0) 
        printf(" - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
     if (i==n-releg1-1 && releg1>0)
        printf("------------------------------------------------------\n");
    }
    else if (ngr>1) {
     if (i%((n+ngr-1)/ngr)==promo1/ngr-1)
        printf("------------------------------------------------------\n");
    }

    if (ngr>1 && (i+1)%((n+ngr-1)/ngr)==0) {
      printf("\n");
    }
  }
  printf("\n");
}

//---------------------------------------------
int League::AddResult(int a, int b, int x, int y) {
  int k = 0;
  do {
    if (res[k][a][b] == UNKNOWN) res[k][a][b] = 100*x+y;
    else k++;
  } while (k<numr && res[k][a][b]==UNKNOWN);
  if (k>=numr) return 0;

  gsc[a] += x; gre[a] += y;
  gsc[b] += y; gre[b] += x;
  if (tbr%NUMORD==GDIFF2) {
    if (x-y>=2) pts[a]++;
    if (y-x>=2) pts[b]++;
  }
  if (x>y) { 
    win[a]++; los[b]++; pts[a] += ppv; 
  }
  else if (x==y) { 
    drw[a]++; drw[b]++; pts[a]++; pts[b]++; 
    if (tbr==DRAW10 && drw[a]>10) pts[a]--;
    if (tbr==DRAW10 && drw[b]>10) pts[b]--;
    if (tbr==DRAW8 && drw[a]>8) pts[a]--;
    if (tbr==DRAW8 && drw[b]>8) pts[b]--;
  }
  else { 
    los[a]++; win[b]++; pts[b] += ppv; 
    if (tbr%NUMORD==AWAY3) pts[b]++;
  }
  return 1;
}

//---------------------------------------------
int League::DelResult(int a, int b, int x, int y) {
  gsc[a] -= x; gre[a] -= y;
  gsc[b] -= y; gre[b] -= x;
  if (x>y) { win[a]--; los[b]--; pts[a] -= ppv; }
    else if (x==y) { drw[a]--; drw[b]--; pts[a]--; pts[b]--; }
     else { los[a]--; win[b]--; pts[b] -= ppv; }
  return 1;
}

//---------------------------------------------
int League::InteractiveAdd(int a, int b, int x, int y, int z) {
  return 0;
}

//---------------------------------------------
int League::Alloc(char *filename) {
  FILE *f;
  int i, j, k, x, y, r, z;
  format = REGULAR;
  ngr = 1;
  f = fopen(filename, "rt");
  if (NULL == f) { 
     printf("File %s not found.\n", filename); 
     n = 0;
     return 0;
     infile = NULL;
  }
  infile = new char[strlen(filename)+1];
  strcpy(infile, filename);
  char syr[100];
  strcpy(syr, filename);
  char *tkd = strtok(syr, ".");
  char *tky = strtok(NULL, ".");
  char *tkl = strtok(tkd, "-");
  char *tkf = strtok(NULL, ".-");
  if (tky) year = atoi(tky); else year = 0;
  // Loading file
  char s[500], *tok[20];
  fscanf(f, "%d %d %d %d %d %d %d\n", &n, &ppv, &tbr, &promo1, &promo2, &releg1, &releg2);

  id  = new int[n];
  win = new int[n];
  drw = new int[n];
  los = new int[n];
  gsc = new int[n];
  gre = new int[n];
  pts = new int[n];
  pen = new int[n];
  pdt = new int[n];
  desc = new char*[n];

  tbwin = new int[n];
  tbdrw = new int[n];
  tblos = new int[n];
  tbgsc = new int[n];
  tbgre = new int[n];
  tbrk  = new int[n];

  numr = tbr/ROUNDS + 1;
  rnd = new int**[numr];
  res = new int**[numr];
  rank = new int[n];
  for (i=0; i<numr; i++) {
    rnd[i] = new int*[n];
    res[i] = new int*[n];
  }
  for (i=0; i<n; i++) {
    for (k=0; k<numr; k++) res[k][i] = new int[n];
    for (k=0; k<numr; k++) rnd[k][i] = new int[n];
    fgets(s, 200, f);
    tok[0] = strtok(s, " ");
    for (int k=1; k<12; k++) tok[k] = strtok(NULL, " \n");
    id[i]  = atoi(tok[0]);
    win[i] = atoi(tok[2]);
    drw[i] = atoi(tok[3]);
    los[i] = atoi(tok[4]);
    gsc[i] = atoi(tok[5]);
    gre[i] = atoi(tok[6]);
    pts[i] = atoi(tok[7]);   
    if (tok[8]) pen[i] = atoi(tok[8]); else pen[i] = 0;
    if (tok[9]) pdt[i] = atoi(tok[9]); else pdt[i] = 0;
    if (tok[10]) {
      desc[i] = new char[strlen(tok[10])+1];
      strcpy(desc[i], tok[10]); 
    }
    else {
      desc[i] = new char[1]; desc[i][0] = 0;
    }
  }

  int frn = 1000;
  int frd = 1000;
  for (k=0; k<numr; k++) {
   for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      fscanf(f, "%d %d", &r, &z);
      rnd[k][i][j] = r;
      res[k][i][j] = z;
      int tr = rnd[k][i][j]/1000;
      int td = rnd[k][i][j]%1000;
      if (rnd[k][i][j]>0 && tr==0 && frn!=1 && td < frd) frd = td;
      if (tr==1 && td<frd) {frd = td; frn = 1;}
    }
    fscanf(f, "\n");
   }
  }
  fclose(f);

  fd = frd;
  ld = fd + 649;
  hla = hlb = UNKNOWN;
  lastr = 650;
  return 1;
}

//---------------------------------------------
int League::Load(char *filename) {
  int r = Alloc(filename);
  return r;
}

//---------------------------------------------
void League::ListMostRecent() {
  int lastrnd, lastr, lastrh[16], lastrg[16], lastrd[16], lastrs[16];
  int storethis;
  int lastrng = 0;
  time_t  tt;
  struct  tm* t = new tm;
  time(&tt);
  t = localtime(&tt);
  int today = (t->tm_mon +1)*50 + t->tm_mday;
  lastrnd = -1;
  for (int i=0; i<n; i++) {
    for (int j=0; j<n; j++) {
      for (int k=0; k<2; j++) {
      if (rnd[k][i][j]>50) {
        int thisd = rnd[k][i][j]%1000;
        int storethis = 0;
        int dl = abs(today-lastrnd);
        int dc = abs(today-thisd);
        if (dc < dl) {
          lastrng = 0;
          storethis = 1;
          lastrnd = thisd;
          lastr = rnd[k][i][j]/1000;
        }
        else if (dc == dl) { storethis = 1; }
        if (storethis) {
          lastrh[lastrng] = i;
          lastrg[lastrng] = j;
          lastrd[lastrng] = thisd;
          lastrs[lastrng] = res[k][i][j];
          lastrng++;
          lastr = rnd[k][i][j]/1000;
        }
      }
     }
    }
  }
}

//---------------------------------------------
int League::Save(char *filename) {
  FILE *f;
  int i, j, k;
  char *outputfile = new char[strlen(filename)+5];
  strcpy(outputfile, filename);
  strcat(outputfile, ".old");
  rename(filename, outputfile);
  f = fopen(filename, "wt");
  fprintf(f, "%d %d %d %d %d %d %d\n", n, ppv, tbr, promo1, promo2, releg1, releg2);
  for (i=0; i<n; i++) {  
    fprintf(f, "%4d%4d%4d%4d%4d%4d%4d%4d",
      id[i],  win[i]+drw[i]+los[i], 
      win[i], drw[i], los[i], gsc[i], gre[i], pts[i]);
    if (pen[i]!=0) fprintf(f, "%4d%4d", pen[i], pdt[i]);
    if (strlen(desc[i])>0) {
      if (pen[i]==0)  fprintf(f, "%4d%4d", pen[i], pdt[i]);
      fprintf(f, " %s", desc[i]);
    }
    fprintf(f, "\n");
  }
  for (k=0; k<numr; k++) {
   for (i=0; i<n; i++) {
    for (j=0; j<n; j++)
      fprintf(f, "%6d%5d", rnd[k][i][j], res[k][i][j]);
    fprintf(f, "\n");
   }
  }
  fclose(f);
}

//---------------------------------------------
void League::Synoptical() {
  char *symbol = new char[4];
  symbol[3] = 0;
  printf("\n                ");
  for (int i=0; i<n; i++) {
    memmove(symbol, F->NickOf(id[rank[i]], year), 3);
    printf("%3s ", symbol);
  }
  printf("\n");
  for (int k=0; k<numr; k++) {
   for (int i=0; i<n; i++) {
    printf("%-15s", F->NickOf(id[rank[i]], year));
    for (int j=0; j<n; j++) {
      int x = res[k][rank[i]][rank[j]];
      if (i==j) printf(" xxx");
       else if (x >=0)
         printf("%2d-%d", x/100, x%100);
        else printf("  - ");
    }
    printf("\n");
   }
  }
}

//---------------------------------------------
void League::Season(int det, int ifd, int ild) {
  int ng;
  lastr = UNKNOWN;
  int tfd = ifd;
  int tld = ild;
  if (tfd==DEFAULT_FIRST_DATE) tfd = 1;
  if (tld==DEFAULT_LAST_DATE) tld = tfd + 649;
  League *A = new League(F);
  if (!A->Load(infile)) return;
  Reset();
  for (int d=tfd; d<=tld; d++) {
    int r = d%650;
    ng = 0;
    for (int k=0; k<numr; k++) {
     for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        if (A->rnd[k][i][j]%1000==r && A->res[k][i][j]>=0) {
          if (ng==0) {
            if (consecutive(r,lastr)) {
              if (det>=DETAILED) { Ranking(); Listing(); }
              else printf("\n");
            }
            if (A->rnd[k][i][j]/1000>0) printf("Round %d ", A->rnd[k][i][j]/1000);
            printf("[%s %d]\n", month[r/50], d%50);
          }
          AddResult(i, j, A->res[k][i][j]/100, A->res[k][i][j]%100);
          printf("%-15s%d-%d %s\n", F->NickOf(id[i], year), A->res[k][i][j]/100,
                  A->res[k][i][j]%100, F->NickOf(id[j], year));
          ng++;
        }
      }
     }
    }
    if (ng>0) lastr = r;
  }

  // Full results but unknown dates
  if (win[0]+drw[0]+los[0]==0) {
   for (int k=0; k<numr; k++) {
    for (int i=0; i<n; i++) {
     for (int j=0; j<n; j++) {
        if (A->res[k][i][j]>UNKNOWN && A->rnd[k][i][j]==UNKNOWN)
          AddResult(i,j, A->res[k][i][j]/100, A->res[k][i][j]%100);
     }
    }
   }
  }

  Ranking();
  Listing();
  if (det>=DETAILED) Synoptical();
  delete A;
}

/*****************************************************************
*****               Utilities                                *****
******************************************************************/
 
int after(int dt, int r) {
  if (dt==0) return 1;
  if (dt>400) {
    if (r<400) return 1;
    else return (dt<=r);
  }
  if (r>400) return 0;
  else return (dt<=r);
}

int consecutive(int r, int last) {
  if (last%50>=30 && r%50==1) return 0;
  if (last>0 && (r-last>1 || r<last)) return 1;
  return 0;
}


