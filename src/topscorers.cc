#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SCORERS 20

struct Node {
  char *name;
  int  g;
  Node *next;
  Node (char*, int, Node*);
};

Node::Node(char *s, int y, Node *n=NULL) {
  name = new char [strlen(s)+1];
  strcpy(name, s);
  g = y;
  next = n;
}

class List {
 public: 
  Node *head;
  char *name;
  List(char *);
  void Add(char *, int);
  void Print();
  void Sort();
};

List::List(char *s) {
  name = new char [strlen(s)+1];
  strcpy(name, s);
  head == NULL;
}


void List::Add(char *s, int x) {
  if (head==NULL)
    head = new Node(s, x, NULL);
  else {
    Node *p = head;
    while (p!=NULL && strcmp(p->name, s)!=0) p=p->next;
    if (p==NULL) head = new Node(s, x, head);
    else p->g += x;
  }
}

void List::Print() {
  printf("%s:\n", name);
  for (Node *p=head; p!=NULL; p=p->next)
    printf(" .%s (%d)\n", p->name, p->g);
}

void List::Sort() {
  int sorted, last, aux;
  char *xn;
  do {
    sorted = 1;
    Node *p = head;
    while (p!=NULL && p->next!=NULL) {
      if (p->g < p->next->g) {
        sorted = 0;
        xn = p->name;
        p->name = p->next->name; 
        p->next->name = xn;
        aux = p->g;
        p->g = p->next->g;
        p->next->g = aux;
      }
      p = p->next;
    }
  } while (!sorted);
}


List **L;
int NL;

int Find(char *team) {
  if (strlen(team)<3) return -1;
  int i;
  for (i=0; i<NL; i++)
    if (strcmp(L[i]->name, team)==0) return i;
  if (i>=NL)
    L[NL++] = new List(team);
  return i;
}


int main(int argc, char**argv) {
  if (argc < 2) {
    printf("ERROR: No input file specified.\n");
    return 2;
  }
  FILE *f = fopen(argv[1], "rt");
  if (f==NULL) {
    printf("ERROR: file %s not found.\n", argv[1]);
    return 1;
  }
  char s[200], *tk[100], home[100], guest[100], scorer[100];
  L = new List*[200];
  int j, x;
  NL = 0;
  int h0;
  while (!feof(f)) {
    fgets(s, 200, f);
    if (strlen(s) < 10) continue;
    if (s[0] == '<') continue;
    int k=1;
    while (k<strlen(s)-1 && !(
           s[k-1]>='0' && s[k-1]<='9' && s[k]=='-' &&
           s[k+1]>='0' && s[k+1]<='9')) k++;
    if (k<strlen(s)-1) {
      tk[0] = strtok(s, " \t\n");
      for (j=1; j<10; j++) tk[j] = strtok(NULL, " \t\n");
      strcpy(home, tk[0]);
      for (j=1; j<10 && (tk[j][0]<'0' || tk[j][0]>'9'); j++) {
       strcat(home, " ");
       strcat(home, tk[j]);
      }
      h0 = (tk[j][0]=='0');
      j++;
      strcpy(guest, tk[j]);
      for (j++; tk[j]!=NULL && j<10 && (tk[j][0]<'0' || tk[j][0]>'9'); j++) {
       strcat(guest, " ");
       strcat(guest, tk[j]);
      }
//      printf("%s vs %s\n", home, guest); 
    } 
    if (strlen(s) > 5 && s[2] == '[') {
      if (h0) x = Find(guest); else x = Find(home);
      tk[0] = strtok(s+3, " \t\n");
      for (j=1; j<30; j++) tk[j] = strtok(NULL, " \t\n");
      int h = 0;
      int gsc = 0;
      while (tk[h]!=NULL) {
        strcpy(scorer, tk[h]); h++;
        while (tk[h]!=NULL && (tk[h][0]<'0' || tk[h][0]>'9')) {
          strcat(scorer, " ");
          strcat(scorer, tk[h]);
          h++;
        }
        do {
          if (strstr(tk[h], "og")!=NULL) {
            strcat(scorer, " (");
            if (strcmp(L[x]->name, home)==0) strcat(scorer, guest);
            else strcat(scorer, home);
            strcat(scorer, ")-og");
          }
          if (x>=0) L[x]->Add(scorer, 1);
          if (tk[h][strlen(tk[h])-1]==';') x = Find(guest);
          h++;
        } while (tk[h]!=NULL && tk[h][0]>='1' && tk[h][0]<='9');
      }
    }
    s[0] = 0;
  }
  fclose(f);


  for (int i=0; i<NL; i++) {
    printf("%d. %s\n", i+1, L[i]->name);
  }
  for (int i=0; i<NL; i++) {
    printf("%d: ", i+1);
    L[i]->Sort();
    L[i]->Print();
  }

  printf("\n\nTop scorers:\n");
  printf("===============\n");

  Node **w = new Node*[NL];
  int *numg = new int[NL];
  int *rnk  = new int[NL];
  int sorted;

  for (int i=0; i<NL; i++) {
    w[i] = L[i]->head;
    rnk[i] = i;
  }
  for (int t=0; t<MAX_SCORERS; t++) {
    for (int i=0; i<NL; i++) numg[i] = (w[i]==NULL?0:w[i]->g);
    do {
      sorted = 1;
      for (int j=0; j<NL-1; j++) {
        if (numg[rnk[j]] < numg[rnk[j+1]]) {
          sorted = 0;
          int aux = rnk[j];
          rnk[j] = rnk[j+1];
          rnk[j+1] = aux;
        }
      }
    } while (!sorted);
    printf("%2d. %-15s %-15s %2d\n", t+1, 
      w[rnk[0]]->name, L[rnk[0]]->name, w[rnk[0]]->g);
    w[rnk[0]] = w[rnk[0]]->next;
  }

  return 0;
}
