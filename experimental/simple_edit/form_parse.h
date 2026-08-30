#ifndef FORM_PARSE_H
#define FORM_PARSE_H



/* rule: X & Label Above */
struct form_entry {
  int label;
  int x,y,len;
  int typ;
};

int form_scan(int scr);
void form_dump(int f);
void form_free(int f);
int form_from_file(char *name);

#endif
