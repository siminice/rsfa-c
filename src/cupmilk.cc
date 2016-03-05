
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


const char *Month[] = {"January", "February", "March", "April", "May", "June",
                 "July", "August", "September", "October", "November", "December"};


char **club;
char **mnem;
int NC, NMA;

char malias1[100][100], malias2[100][100];

void Load(char *filename) {
  FILE *f;
  char s[500], *tok[20], *ystr, *name, *nick;
  f = fopen("teams.dat", "rt");
  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];
  for (int i=0; i<NC; i++) {
    fgets(s, 500, f);
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
  f = fopen("milkalias.dat", "rt");
  if (f==NULL) return;
  int j = 0;
  while (!feof(f)) {
    fgets(s, 100, f);
    if (strlen(s)>10) {
      strcpy(malias1[j], strtok(s, ","));
      strcpy(malias2[j], strtok(NULL, "\n"));
      j++;
    }
    s[0] = 0;
  }
  NMA = j;
  fclose(f);
}



int mon(char *s) {
  for (int i=0; i<12; i++)
    if (strcmp(Month[i], s)==0) return i;
  return 0;
}

int idx(char *s) {
  for (int i=0; i<NC; i++)
   if (strcmp(mnem[i], s)==0) return i;
  // few aliases
  for (int j=0; j<NMA; j++)
    if (strcmp(s, malias1[j])==0) return idx(malias2[j]);
  return -1;

  // few aliases
  if (strcmp(s, "Boston")==0) return idx("Boston Utd");
  if (strcmp(s, "Banbury Spencer")==0) return idx("Banbury Utd");
  if (strcmp(s, "Dover Ath")==0) return idx("Dover");
  if (strcmp(s, "Hinckley Ath")==0) return idx("Hinckley Utd");
  if (strcmp(s, "Grantham Town")==0) return idx("Grantham");
  if (strcmp(s, "Ramsgate Ath")==0) return idx("Ramsgate");
  if (strcmp(s, "Fleetwood")==0) return idx("Fleetwood Town");
  if (strcmp(s, "Vauxhall M Luton")==0) return idx("Vauxhall Motors");
  if (strcmp(s, "Accrington St(1)")==0) return idx("Accrington Stan");
  if (strcmp(s, "New Brighton")==0) return idx("New Brighton Twr");  
  if (strcmp(s, "Aldershot Town")==0) return idx("Aldershot");
  if (strcmp(s, "Bedford Town (1)")==0) return idx("Bedford Town");
  if (strcmp(s, "Dagenham")==0) return idx("Dagenham & Red");
  if (strcmp(s, "Brentford Town")==0) return idx("Brentford");
  if (strcmp(s, "Bury Town")==0) return idx("Bury");
  if (strcmp(s, "Skelmersdale Utd")==0) return idx("Skelmersdale");
  if (strcmp(s, "Newport Co AFC")==0) return idx("Newport Co");
  if (strcmp(s, "South Shields(2)")==0) return idx("South Shields");
  if (strcmp(s, "Waterlooville")==0) return idx("Havant & Waterlv");
  if (strcmp(s, "Walton")==0) return idx("Walton & Hersham");
  if (strcmp(s, "Gateshead Utd")==0) return idx("Gateshead");
  if (strcmp(s, "Folkestone & Shp")==0) return idx("Folkestone");
  if (strcmp(s, "Folkestone Inv")==0) return idx("Folkestone");
  if (strcmp(s, "Leytnstne/Ilford")==0) return idx("Leytonstone");
  if (strcmp(s, "Sutton Cold Town")==0) return idx("Sutton Utd");
  if (strcmp(s, "Sutton Town")==0) return idx("Sutton Utd");
  if (strcmp(s, "Shepshed Dyn")==0) return idx("Shepshed Ch");
  if (strcmp(s, "AFC Sudbury")==0) return idx("Sudbury Town");
  if (strcmp(s, "Harrogate Rail A")==0) return idx("Harrogate Town");
  if (strcmp(s, "Hastings Utd (1)")==0) return idx("Hastings Utd");
  if (strcmp(s, "AP Leamington")==0) return idx("Leamington");
  if (strcmp(s, "Tunbridge WR (1)")==0) return idx("Tunbridge W Utd");
  if (strcmp(s, "Corinthians")==0) return idx("Corinthian Cas");
  if (strcmp(s, "Burton Town")==0) return idx("Burton Utd");
  if (strcmp(s, "Birmingham Co Tr")==0) return idx("Birmingham Co");
  if (strcmp(s, "Barry")==0) return idx("Barry Town");
  if (strcmp(s, "Ashford")==0) return idx("Ashford Town");
  if (strcmp(s, "Merthyr Town")==0) return idx("Merthyr Tydfil");
  if (strcmp(s, "Boston (1)")==0) return idx("Boston Utd");
  if (strcmp(s, "Manchester Centr")==0) return idx("Manchester Cen");
  if (strcmp(s, "Peterboro & FU")==0) return idx("Peterborough Utd");
  if (strcmp(s, "Chelmsford")==0) return idx("Chelmsford City");
  if (strcmp(s, "Wigan Boro")==0) return idx("Wigan Ath");
  if (strcmp(s, "Rhyl Ath")==0) return idx("Rhyl");
  if (strcmp(s, "Caernarvon Ath")==0) return idx("Caernarfon Town");
  if (strcmp(s, "Dagenham Town")==0) return idx("Dagenham & Red");
  if (strcmp(s, "Taunton Town (1)")==0) return idx("Taunton Town");
  if (strcmp(s, "Poole")==0) return idx("Poole Town");
  if (strcmp(s, "Bedlington Utd")==0) return idx("Bedlington Terr");
  if (strcmp(s, "Chilton Colliery")==0) return idx("Chilton Collier");
  if (strcmp(s, "South Bank")==0) return idx("South Bank St P");
  if (strcmp(s, "Burton Alb")==0) return idx("Burton Utd");
  if (strcmp(s, "Horden Ath")==0) return idx("Horden CW");
  if (strcmp(s, "")==0) return idx("");
  return -1;
}

int main(int argc, char **argv) {

  Load("teams.dat");
 
  if (argc<4) {
    printf("Usage: milk <eng> <cup> <season>\n");    
    return -1;
  }

  char s[200], t[200], u[200], dirname[100], filename[100];
  char *tok[10], home[32], guest[32];
  int z, m, y, r, a, b, h, g;
  int br[10];

    sprintf(filename, "/Users/radu/rsssf/statto/%s-%s/%s/%s.txt", argv[2], argv[1], argv[3], argv[3]);
    FILE *f = fopen(filename, "rt");
    if (f==NULL) {
      printf("Cannot open %s...\n", filename);
      return -2;
    }
    for (int i=0; i<10; i++) br[i] = 0;
    do { 
      fgets(s, 200, f);
      if (strlen(s)>4) {
        if (s[0]==' ') {
          if (s[1]==' ') {
            if (strstr(s, "bye")!=NULL) {
              tok[0] = strtok(s+2, " \t\n");
//              printf("%s bye\n", tok[0]);
            }
            else if (strstr(s, "w/o")!=NULL || strstr(s, " v ")!=NULL) {
//              printf("%s", s);
            }
            else {
              int k = 2;
              while (k<strlen(s)-2 && !(s[k+1]=='-' && s[k]>='0' && s[k]<='9' && s[k+2]>='0' && s[k+2]<='9')) k++;
              int j = k-1; while (j>=0 && s[j]!=' ') j--;
              int l = k+3; while (l<strlen(s) && s[l]!=' ') l++;
              strncpy(home, s+2, j-2); home[j-2] = 0;
              strncpy(guest, s+l+1, strlen(s)-l-2); guest[strlen(s)-l-2] = 0;
              a = atoi(s+j+1);
              b = atoi(s+k+2);
              //printf("%2d.%02d.%4d R%d : %s - %s %d-%d [%d]\n", z, m+1, y, r, home, guest, a, b, NC);
              h = idx(home);
              g = idx(guest);
              if (h<0) printf("Cannot identify %s\n", home);
              if (g<0) printf("Cannot identify %s\n", guest);

              if (h>=0 && g>=0) {
 	        u[0]  = 48 + (y - 1870)/75;
 	        u[1]  = 48 + (y - 1870)%75;
	        u[2]  = (char) 49+m;
	        u[3]  = (char) 48+z;
	        u[4]  = (argv[2][0]=='l'?'L':'K');
	        u[5]  = ' ';
	        u[6]  = (char) 49+r;
	        u[7]  = (char) 48+h/75;
	        u[8]  = (char) 48+h%75;
	        u[9]  = (char) 48+g/75;
	        u[10] = (char) 48+g%75;
	        u[11] = (char) 48+a;
	        u[12] = (char) 48+b;
	        u[13] = 0;
                printf("%s\n", u);
              }
            }
          }
          else {
            tok[0] = strtok(s+1, "\t\n ");
            tok[1] = strtok(NULL, " \t\n");
            tok[2] = strtok(NULL, " \t\n");
            tok[3] = strtok(NULL, " \t\n");
            if (tok[1]!=NULL && tok[2]!=NULL && tok[3]!=NULL) {
              y = atoi(tok[3]);
              m = mon(tok[2]); 
                if (m<7) {m+=12;} else y++;
              z = atoi(tok[1]);
            }
          }
        }
        else if (strstr(s, "Intermediate")==s) r = 8;
        else if (strstr(s, "First")==s) r = 7;
        else if (strstr(s, "Second")==s) r = 6;
        else if (strstr(s, "Third")==s) r = 5;
        else if (strstr(s, "Fourth")==s) r = 4;
        else if (strstr(s, "Fifth")==s) r = 3;
        else if (strstr(s, "Sixth")==s) r = 2;
        else if (strstr(s, "Quarter")==s) r = 2;
        else if (strstr(s, "Semi-Final")==s) r = 1;
        else if (strstr(s, "Final")==s) r = 0;
      }
      s[0] = 0;
    } while (!feof(f));
    fclose(f);
  return 1;
}
