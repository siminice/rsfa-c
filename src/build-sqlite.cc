// completely messed up program to perform queries on 
// an artisanal database

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>

#define NC         350
#define MAX_NAMES 1000
#define NAME_LEN    60
#define FULL_LEN   120
#define MAX_CITY   300
#define MAX_SCORE   35

const char *Unofficial[] = 
{"African Games",
 "Amilcar Cabral Cup",
 "Arab Cup",
 "Asian Games",
 "Balkan Cup",
 "Baltic Cup",
 "CECAFA Cup",
 "CEDEAO Tournament",
 "UDEAC/CEMAC Cup",
 "Central African Games",
 "Central American Games",
 "COSAFA Cup",
 "Dr.Gerö Cup",
 "East Asian FF Tournament",
 "Gulf Cup",
 "Melanesian Cup",
 "Merdeka Tournament",
 "Nordic Championship",
 "South Asian FF Cup",
 "South Asian Federation Games",
 "South East Asian Games/Tiger Cup",
 "South Pacific Games",
 "West Asian FF Championship",
 "West Asian Games",
 "British Home Championship",
 "Indian Ocean Games",
 "AFC Challenge Cup",
 "Coupe de l'Outre Mer",
 "Nile Basin Tournament"
};

void ToLower(char *s) {
  if (!s) return;
  int l = strlen(s);
  if (l<=3) return;
  if (strcmp(s, "USSR")==0) return;
  for (int i=1; i<l; i++)
    if (s[i-1]!=' ' && s[i-1]!='.' && s[i-1]!='-' && s[i]>='A' && s[i]<='Z') s[i]=s[i]+32;
  if (s[1]=='r' && s[2]==' ') s[1]='R';
}

void RmBlanks(char *s) {
  if (!s) return;
  int l = strlen(s);
  for (int i=1; i<l; i++) 
    if (s[i]==' ') s[i]='_';
}

class Entry {
 public:
  int  year;
  int  month;
  int  day;
  int  prelim;
  int  tourn;
  int  edition;
  int  round;
  int  number;
  int  host;
  int  city;
  int  home;
  int  guest;
  int  score[8];
  int  marc[MAX_SCORE];
  Entry();
  void Copy(Entry*);
  void Decode(char*);
  int Check(int, int);
};

struct alias {
  char *name;
  int  year;
  int  zone;
  alias *next;
  alias(char *s, int y, int z, alias *a = NULL);
  ~alias();
};

// global variables

const char* Month[] ={"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char* Round[] ={"group", "round", "play-off", "1/8 finals", 
                "quarterfinals", "semifinals", "3rd place", "Final", ""};
int    NT;
char   Country[NC][NAME_LEN];
char   Mnem[NC][4];
int    NP[NC];
char   Player[NC][MAX_NAMES][NAME_LEN];
char   FullName[NC][MAX_NAMES][FULL_LEN];
char   City[NC][MAX_CITY][NAME_LEN];
int    ncity[NC];
char   Zone[NC];
const char   *Cont[] = {"EUROPE","CONMEBOL","CONCACAF","AFRICA","ASIA","OCEANIA"};
alias  *Alias[NC];
Entry  E, X, Y;
FILE*  outf;
char   **data;
char   **friendly;
int    M, FM;

int FindC(char*);
char* CountryName(int,int);
int ZoneOf(int,int);
void Load();

//-------------------------------
// class Entry
//-------------------------------

Entry::Entry() {
 // nothing to do !
};

void Entry::Copy(Entry *e) {
  year    = e->year;
  month   = e->month;
  day     = e->day;
  prelim  = e->prelim;
  tourn   = e->tourn;
  edition = e->edition;
  round   = e->round;
  number  = e->number;
  host    = e->host;
  city    = e->city;
  home    = e->home;
  guest   = e->guest;
  for (int i=0; i<8; i++)
    score[i] = e->score[i];
  for (int j=0; j<score[0]+score[1]; j++) 
    marc[j] = e->marc[j];
}

void Entry::Decode(char* s) {
 int i;
  char t[20];
  strncpy(t, s, 4); t[4] = 0;
  year    = atoi(t);
  month   = s[4] - 49;
  day     = s[5] - 48;
  prelim  = s[6];
  tourn   = s[7];
  strncpy(t, s+8, 4);
  edition = atoi(t);
  round   = s[12];
  number  = s[13] - 48;
  host    = FindC(s+14);
  city    = ((unsigned char) s[17]) - 65;
  home    = FindC(s+18);
  guest   = FindC(s+21);
  for (i=0; i<8; i++)
    score[i] = (i+24 < strlen((char *)s) ? s[i+24] - 48 : -1);
  for (i = 0; i < score[0] + score[1]; i++)
    if (32 + 2*i  < strlen((char*)s)) {
      if (s[2*i+32] < 77)
        marc[i] = 90*(s[32+2*i]-32) + s[32+2*i+1] - 32;
      else
        marc[i] = - ( 90*(s[32+2*i]-77) + s[32+2*i+1] - 32);
    }
    else marc[i] = 0;
};



alias::alias(char *s, int y, int z, alias *a) {
  name = new char[strlen(s)+1];
  strcpy(name, s);
  year = y;
  zone = z;
  next = a;
}

alias::~alias() {
  if (next) delete next;
  delete name;
}

//-----------------------------------
//  Global procedures
//-----------------------------------

void Load() {
 FILE *f;
 char s[200];
 int t = 0;
 int i;
 char *tok[20];

 f = fopen("country.dat", "rt");
 fgets(s, 200, f);
 NT = atoi(s);
 for (t=0; t<NT; t++) {
   fgets(s, 200, f);
   strncpy(Mnem[t], s, 3);
   Zone[t] = s[5] - 48;
   Alias[t] = NULL;
   tok[0] = strtok(s, ",")+6;
   tok[1] = strtok(NULL, ".");

   strcpy(Country[t], tok[0]);
   for (int j=1; j<5; j++) {
     tok[2*j] = tok[2*j+1] = NULL;
     tok[2*j] = strtok(NULL, ",");
     tok[2*j+1] = strtok(NULL, ".");
   }
   if (tok[2]!=NULL) {
     int k=0;
     while (tok[k]!=NULL) k++;
     k-=2;
     while (k>=0) {
       Alias[t] = new alias(tok[k], atoi(tok[k+1]), Zone[t], Alias[t]);
       k-=2;
     }
   }
 }
 fclose(f);

 int maxnp = 0;
 t = -1;
 f = fopen("players.dat", "rt");
 while (!feof(f)) {
   fgets(s, 120, f);
   if (strlen(s)) s[strlen(s) - 1] = 0;
   if (s[0] == '#') {
     if (t>=0) NP[t] = i;
     if (NP[t] > maxnp) maxnp = NP[t];
     t++; 
     i=0;
   }
   else {
     char *shrtn = strtok(s, "~\n");
     if (shrtn==NULL || strlen(shrtn)==0) continue;
     char *first = strtok(NULL, "\n");
     strncpy(Player[t][i++], s, NAME_LEN); 
     if (first==NULL || strlen(first)==0) 
      strncpy(FullName[t][i-1], s, FULL_LEN);
     else {
       char *last = strrchr(shrtn, '.');
       if (last==NULL) last = shrtn;
       else last = last + 1;
       sprintf(FullName[t][i-1], "%s %s", first, last);
     }
   }
 }
 fclose(f);
 NT = t;
 t = -1;

 f = fopen("city.dat", "rt");
 while (!feof(f)) {
   fgets(s, 200, f);
   s[strlen(s) - 1] = 0;
   if (s[0] == '#') {
     t++; i=0;
   }
   else 
     strcpy(City[t][i++], s);
 }
 fclose(f);

 f = fopen("friendly.dat", "rt");
 FM = 0;
 while(!feof(f)) {
   fgets(s, 200, f);
   if (strlen(s) > 2) FM++;
   s[0] = 0;
 }
 fclose(f);

 f = fopen("internat.dat", "rt");
 M = 0;
 while(!feof(f)) {
   fgets(s, 200, f);
   if (strlen(s) > 2) M++;
   s[0] = 0;
 }
 fclose(f);
// friendly = new char*[FM];

 data = new char*[M+FM];
 f = fopen("friendly.dat", "rt");
 for (int j=0; j<FM; j++) {
   fgets(s, 200, f);
   int len = strlen(s);
   s[len-1] = 0;
   data[j] = new char[len];
   strcpy(data[j], s);
 }
 fclose(f);

 f = fopen("internat.dat", "rt");
 for (int j=0; j<M; j++) {
   fgets(s, 200, f);
   int len = strlen(s);
   s[len-1] = 0;
   data[FM+j] = new char[len];
   strcpy(data[FM+j], s);
 }
 fclose(f);
 M = M+FM;

};

int FindC(char* s) {
  for (int i=0; i<NT; i++)
    if (Mnem[i][0]==s[0] && Mnem[i][1]==s[1] && Mnem[i][2]==s[2]) return i;
  return 0;
}

//--------------------------------------------

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}


  char *cmd = new char[512];
  sqlite3 *db;
  char *zErrMsg = 0;

void sqlex() {
  int rc = sqlite3_exec(db, cmd, callback, 0, &zErrMsg);
  if (rc!=SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }  
}

int main(int argc, char** argv) {

  FILE *f;
  int t, i, rc;
  char *s      = new char[1024];
  char *first  = new char[64];
  char *last   = new char[64];
  char *shortn = new char[64];

  Load(); 

  rc = sqlite3_open("../dbs/fifa.db", &db);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }

  sprintf(cmd, "delete from tcountry;");
  sqlex();

  for (t=0; t<NT; t++) {
    sprintf(cmd, "insert into tcountry values('%s', %d, '%s');", Mnem[t], Zone[t]+1, Country[t]);
    sqlex();
  }

  sprintf(cmd, "delete from tstadium;");
  sqlex();

  int sid=0;
  t = -1;
 
  f = fopen("city.dat", "rt");
  while (!feof(f)) {
    fgets(s, 200, f);
    if (feof(f)) continue;
    s[strlen(s) - 1] = 0;
    if (s[0] == '#') {
      t++;
    }
    else if (Mnem[t][2]!='-') {
      sid++;
      sprintf(cmd, "insert into tstadium values(%d, \"%s\", \"%s\", \"%s\");", sid, s, s, Mnem[t]);
      sqlex();
    }
  }
  fclose(f);

  sprintf(cmd, "delete from tplayer;");
  sqlex();

 t = -1;
 int pid = 0;
 f = fopen("players.dat", "rt");
 while (!feof(f)) {
   fgets(s, 120, f);
   if (feof(f)) continue;
   if (strlen(s)) s[strlen(s) - 1] = 0;
   if (s[0] == '#') {
     t++;
   }
   else { 
     char *sn = strtok(s, "~\n");
     if (sn==NULL || strlen(sn)==0) continue;
     char *fn = strtok(NULL, "\n");
     if (fn==NULL || strlen(fn)==0) {
       strcpy(first, "");
     }
     else strcpy(first, fn);

       char *ln = strrchr(sn, '.');
       if (ln==NULL) ln = sn;
       else ln = ln + 1;
       pid++;
     if (pid%100==0) printf("Adding item #%d...\n", pid);
     sprintf(cmd, "insert into tplayer values(%d, \"%s\", \"%s\", \"%s\", '1800-01-01', \"%s\");", 
         pid, first, ln, sn, Mnem[t]);
     sqlex();
   }
 }
 fclose(f);

  sprintf(cmd, "delete from tresults;");
  sqlex();

  for (int i=0; i<M+FM; i++) {
    E.Decode(data[i]);
    if (i%100 == 0) printf("Adding result #%d...\n", i);
    sprintf(cmd, "insert into tresults values(%d, '%d-%02d-%02d', '%c', '%c', %d, '%c', %d, \"%s\", %d, \"%s\", \"%s\", %d, %d, %d, %d, %d, %d, %d, %d);",
            i, E.year, E.month, E.day, char(E.prelim), char(E.tourn), E.edition, char(E.round), E.number, 
            Mnem[E.host], E.city, Mnem[E.home], Mnem[E.guest], 
            E.score[0], E.score[1], E.score[2], E.score[3], E.score[4], E.score[5], E.score[6], E.score[7]);
            
    sqlex();
  }

  sqlite3_close(db);

  return 0;
}
