#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mls.h"

int main() {
    m_init();
    const char *s = "Column1\tColumn2\tColumn3";
    int columns = s_split(0, s, '\t', 0);
    printf("Split columns into %d\n", m_len(columns));
    int p;
    char **cell_text;
    m_foreach(columns, p, cell_text) {
        printf("Col %d: '%s'\n", p, *cell_text);
    }
    m_free_strings(columns, 0);
    m_destruct();
    return 0;
}
