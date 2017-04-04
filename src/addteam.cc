#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alias.hh"
#include "fed.hh"

int main(int argc, char **argv) {
  char *tfilename = strdup("teams.dat");
  char *sfilename = strdup("riku.dat");
  char *afilename = strdup("alias.dat");
  int y = 1800;

  Fed *F = new Fed();
  if (!F->Load(tfilename, FIXED)) return -1;
  F->List();

  Fed *S = new Fed();
  if (!S->Load(sfilename, VAR)) return -2;
  S->List();

  AliasCollection *A = new AliasCollection();
  if (!A->Load(F->NC, afilename)) return -3;
  A->List();

  char mnem[255], name[255], nick[255], swnick[255], swname[255];

  printf("Mnemonic: ");
  do { fgets(mnem, 16, stdin); } while (!strlen(mnem));
  mnem[strlen(mnem)-1] = 0;
  int i = F->Find(mnem);
  if (i!=UNKNOWN) {
    printf("Already exists.\n");
    printf("%4d: %s, %s\n", i, F->mnem[i], F->club[i]);
  } else {
    printf("Fullname: ");
    do { fgets(name, 60 , stdin); } while (!strlen(name));
    name[strlen(name)-1] = 0;
    printf("Nickname: ");
    do { fgets(nick, 16, stdin); } while (!strlen(nick));
    nick[strlen(nick)-1] = 0;
    printf("SW handle: ");
    do { fgets(swnick, 40, stdin); } while (!strlen(swnick));
    swnick[strlen(swnick)-1] = 0;
    sprintf(swname, "%s", name);
    printf("Adding:\n");
    printf("%-15s: %s|%s\n", tfilename, mnem, name);
    printf("%-15s: %s,%s\n", sfilename, swnick, swname);
    printf("%-15s: *%d %s~%s\n", afilename, y, name, nick);
    char *opt = new char[128];
    printf("Confirm [y/n] "); fgets(opt, 8, stdin);
    if (opt[0]=='y') {
      F->Add(mnem, name);
      S->Add(swnick, swname);
      A->Add(new alias(y, name, nick));
      F->Save(tfilename, FIXED);
      S->Save(sfilename, VAR);
      A->Save(afilename);
    }
  }
  return 0;
}
