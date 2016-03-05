#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int Extract(char *filename, char *dir, char *pattern) {
  char fullname[128];
  sprintf(fullname, "/Users/radu/rsssf/sat/web/%s/%s", dir, filename);
  FILE *f = fopen(fullname, "rt");
  if (!f) {
    printf("ERROR: file %s not found.\n", fullname);
    return 1;
  }
  char s[1024], *tk[10], tm[100];
  while (!feof(f)) {
    fgets(s, 1024, f);
    if (strstr(s, "xt team la")!=NULL) {
      tk[0] = strstr(s, "href");
      if (!tk[0]) continue;
      tk[0] += 6;
      tk[1] = strtok(tk[0], "\"");
      strncpy(tm, tk[0], 100);
      strtok(tm, "/");
      for (int i=2; i<=4; i++) {
        tk[i] = strtok(NULL, "/");
      }
      if (strstr(tk[0], pattern)!=NULL) {
        printf("./httrack \"http://www.soccerway.com%s\" -O \"/Users/radu/rsssf/sat/web/%s/\" --get\n", tk[0], dir);
        printf("cat /Users/radu/rsssf/sat/web/%s/index-2.html >> /Users/radu/rsssf/sat/web/%s/res-%s\n", dir, dir, filename);
      }
    }
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("ERROR: no input file specified.\n");
    printf("Usage: get-sw-team-names a.html sco scotland\n");
    return 1;
  }
  if (argc < 3) {
    printf("ERROR: no output directory specified.\n");
    printf("Usage: get-sw-team-names a.html sco scotland\n");
    return 2;
  }
  if (argc < 4) {
    printf("ERROR: no pattern specified.\n");
    printf("Usage: get-sw-team-names a.html sco scotland\n");
    return 3;
  }
  printf("#!/bin/csh\n\n");
  printf("rm -f /Users/radu/rsssf/sat/web/res-%s\n", argv[1]);
  printf("touch /Users/radu/rsssf/sat/web/res-%s\n", argv[1]);
  printf("cd /Users/radu/Programs/httrack-3.40.3/src/\n\n");
  Extract(argv[1], argv[2], argv[3]);
  printf("\ncd /Users/radu/rsssf/sat/%s\n", argv[2]);
  return 0;
}

