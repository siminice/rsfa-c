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
int  NC, r;

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

int Find(char* s) {
  for (int i=0; i<NC; i++)
    if (strcmp(mnem[i], s)==0) return i;
//  for (int i=0; i<NC; i++)
//    if (NULL != strstr(mnem[i], s)) return i;
  return -1;
}

int Extract(char *filename) {
  FILE *f = fopen(filename, "rt");
  if (f==NULL) return 2;
  char *s = new char[2048];
  int a, b, x, y, m, m1, z, z1, yr, aggr, he, ge, hp , gp;
  char *tk, *tkw, *tkm, *tkd, *tkyr, *tks, *tke, *t1, *t2, *tkx, *tky;
  char home[64], guest[64], score[64], scr[64], dow[64], u[64];

  aggr = 0;
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
        aggr = (strstr(dow, "Aggr")!=NULL);
        if (aggr>0) continue;
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
          m = m1-1;
          z = z1;
        }
      }
      
      if (tkyr) yr = atoi(tkyr)+1;

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
      if (a<0) printf("\033[31;1mCannot find team %s\033[0m...\n", home);

      fgets(s, 1024, f);
      tk = strstr(s, "orange");
      if (tk!=NULL) continue;
      tk = strstr(s, "_score");
      if (tk==NULL) continue;      
      tks = strchr(tk, '>');
      if (tks==NULL) continue;
      tks++;
      tke = strstr(tks, "</a");
      if (tke==NULL) continue;
      tke[0] = 0;
      strcpy(score, tks);
      strcpy(scr, tks);
//           printf("Chopped off score line: %s\n", score);
      if (strstr(score, "PSTP")!=NULL) continue;
      if (strchr(score, ':')!=NULL) continue;
      tkx = strtok(scr, "-");
      tky = strtok(NULL, "-\n");

      if (tkx==NULL || tky==NULL) continue;

//         printf("Pieces: %s | %s\n", tkx, tky);
      he = hp = ge = gp = 0;
      if (strstr(tkx, "span")!=NULL) {
        tk = strstr(tkx, "dyn");
        if (tk==NULL) continue;
        if (tk[3]=='h') he = hp = 0;
        else if (tk[6]=='E') he = 1;
        else if (tk[6]=='P') hp = 1;
        tk = strrchr(tkx, '>');
        tkx = tk+1;
      }
      if (strstr(tky, "span")!=NULL) {
        tk = strstr(tky, "dyn");
        if (tk==NULL) continue;
        if (tk[3]=='h') ge = gp = 0;
        else if (tk[6]=='E') ge = 1;
        else if (tk[6]=='P') gp = 1;
      }

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
      if (b<0) printf("\033[31;1mCannot find team %s\033[0m...\n", guest);
      if (a<0 || b<0) continue;

//      AddResult(a, b, x, y, 50*m+z);
                u[0]  = 48 + (yr - 1870)/75;
                u[1]  = 48 + (yr - 1870)%75;
                u[2]  = (char) 48+m;
                u[3]  = (char) 48+z;
                u[4]  = 'K';
                u[5]  = ' ';
                u[6]  = (char) 48+r;
                u[7]  = (char) 48+a/75;
                u[8]  = (char) 48+a%75;
                u[9]  = (char) 48+b/75;
                u[10] = (char) 48+b%75;
                u[11] = (char) 48+x;
                u[12] = (char) 48+y;
                if (he+hp+ge+gp==0) u[13] = 0;
                else {
                  if (he+ge>0) u[13] = 'e';
                  else if (hp>0) u[13] = '+';
                  else if (gp>0) u[13] = '-';
                  u[14] = 0;
                }
                if (aggr==0) printf("%s\n", u);      
    }
  }
}

int main(int argc, char **argv) {
  char updatesfile[64];

  Load();
  strcpy(updatesfile, "./html/cup-results.html");
  if (argc > 1) { 
    strcpy(updatesfile, argv[1]);
  }
  if (argc > 2) { 
    r = atoi(argv[2]);
  }
  Extract(updatesfile);
  return 1;  
}
