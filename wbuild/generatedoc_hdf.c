/*   wbuild
     Structured HDF documentation backend.

     Emits one <Widget>.hdf plus an index.hdf.  HDF is the general
     "Human Data Forms" language (nested lists, seq arrays, raw strings,
     rem comments), not the flat theme format used by themes.hdf.
*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <wbuild.h>
#include <wsym.h>

static void hdf_indent(FILE *f, int n)
{
    while (n-- > 0)
        fputs("  ", f);
}

/* Emit a raw string using the shortest delimiter that does not occur in s. */
static void hdf_raw(FILE *f, const char *s)
{
    int eq = 0, i;

    if (!s) {
        fputc('%', f);
        return;
    }
    /* A content ending in ']' would make "]]" ambiguous with the closing
     * delimiter, so use at least one '=' in that case. */
    if (*s && s[strlen(s) - 1] == ']')
        eq = 1;
    for (;;) {
        char close[80];
        int n = 0;
        close[n++] = ']';
        for (i = 0; i < eq && n < (int)sizeof(close) - 2; i++)
            close[n++] = '=';
        close[n++] = ']';
        close[n] = 0;
        if (!strstr(s, close))
            break;
        eq++;
    }
    fputc('[', f);
    for (i = 0; i < eq; i++)
        fputc('=', f);
    fputc('[', f);
    fputs(s, f);
    fputc(']', f);
    for (i = 0; i < eq; i++)
        fputc('=', f);
    fputc(']', f);
}

static int hdf_keyword_ok(const char *s)
{
    const char *p;
    if (!s || !*s)
        return 0;
    if (!(isalpha((unsigned char)*s) || *s == '_'))
        return 0;
    for (p = s + 1; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (!(isalnum(c) || c == '_' || c == '.' || c == '/' ||
              c == ':' || c == '-'))
            return 0;
    }
    if (!strcmp(s, "true") || !strcmp(s, "false") || !strcmp(s, "null") ||
        !strcmp(s, "seq") || !strcmp(s, "rem") || !strcmp(s, "def") ||
        !strcmp(s, "for") || !strcmp(s, "lit") || !strcmp(s, "fold"))
        return 0;
    return 1;
}

static void hdf_atom(FILE *f, const char *s)
{
    if (hdf_keyword_ok(s))
        fputs(s, f);
    else
        hdf_raw(f, s);
}

static void hdf_bool(FILE *f, int v)
{
    fputs(v ? "true" : "false", f);
}

static int is_number(const char *s)
{
    char *end = NULL;
    if (!s || !*s)
        return 0;
    (void)strtod(s, &end);
    return end && *end == 0;
}

static char *trim_copy(const char *s)
{
    const char *a, *b;
    if (!s)
        return NULL;
    a = s;
    while (*a && isspace((unsigned char)*a))
        a++;
    b = s + strlen(s);
    while (b > a && isspace((unsigned char)b[-1]))
        b--;
    return strndup(a, (size_t)(b - a));
}

static void hdf_default(FILE *f, const char *s)
{
    char *trimmed = trim_copy(s);
    if (trimmed) {
        s = trimmed;
    }
    if (!s || !*s) {
        free(trimmed);
        hdf_raw(f, "");
        return;
    }
    if (!strcmp(s, "True")) {
        fputs("true", f);
        free(trimmed);
        return;
    }
    if (!strcmp(s, "False")) {
        fputs("false", f);
        free(trimmed);
        return;
    }
    if (!strcmp(s, "NULL")) {
        fputs("null", f);
        free(trimmed);
        return;
    }
    if (is_number(s)) {
        fputs(s, f);
        free(trimmed);
        return;
    }
    hdf_raw(f, s);
    free(trimmed);
}

static char *xstrdup(const char *s)
{
    char *p;
    if (!s)
        return NULL;
    p = malloc(strlen(s) + 1);
    if (p)
        strcpy(p, s);
    return p;
}

static char *res_name(Decl d)
{
    char *p = NULL;
    if (!d)
        return NULL;
    if (d->namesym)
        return xstrdup(get(d->namesym));
    if (asprintf(&p, "XtN%s", get(d->name)) < 0)
        return NULL;
    return p;
}

static char *res_class(Decl d)
{
    char *p = NULL;
    STRING s;
    if (!d)
        return NULL;
    s = d->namesym ? get_classname(d->namesym) : get_classname(d->name);
    if (asprintf(&p, "XtC%s", get(s)) < 0)
        return NULL;
    return p;
}

static char *res_type(Decl d)
{
    if (!d)
        return NULL;
    if (d->typesym)
        return trim_copy(get(d->typesym));
    return trim_copy(d->type ? get(d->type) : "");
}

static char *build_signature(Section s, Section s2)
{
    char *buf = NULL;
    size_t len = 0;
    FILE *m;
    Decl d;
    Section sig = s2 ? s2 : s;

    if (!s || !s->decl)
        return NULL;
    m = open_memstream(&buf, &len);
    if (!m)
        return NULL;
    if (s->decl->type)
        fprintf(m, "%s ", get(s->decl->type));
    fprintf(m, "%s(", get(s->decl->name));
    if (sig && sig->decl) {
        for (d = sig->decl->params; d; d = d->next) {
            if (d->type)
                fprintf(m, "%s ", get(d->type));
            if (d->name)
                fputs(get(d->name), m);
            if (d->suffix)
                fputs(get(d->suffix), m);
            if (d->next)
                fputs(", ", m);
        }
    }
    fputc(')', m);
    fclose(m);
    return buf;
}

static Class find_class_by_name(const char *name)
{
    Class c;
    for (c = classes; c; c = c->next)
        if (!strcmp(get(c->name), name))
            return c;
    return NULL;
}

static Class find_action_class(Class c, const char *name)
{
    Class h;
    Section s;
    if (!c)
        return NULL;
    if ((h = find_action_class(c->super, name)) != NULL)
        return h;
    for (s = c->actions; s; s = s->next)
        if (s->decl && s->decl->tp == Proc &&
            !strcmp(get(s->decl->name), name))
            return c;
    return NULL;
}

static Class find_utility_class(Class c, const char *name)
{
    Class h;
    Section s;
    if (!c)
        return NULL;
    if ((h = find_utility_class(c->super, name)) != NULL)
        return h;
    for (s = c->utilities; s; s = s->next)
        if (s->decl && s->decl->tp == Proc &&
            !strcmp(get(s->decl->name), name))
            return c;
    return NULL;
}

/* Resolve a |symbol| from a doc comment to an HDF anchor. */
static char *resolve_symbol_anchor(Class self, const char *token)
{
    char *anchor = NULL;
    char name[256];
    const char *n = token;
    Class owner;
    Section s;

    if (!token || !*token)
        return NULL;
    if (!strncmp(token, "widget/", 7) || !strncmp(token, "resource/", 9) ||
        !strncmp(token, "method/", 7) || !strncmp(token, "action/", 7) ||
        !strncmp(token, "utility/", 8) || !strncmp(token, "type/", 5))
        return xstrdup(token);

    if (!strncmp(token, "XtN", 3))
        n = token + 3;
    if (strlen(n) >= sizeof(name))
        return NULL;
    strcpy(name, n);

    owner = find_class_by_name(token);
    if (owner) {
        if (asprintf(&anchor, "widget/%s", token) < 0)
            return NULL;
        return anchor;
    }

    s = find_pubvar(self, hash(name));
    if (s && s->decl) {
        owner = find_instvar_class(self, hash(name));
        if (owner) {
            char *rn = res_name(s->decl);
            if (rn && asprintf(&anchor, "resource/%s/%s",
                               get(owner->name), rn) >= 0) {
                free(rn);
                return anchor;
            }
            free(rn);
        }
    }

    s = find_constraint(self, hash(name));
    if (s && s->decl) {
        owner = find_constr_class(self, hash(name));
        if (owner) {
            char *rn = res_name(s->decl);
            if (rn && asprintf(&anchor, "resource/%s/%s",
                               get(owner->name), rn) >= 0) {
                free(rn);
                return anchor;
            }
            free(rn);
        }
    }

    s = find_method(self, hash(name));
    if (s && s->decl) {
        owner = find_method_class(self, hash(name));
        if (owner && asprintf(&anchor, "method/%s/%s",
                              get(owner->name), name) >= 0)
            return anchor;
    }

    owner = find_action_class(self, name);
    if (owner && asprintf(&anchor, "action/%s/%s",
                          get(owner->name), name) >= 0)
        return anchor;

    owner = find_utility_class(self, name);
    if (owner && asprintf(&anchor, "utility/%s/%s",
                          get(owner->name), name) >= 0)
        return anchor;

    return NULL;
}

static int count_symbol_links(Class self, const char *text)
{
    const char *p = text;
    int n = 0;
    if (!text)
        return 0;
    while (*p) {
        if (*p == '|') {
            const char *q = strchr(p + 1, '|');
            char *token, *anchor;
            if (!q)
                break;
            token = strndup(p + 1, (size_t)(q - p - 1));
            anchor = token ? resolve_symbol_anchor(self, token) : NULL;
            if (anchor) {
                n++;
                free(anchor);
            }
            free(token);
            p = q + 1;
        } else {
            p++;
        }
    }
    return n;
}

static void emit_description_links(FILE *f, Class self, int indent,
                                   const char *text)
{
    const char *p = text;
    int count = count_symbol_links(self, text);
    int first = 1;

    if (count <= 0)
        return;
    hdf_indent(f, indent);
    fputs("description_links (seq\n", f);
    while (p && *p) {
        if (*p == '|') {
            const char *q = strchr(p + 1, '|');
            char *token, *anchor;
            if (!q)
                break;
            token = strndup(p + 1, (size_t)(q - p - 1));
            anchor = token ? resolve_symbol_anchor(self, token) : NULL;
            if (anchor) {
                (void)first;
                hdf_indent(f, indent + 1);
                fputs("(start ", f);
                fprintf(f, "%ld", (long)(p - text));
                fputs(" end ", f);
                fprintf(f, "%ld", (long)(q + 1 - text));
                fputs(" label ", f);
                hdf_atom(f, token);
                fputs(" target ", f);
                hdf_atom(f, anchor);
                fputs(" kind symbol)\n", f);
                first = 0;
                free(anchor);
            }
            free(token);
            p = q + 1;
        } else {
            p++;
        }
    }
    hdf_indent(f, indent);
    fputs(")\n", f);
}

static void emit_resource(FILE *f, Class self, Class owner, Section s,
                          int constraint)
{
    char *rn, *rc, *rt, *anchor = NULL;
    const char *text;

    if (!s || !s->decl)
        return;
    rn = res_name(s->decl);
    rc = res_class(s->decl);
    rt = res_type(s->decl);
    if (rn && owner)
        asprintf(&anchor, "resource/%s/%s", get(owner->name), rn);

    hdf_indent(f, 4);
    fputs("(kind resource\n", f);
    if (anchor) {
        hdf_indent(f, 6); fputs("anchor ", f); hdf_atom(f, anchor);
        fputc('\n', f);
    }
    hdf_indent(f, 6); fputs("name ", f); hdf_atom(f, rn);
    fputc('\n', f);
    hdf_indent(f, 6); fputs("class ", f); hdf_atom(f, rc);
    fputc('\n', f);
    hdf_indent(f, 6); fputs("type ", f); hdf_raw(f, rt ? rt : "");
    fputc('\n', f);
    hdf_indent(f, 6); fputs("default ", f);
    hdf_default(f, s->decl->value ? get(s->decl->value) : NULL);
    fputc('\n', f);
    text = s->text ? trim_copy(get(s->text)) : NULL;
    if (text && *text) {
        hdf_indent(f, 6); fputs("description ", f); hdf_raw(f, text);
        fputc('\n', f);
        emit_description_links(f, self, 6, text);
    }
    if (owner) {
        hdf_indent(f, 6); fputs("owner ", f); hdf_atom(f, get(owner->name));
        fputc('\n', f);
        hdf_indent(f, 6); fputs("owner_link widget/", f);
        hdf_atom(f, get(owner->name));
        fputc('\n', f);
        hdf_indent(f, 6); fputs("inherited ", f);
        hdf_bool(f, owner != self);
        fputc('\n', f);
    }
    hdf_indent(f, 6); fputs("constraint ", f); hdf_bool(f, constraint);
    fputc('\n', f);
    hdf_indent(f, 4); fputs(")\n", f);

    free(rn); free(rc); free(rt); free(anchor);
    free((void *)text);
}

static void emit_resources_for(FILE *f, Class self, Class part, int constraint)
{
    Section s;
    if (!part)
        return;
    if (!constraint) {
        for (s = part->publicvars; s; s = s->next)
            if (s->decl && s->decl->tp == Var &&
                find_instvar_class(part, s->decl->name) == part)
                emit_resource(f, self, part, s, 0);
    } else {
        for (s = part->constraints; s; s = s->next)
            if (s->decl && s->decl->tp == Var &&
                find_constr_class(part, s->decl->name) == part)
                emit_resource(f, self, part, s, 1);
    }
    emit_resources_for(f, self, part->super, constraint);
}

static void emit_method(FILE *f, Class self, Section s)
{
    Section first = find_method(self, s->decl->name);
    char *sig = build_signature(s, first);
    char *text = s->text ? trim_copy(get(s->text)) : NULL;

    hdf_indent(f, 4);
    fputs("(kind method\n", f);
    hdf_indent(f, 6); fputs("anchor method/", f);
    hdf_atom(f, get(self->name)); fputc('/', f);
    hdf_atom(f, get(s->decl->name)); fputc('\n', f);
    hdf_indent(f, 6); fputs("name ", f); hdf_atom(f, get(s->decl->name));
    fputc('\n', f);
    hdf_indent(f, 6); fputs("return_type ", f);
    hdf_raw(f, s->decl->type ? get(s->decl->type) : "void"); fputc('\n', f);
    hdf_indent(f, 6); fputs("signature ", f); hdf_raw(f, sig ? sig : "");
    fputc('\n', f);
    if (text && *text) {
        hdf_indent(f, 6); fputs("description ", f); hdf_raw(f, text);
        fputc('\n', f);
        emit_description_links(f, self, 6, text);
    }
    hdf_indent(f, 6); fputs("owner ", f); hdf_atom(f, get(self->name));
    fputc('\n', f);
    hdf_indent(f, 6); fputs("owner_link widget/", f);
    hdf_atom(f, get(self->name)); fputc('\n', f);
    hdf_indent(f, 6); fputs("override ", f);
    hdf_bool(f, first && first != s);
    fputc('\n', f);
    hdf_indent(f, 4); fputs(")\n", f);
    free(sig);
    free(text);
}

static void emit_utility(FILE *f, Class self, Section s)
{
    char *sig = build_signature(s, s);
    char *text = s->text ? trim_copy(get(s->text)) : NULL;

    hdf_indent(f, 4);
    fputs("(kind utility\n", f);
    hdf_indent(f, 6); fputs("anchor utility/", f);
    hdf_atom(f, get(self->name)); fputc('/', f);
    hdf_atom(f, get(s->decl->name)); fputc('\n', f);
    hdf_indent(f, 6); fputs("name ", f); hdf_atom(f, get(s->decl->name));
    fputc('\n', f);
    hdf_indent(f, 6); fputs("return_type ", f);
    hdf_raw(f, s->decl->type ? get(s->decl->type) : "void"); fputc('\n', f);
    hdf_indent(f, 6); fputs("signature ", f); hdf_raw(f, sig ? sig : "");
    fputc('\n', f);
    if (text && *text) {
        hdf_indent(f, 6); fputs("description ", f); hdf_raw(f, text);
        fputc('\n', f);
        emit_description_links(f, self, 6, text);
    }
    hdf_indent(f, 6); fputs("owner ", f); hdf_atom(f, get(self->name));
    fputc('\n', f);
    hdf_indent(f, 6); fputs("owner_link widget/", f);
    hdf_atom(f, get(self->name)); fputc('\n', f);
    hdf_indent(f, 4); fputs(")\n", f);
    free(sig);
    free(text);
}

static void emit_action(FILE *f, Class self, Section s)
{
    char *sig = build_signature(s, s);
    char *text = s->text ? trim_copy(get(s->text)) : NULL;

    hdf_indent(f, 4);
    fputs("(kind action\n", f);
    hdf_indent(f, 6); fputs("anchor action/", f);
    hdf_atom(f, get(self->name)); fputc('/', f);
    hdf_atom(f, get(s->decl->name)); fputc('\n', f);
    hdf_indent(f, 6); fputs("name ", f); hdf_atom(f, get(s->decl->name));
    fputc('\n', f);
    if (sig) {
        hdf_indent(f, 6); fputs("signature ", f); hdf_raw(f, sig);
        fputc('\n', f);
    }
    if (text && *text) {
        hdf_indent(f, 6); fputs("description ", f); hdf_raw(f, text);
        fputc('\n', f);
        emit_description_links(f, self, 6, text);
    }
    hdf_indent(f, 6); fputs("owner ", f); hdf_atom(f, get(self->name));
    fputc('\n', f);
    hdf_indent(f, 6); fputs("owner_link widget/", f);
    hdf_atom(f, get(self->name)); fputc('\n', f);
    hdf_indent(f, 4); fputs(")\n", f);
    free(sig);
    free(text);
}

static void emit_translation(FILE *f, Class self, Section s)
{
    const char *event = s->decl && s->decl->type ? get(s->decl->type) : "";
    const char *action = s->decl && s->decl->value ? get(s->decl->value) : "";
    char name[256];
    const char *paren;
    Class owner = NULL;

    if (action && *action) {
        paren = strchr(action, '(');
        if (paren) {
            size_t n = (size_t)(paren - action);
            if (n >= sizeof(name)) n = sizeof(name) - 1;
            memcpy(name, action, n);
            name[n] = 0;
        } else {
            strncpy(name, action, sizeof(name) - 1);
            name[sizeof(name) - 1] = 0;
        }
        owner = find_action_class(self, name);
    } else {
        name[0] = 0;
    }

    hdf_indent(f, 4);
    fputs("(kind translation\n", f);
    hdf_indent(f, 6); fputs("event ", f); hdf_raw(f, event); fputc('\n', f);
    hdf_indent(f, 6); fputs("action ", f); hdf_atom(f, name); fputc('\n', f);
    if (owner) {
        hdf_indent(f, 6); fputs("action_link action/", f);
        hdf_atom(f, get(owner->name)); fputc('/', f);
        hdf_atom(f, name); fputc('\n', f);
    }
    if (s->text) {
        char *text = trim_copy(get(s->text));
        hdf_indent(f, 6); fputs("description ", f);
        hdf_raw(f, text ? text : ""); fputc('\n', f);
        free(text);
    }
    hdf_indent(f, 4); fputs(")\n", f);
}

static void emit_export(FILE *f, Class self, Section s)
{
    const char *kind = "var";
    char *sig = NULL;

    if (s->decl) {
        switch (s->decl->tp) {
        case Proc: kind = "proc"; sig = build_signature(s, s); break;
        case Type: kind = "type"; break;
        case Def:  kind = "def";  break;
        default:   kind = "var";  break;
        }
    }
    hdf_indent(f, 4);
    fputs("(kind export\n", f);
    hdf_indent(f, 6); fputs("kind ", f); hdf_atom(f, kind); fputc('\n', f);
    if (s->decl) {
        hdf_indent(f, 6); fputs("name ", f); hdf_atom(f, get(s->decl->name));
        fputc('\n', f);
    }
    if (sig) {
        hdf_indent(f, 6); fputs("signature ", f); hdf_raw(f, sig);
        fputc('\n', f);
    }
    if (s->text) {
        char *text = trim_copy(get(s->text));
        hdf_indent(f, 6); fputs("description ", f);
        hdf_raw(f, text ? text : ""); fputc('\n', f);
        free(text);
    }
    hdf_indent(f, 4); fputs(")\n", f);
    free(sig);
}

static void emit_classvar(FILE *f, Class self, Section s)
{
    (void)self;
    if (!s->decl)
        return;
    hdf_indent(f, 4);
    fputs("(kind classvar\n", f);
    hdf_indent(f, 6); fputs("name ", f); hdf_atom(f, get(s->decl->name));
    fputc('\n', f);
    hdf_indent(f, 6); fputs("type ", f);
    hdf_raw(f, s->decl->type ? get(s->decl->type) : ""); fputc('\n', f);
    hdf_indent(f, 6); fputs("value ", f);
    hdf_default(f, s->decl->value ? get(s->decl->value) : NULL);
    fputc('\n', f);
    if (s->text) {
        char *text = trim_copy(get(s->text));
        hdf_indent(f, 6); fputs("description ", f);
        hdf_raw(f, text ? text : ""); fputc('\n', f);
        free(text);
    }
    hdf_indent(f, 4); fputs(")\n", f);
}

void generate_hdf_doc(FILE *f, Class c)
{
    Section s;
    char *text = c->text ? trim_copy(get(c->text)) : NULL;

    fputs("(rem \"Generated by wbuild; schema 1\")\n\n", f);
    fputs("(\n", f);
    hdf_indent(f, 2); fputs("kind widget\n", f);
    hdf_indent(f, 2); fputs("schema_version 1\n", f);
    hdf_indent(f, 2); fputs("anchor widget/", f);
    hdf_atom(f, get(c->name)); fputc('\n', f);
    hdf_indent(f, 2); fputs("class ", f); hdf_atom(f, get(c->name));
    fputc('\n', f);
    hdf_indent(f, 2); fputs("file ", f);
    hdf_raw(f, c->filename ? get(c->filename) : ""); fputc('\n', f);
    hdf_indent(f, 2); fputs("superclass ", f);
    if (c->super)
        hdf_atom(f, get(c->super->name));
    else
        fputc('%', f);
    fputc('\n', f);
    if (c->super) {
        hdf_indent(f, 2); fputs("superclass_link widget/", f);
        hdf_atom(f, get(c->super->name)); fputc('\n', f);
    }
    hdf_indent(f, 2); fputs("superclass_chain (seq\n", f);
    {
        Class p;
        for (p = c->super; p; p = p->super) {
            hdf_indent(f, 4);
            fputs("(class ", f); hdf_atom(f, get(p->name));
            fputs(" anchor widget/", f); hdf_atom(f, get(p->name));
            fputs(")\n", f);
        }
    }
    hdf_indent(f, 2); fputs(")\n", f);

    if (text && *text) {
        hdf_indent(f, 2); fputs("description ", f); hdf_raw(f, text);
        fputc('\n', f);
        emit_description_links(f, c, 2, text);
    }

    hdf_indent(f, 2); fputs("resources (seq\n", f);
    emit_resources_for(f, c, c, 0);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("constraints (seq\n", f);
    emit_resources_for(f, c, c, 1);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("methods (seq\n", f);
    for (s = c->methods; s; s = s->next)
        if (s->decl && s->decl->tp == Proc)
            emit_method(f, c, s);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("actions (seq\n", f);
    for (s = c->actions; s; s = s->next)
        if (s->decl && s->decl->tp == Proc)
            emit_action(f, c, s);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("translations (seq\n", f);
    for (s = c->translations; s; s = s->next)
        if (s->decl)
            emit_translation(f, c, s);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("exports (seq\n", f);
    for (s = c->exports; s; s = s->next)
        if (s->decl)
            emit_export(f, c, s);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("classvars (seq\n", f);
    for (s = c->classvars; s; s = s->next)
        if (s->decl)
            emit_classvar(f, c, s);
    hdf_indent(f, 2); fputs(")\n", f);

    hdf_indent(f, 2); fputs("utilities (seq\n", f);
    for (s = c->utilities; s; s = s->next)
        if (s->decl && s->decl->tp == Proc)
            emit_utility(f, c, s);
    hdf_indent(f, 2); fputs(")\n", f);

    fputs(")\n", f);
    free(text);
}
