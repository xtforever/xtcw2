#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mls.h"
#include "retex.h"
#include "node.h"
#include "linebreak.h"
#include "glue.h"

int main() {
    m_init();
    extern int trace_level;
    trace_level = 2;

    int p_list = m_create(10, sizeof(Node));
    
    // Word 1 (50pt)
    node_create_char(p_list, '1', FROM_INT(50), FROM_INT(10), 0, 10.0, 0, 0, 0);
    // Glue (10pt)
    Glue g = glue_create(FROM_INT(10), 0, 0, 0, 0);
    node_create_glue(p_list, g, 0);
    // Word 2 (50pt)
    node_create_char(p_list, '2', FROM_INT(50), FROM_INT(10), 0, 10.0, 0, 0, 0);
    node_create_glue(p_list, g, 0);
    // Word 3 (50pt)
    node_create_char(p_list, '3', FROM_INT(50), FROM_INT(10), 0, 10.0, 0, 0, 0);

    // Target 80pt.
    // Line 1: Word 1 (50pt) + Glue(10pt) = 60pt. Word 2 (50pt) does NOT fit.
    // Line 2: Word 2 (50pt) + Glue(10pt) = 60pt. Word 3 (50pt) does NOT fit.
    // Line 3: Word 3 (50pt)
    
    Glue left = glue_create(0, 0, 0, 0, 0);
    Glue right = glue_create(0, 0, 0, 0, 0);
    
    int lines = line_break(p_list, FROM_INT(80), left, right, 0, 0);
    printf("Line count: %d\n", m_len(lines));
    node_list_free(lines);

    /* Regression: the final line of a justified paragraph must be allowed to
     * be short (TeX parfillskip).  Without it the DP prefers a tiny first
     * line plus a full last line, which stretches the first line to pieces.
     *
     * 11 words x 10pt + 10 glues x 5pt = 160pt at a 100pt measure:
     * correct split is 7 words (100pt) + 4 words (55pt).
     * The broken split was 4 words (55pt) + 7 words (100pt). */
    {
        int para = m_create(32, sizeof(Node));
        Glue word_gap = glue_create(FROM_INT(5), FROM_INT(2),
                                    ORDER_NORMAL, FROM_INT(1), ORDER_NORMAL);
        int w;
        for (w = 0; w < 11; w++) {
            if (w)
                node_create_glue(para, word_gap, 0);
            node_create_char(para, 'a' + w, FROM_INT(10), FROM_INT(10),
                             0, 10.0, 0, 0, 0);
        }
        LineBreakParams lbp;
        memset(&lbp, 0, sizeof(lbp));
        lbp.width = FROM_INT(100);
        lbp.left_skip = glue_create(0, 0, 0, 0, 0);
        lbp.right_skip = glue_create(0, 0, 0, 0, 0);
        lbp.tolerance = 200;
        lbp.emergencystretch = FROM_INT(5);
        lbp.hyphen_penalty = 50;

        int kp_lines = line_break_kp(para, &lbp);
        assert(m_len(kp_lines) == 2);
        Node *first = (Node *)mls(kp_lines, 0);
        double first_w = TO_DOUBLE(first->width);
        printf("parfill first line width: %.2f\n", first_w);
        assert(first_w >= 90.0);
        node_list_free(kp_lines);
        node_list_free(para);
    }

    m_destruct();
    return 0;
}
