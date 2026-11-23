/* Print Gridbox ABI-relevant sizes/offsets.
 *
 * Build once against plainc_widgets/GridboxP.h and once against the generated
 * header; diff the output.  GridboxClassRec is intentionally not printed: the
 * unused gridbox_class extension slot differs between the hand-written and
 * generated class parts.
 */
#include <stdio.h>
#include <stddef.h>
#include <X11/IntrinsicP.h>

#ifdef USE_PORT
#include <xtcw/GridboxP.h>
#else
#include "GridboxP.h"
#endif

#ifdef USE_PORT
typedef GridboxConstraintRec GBCR;
#else
typedef GridboxConstraintsRec GBCR;
#endif

#define P(label, expr) printf("%-28s %zu\n", label, (size_t)(expr))

int main(void)
{
    P("sizeof(GridboxClassRec)", sizeof(GridboxClassRec));
    P("sizeof(GridboxPart)", sizeof(GridboxPart));
    P("sizeof(GridboxRec)", sizeof(GridboxRec));
    P("sizeof(constraint rec)", sizeof(GBCR));

    P("off.defaultDistance", offsetof(GridboxRec, gridbox.defaultDistance));
    P("off.nx", offsetof(GridboxRec, gridbox.nx));
    P("off.ny", offsetof(GridboxRec, gridbox.ny));
    P("off.maxgw", offsetof(GridboxRec, gridbox.maxgw));
    P("off.maxgh", offsetof(GridboxRec, gridbox.maxgh));
    P("off.max_wids", offsetof(GridboxRec, gridbox.max_wids));
    P("off.wids", offsetof(GridboxRec, gridbox.wids));
    P("off.total_wid", offsetof(GridboxRec, gridbox.total_wid));
    P("off.total_weightx", offsetof(GridboxRec, gridbox.total_weightx));
    P("off.needs_layout", offsetof(GridboxRec, gridbox.needs_layout));

    P("off.gridx", offsetof(GBCR, gridbox.gridx));
    P("off.gridy", offsetof(GBCR, gridbox.gridy));
    P("off.gridWidth", offsetof(GBCR, gridbox.gridWidth));
    P("off.gridHeight", offsetof(GBCR, gridbox.gridHeight));
    P("off.fill", offsetof(GBCR, gridbox.fill));
    P("off.gravity", offsetof(GBCR, gridbox.gravity));
    P("off.weightx", offsetof(GBCR, gridbox.weightx));
    P("off.weighty", offsetof(GBCR, gridbox.weighty));
    P("off.margin", offsetof(GBCR, gridbox.margin));
    P("off.prefWidth", offsetof(GBCR, gridbox.prefWidth));
    P("off.prefHeight", offsetof(GBCR, gridbox.prefHeight));
    P("off.queried", offsetof(GBCR, gridbox.queried));

    return 0;
}
