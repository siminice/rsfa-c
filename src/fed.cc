#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fed.hh"

//*****************************************************************
//*****                   Fed class                           *****
//*****************************************************************

int Fed::Load(const char *filename, int style) {
  FILE *f;
  char s[2000], *tok[20], *ystr, *name, *nick;
  f = fopen(filename, "rt");
  if (f==NULL) return 0;

  fscanf(f, "%d\n", &NC);
  club = new char*[NC];
  mnem = new char*[NC];

  for (int i=0; i<NC; i++) {
    fgets(s, 256, f);
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
  return 1;
}

int Fed::Add(char *amnem, char *aclub) {
  char **oldmnem = mnem;
  char **oldclub = club;
  mnem = new char*[NC+1];
  club = new char*[NC+1];
  for (int i=0; i<NC; i++) {
    mnem[i] = oldmnem[i];
    club[i] = oldclub[i];
  }
  if (amnem!=NULL) {
    mnem[NC] = new char[strlen(amnem)+1];
    strcpy(mnem[NC], amnem);
  } else {
    mnem[NC] = strdup("");
  }
  if (aclub!=NULL) {
    club[NC] = new char[strlen(aclub)+1];
    strcpy(club[NC], aclub);
  } else {
    club[NC] = strdup("");
  }
  NC++;
  delete[] oldmnem;
  delete[] oldclub;
  return 1;
}

//---------------------------------------------
int Fed::Find(char *s) {
  if (s==NULL) return UNKNOWN;
  int found = 0;
  int multi = 0;
  int j;
  int l = strlen(s);
  char *t = new char[l+1];
  strcpy(t, s);

  if (t[0] > 96) t[0] -= 32; // start with capital letter;
  for (int i=0; i<l-1; i++)
    if ((t[i]==32 || t[i]=='.') && t[i+1]>96) t[i+1] -= 32;

  // Exact match
  int i = 0;
  while (i < NC) {
    if (strcmp(mnem[i], t)==0) return i;
    i++;
  }

  // Substring
  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], t)) found = 1;
    else i++;
  }
  if (found) return i;

  // try uppercase all
  for (int j=0; j<l; j++)
    if (t[j]>='a' && t[j]<='z') t[j] -= 32;
  i = 0;
  while (i < NC && !found) {
    if (NULL != strstr(mnem[i], t)) found = 1;
    else i++;
  }
  if (found) return i;
  return UNKNOWN;
}

int Fed::GetUnique(const char *prompt) {
  char name[30];
  int res;
  do {
   printf("%s", prompt);
   do { gets(name); } while (!strlen(name));
   res = Find(name);
  } while (res < 0);
  return res;
}

int Fed::Save(const char *filename, int style) {
  char bfilename[64];
  sprintf(bfilename, "%s.old", filename);
  rename(filename, bfilename);
  FILE *f;
  f = fopen(filename, "wt");
  if (f==NULL) return 0;
  fprintf(f, "%d\n", NC);
  for (int i=0; i<NC; i++) {
    if (style==FIXED) {
      fprintf(f, "%-15s%s\n", mnem[i], club[i]);
    } else {
      fprintf(f, "%s,%s\n", mnem[i], club[i]);
   }
  }
  fclose(f);
  return 1;
}

void Fed::List() {
  printf("%d\n", NC);
  for (int i=0; i<NC; i++) {
    printf("%4d: %s, %s\n", i, mnem[i], club[i]);
  }
}
