#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int NC;

char **club;
char **mnem;

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
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  L = new Aliases*[NC];
  for (int i=0; i<NC; i++) L[i] = new Aliases;
  for (int i=0; i<NC; i++) {
   fgets(s, 60, f);
   s[strlen(s)-1] = 0;
   mnem[i] = new char[16];
   club[i] = new char[32];
   memmove(mnem[i], s, 15);
   for (int j=0; j<30; j++)
     club[i][j] = 32;
   memmove(club[i], s+15, 30);
//   printf("%3d.%-30s [%-15s] extracted...\n", i, club[i], mnem[i]);
  }
  fclose(f);
  f = fopen("alias.dat", "rt");
  if (!f) return;
  for (int i=0; i<NC; i++) {
    if (feof(f)) continue;
    fgets(s, 2000, f);
    if (!s) continue;
    if (strlen(s)<3) continue;
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
    

int Find(char* s) {
  int i, j;
  
  if (s[0] > 96) s[0] -= 32; // start with capital letter;
  for (int i=0; i<strlen(s)-1; i++)
    if ((s[i]==32 || s[i]=='.') && s[i+1]>96) s[i+1] -= 32;
  for (i=0; i<NC; i++) { if (strcmp(mnem[i], s)==0) return i; }
  for (i=0; i<NC; i++) { if (strstr(mnem[i], s)==mnem[i]) return i; }
  // try all capitalized
  j=0; while (j<strlen(s) && s[j]!=' ' && s[j]!='.') {
   if (s[j]>='a' && s[j]<='z') s[j]-=32;
   j++;
  }
  for (i=0; i<NC; i++) { if (strcmp(mnem[i], s)==0) return i; }
  for (i=0; i<NC; i++) { if (strstr(mnem[i], s)==mnem[i]) return i; }
  return -1;
}

int GetUnique(const char *prompt) {
  char name[32];
  int res;
  do {
   printf("%s", prompt);
   do { fgets(name, 32, stdin); } while (strlen(name)<2);
   name[strlen(name)-1] = 0;
   res = Find(name);
  } while (res < 0);
  return res;
}

int main(int argc, char* argv[]) {
  FILE *f;
  char s[64], u[32], score[12];
  int y, z, r, h, g, sc, opt;
  Load();

  opt = 1;
  y = 2000; r = 1; z = 51;
  while (opt) {
    printf("[1] Add [2] Year (%d) [3] Date (%d %s) [4] Round (%d) [0] Exit\n",
            y, z%50, month[(z/50-1)%12+1], r);
    scanf("%d", &opt);
    switch(opt) {
      case 1:
        do {
          h = GetUnique("Home: ");
        } while (h<0);
        printf("  %s\n", NameOf(L, h, y));
        do {
          g = GetUnique("Guest: ");
        } while (g<0);
        printf("  %s\n", NameOf(L, g, y));
        printf("Score: "); //scanf("%d", &sc);
        scanf("%s", score); score[8] = 0;
                u[0]  = 48 + (y - 1870)/75;
                u[1]  = 48 + (y - 1870)%75;
                u[2]  = (char) 48+(z/50);
                u[3]  = (char) 48+(z%50);
                u[4]  = 'K';
                u[5]  = ' ';
                u[6]  = (char) 48+r;
                u[7]  = (char) 48+h/75;
                u[8]  = (char) 48+h%75;
                u[9]  = (char) 48+g/75;
                u[10] = (char) 48+g%75;
                memmove(u+11, score, 9);
                printf("%s\n", u);        
                f = fopen("d", "wt");
                fprintf(f, "%s\n", u);
                fclose(f);
         break;
      case 2:
        do {
          printf("Year: "); scanf("%d", &y);
        } while (y<1900 || y>2100);
        break;
      case 3:
        do {
          printf("New date: "); scanf("%d", &z);
        } while (z<0 || z>1300);
        break;
      case 4:
        do {
          printf("New round: "); scanf("%d", &r);
        } while (r<0 || r>8);
        break;
    }
  }

  return 0;
}
