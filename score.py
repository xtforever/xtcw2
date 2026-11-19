#!/usr/bin/env python3
"""score.py: compute the "nesting + pointer indirection + dereference" metric
per C function.

Usage:  score.py FILE.c
        score.py --selftest

For every function found it prints the c(i) nesting histogram, the p(k)
pointer-indirection histogram, the d(j) dereference histogram, the three
score factors, and the final score.

    score(f) = ( sum_{i>=1} c(i) * 2^(i-1) )
             * ( prod_{k>=1} (k+1)^p(k) )
             * ( prod_{j>=1} (j+1)^d(j) )

c(i) = number of statements of f at nesting (brace) depth i. A statement is
       counted once no matter how many physical lines it spans (line-wrapped
       calls or conditions do not inflate the score); compound-literal and
       initializer braces are not counted as nesting. Depth-0 lines are
       ignored (the sum starts at i=1).
p(k) = number of pointer objects in f (params + locals) with indirection
       level k: `int *s` -> 1, `int **s` -> 2, `int ***p` -> 3.
d(j) = number of `->` dereferences at chain position j, for j >= 2 only. A
       chain `a->x->y` has dereferences at positions 1 and 2; position 1 (the
       `a->x` part, whose null is usually local/obvious) is not counted, while
       position 2 (`->y`, dereferencing a pointer that came from somewhere
       else) is. Only runs of `-> id -> id ...` chain together. Counting every
       `->` multiplicatively explodes the score and drowns the signal, so
       isolated `a->x` dereferences are ignored entirely.
"""

import sys

KEYWORDS = frozenset("""
auto break case char const continue default do double else enum extern float
for goto if inline int long register restrict return short signed sizeof
static struct switch typedef union unsigned void volatile while _Bool
""".split())

TYPE_TOKENS = frozenset(
    "void char short int long float double signed unsigned _Bool struct union enum".split())

QUALIFIERS = frozenset("const volatile restrict static extern register auto inline".split())


def tokenize(src):
    """List of (kind, value, line). Comments and preprocessor lines dropped;
    string/char literals are single tokens so braces inside them don't count."""
    toks = []
    i, n, line = 0, len(src), 1
    while i < n:
        c = src[i]
        if c == '\n':
            line += 1
            i += 1
            continue
        if c.isspace():
            i += 1
            continue
        if c == '#' or src.startswith('//', i):
            j = src.find('\n', i)
            if j == -1:
                break
            line += 1
            i = j + 1
            continue
        if src.startswith('/*', i):
            j = src.find('*/', i + 2)
            if j == -1:
                line += src[i:].count('\n')
                break
            line += src[i:j + 2].count('\n')
            i = j + 2
            continue
        if c in ('"', "'"):
            j = i + 1
            while j < n:
                if src[j] == '\\':
                    j += 2
                    continue
                if src[j] == c:
                    j += 1
                    break
                j += 1
            toks.append(('str', src[i:j], line))
            i = j
            continue
        if c.isalpha() or c == '_':
            j = i + 1
            while j < n and (src[j].isalnum() or src[j] == '_'):
                j += 1
            toks.append(('id', src[i:j], line))
            i = j
            continue
        if c.isdigit():
            j = i + 1
            while j < n and (src[j].isalnum() or src[j] in '._'):
                j += 1
            toks.append(('num', src[i:j], line))
            i = j
            continue
        if src.startswith('->', i):
            toks.append(('op', '->', line))
            i += 2
            continue
        toks.append(('op', c, line))
        i += 1
    return toks


def find_matching_close(toks, open_idx):
    depth = 1
    for j in range(open_idx + 1, len(toks)):
        if toks[j][1] == '{':
            depth += 1
        elif toks[j][1] == '}':
            depth -= 1
            if depth == 0:
                return j
    return None


def find_functions(toks):
    """(name, line, param_toks, body_toks) for each function definition."""
    funcs = []
    n = len(toks)
    i = 0
    while i < n:
        k, v, ln = toks[i]
        if k == 'id' and v not in KEYWORDS and i + 1 < n and toks[i + 1][1] == '(':
            j, depth = i + 2, 1
            while j < n and depth:
                if toks[j][1] == '(':
                    depth += 1
                elif toks[j][1] == ')':
                    depth -= 1
                j += 1
            if j < n and toks[j][1] == '{':
                close = find_matching_close(toks, j)
                if close is not None:
                    funcs.append((v, ln, toks[i + 2:j - 1], toks[j + 1:close]))
                    i = close + 1
                    continue
        i += 1
    return funcs


def split_top(toks, sep):
    parts, cur, depth = [], [], 0
    for t in toks:
        if t[1] == '(':
            depth += 1
        elif t[1] == ')':
            depth -= 1
        elif t[1] == sep and depth == 0:
            parts.append(cur)
            cur = []
            continue
        cur.append(t)
    parts.append(cur)
    return parts


def param_pointers(param_toks):
    levels = []
    for part in split_top(param_toks, ','):
        if not part:
            continue
        # function-pointer/function declarator: '(' '*'* NAME ( ... )
        # scored as NAME's own star level, not all stars before the last id
        # (which is inside the argument list).
        handled = False
        for i, t in enumerate(part):
            if t[0] != 'op' or t[1] != '(':
                continue
            j = i + 1
            while j < len(part) and part[j][0] == 'op' and part[j][1] == '*':
                j += 1
            if j < len(part) and part[j][0] == 'id' and part[j][1] not in KEYWORDS:
                if j - i - 1:
                    levels.append(j - i - 1)
                handled = True
                break
        if handled:
            continue
        ids = [(i, t[1]) for i, t in enumerate(part) if t[0] == 'id']
        if not ids:
            continue
        pos, name = ids[-1]
        if name in KEYWORDS:  # 'void', 'char', ... : not a variable name
            continue
        stars = sum(1 for t in part[:pos] if t[0] == 'op' and t[1] == '*')
        if stars:
            levels.append(stars)
    return levels


def is_declaration(stmt):
    if not stmt:
        return False
    i = 0
    while i < len(stmt) and stmt[i][0] == 'id' and stmt[i][1] in QUALIFIERS:
        i += 1  # skip storage-class / qualifier prefix (static, const, ...)
    if i >= len(stmt):
        return False
    k, v = stmt[i][0], stmt[i][1]
    if k != 'id':
        return False
    if v in TYPE_TOKENS:
        return True
    if v in KEYWORDS:
        return False
    # ponytail: unknown leading id treated as a typedef'd type name, unless it
    # looks like a call/assignment. Ceiling: typedef'd function-pointer types
    # are still missed; the fix is a real C parser.
    if len(stmt) >= i + 2:
        v2 = stmt[i + 1][1]
        if v2 in ('=', '(', ')', '[', ']', '{'):
            return False
    return True


def declarator_levels(stmt):
    levels = []
    for part in split_top(stmt, ','):
        decl = [t for t in part if t[1] != '=']  # drop initializers
        for i, t in enumerate(decl):
            if t[0] == 'id' and t[1] not in KEYWORDS:
                stars = sum(1 for x in decl[:i] if x[0] == 'op' and x[1] == '*')
                if stars:
                    levels.append(stars)
                break
    return levels


def local_pointers(body_toks):
    levels = []
    depth, start = 0, 0
    for i, (_, v, _) in enumerate(body_toks):
        if v == '(':
            depth += 1
        elif v == ')':
            depth -= 1
        elif v == ';' and depth == 0:
            stmt = body_toks[start:i]
            if is_declaration(stmt):
                levels.extend(declarator_levels(stmt))
            start = i + 1
    return levels


BLOCK_STARTERS = frozenset(") else do try".split())


def nesting_counts(body_toks):
    """c(i): number of statements at brace depth i. Counts one unit per
    statement (token run ending at ';' or a block opener), so line-wrapped
    calls/conditions don't inflate the depth-weighted sum. Initializer /
    compound-literal braces are tracked but not counted as nesting."""
    c = {}
    depth = 1  # body is inside the function's opening brace
    paren = 0
    stmt_open = True
    prev = None
    braces = []  # stack of brace tags: 1 = block, 0 = initializer/literal
    for k, v, _ in body_toks:
        if v == '(':
            paren += 1
            continue
        if v == ')':
            paren -= 1
            if paren == 0:
                prev = v  # ')' closes a control-flow condition; '{' after it is a block
            continue
        if paren != 0:
            continue
        if v == '{':
            if prev is None or prev in BLOCK_STARTERS or prev in ';}':
                braces.append(1)
                depth += 1
                stmt_open = True
            else:
                braces.append(0)  # = { ... } / compound literal: mid-statement
        elif v == '}':
            if braces and braces.pop():
                depth -= 1
                stmt_open = True
            # initializer close: still mid-statement, stmt_open unchanged
        elif v == ';':
            stmt_open = True
        elif stmt_open:
            if depth >= 1:
                c[depth] = c.get(depth, 0) + 1
            stmt_open = False
        prev = v
    return c


def deref_counts(body_toks):
    """d(j): number of `->` dereferences at chain position j (j >= 2 only).
    A run of `-> id -> id ...` is one chain; position 1 is ignored because the
    null there is on a locally-visible variable, while deeper positions
    dereference pointers that typically come from elsewhere."""
    d = {}
    chain = 0
    i, n = 0, len(body_toks)
    while i < n:
        if body_toks[i][1] == '->':
            chain += 1
            if chain >= 2:
                d[chain] = d.get(chain, 0) + 1
            if (i + 2 < n and body_toks[i + 1][0] == 'id'
                    and body_toks[i + 2][1] == '->'):
                i += 2
                continue
            chain = 0
        i += 1
    return d


def metric(name, line, param_toks, body_toks):
    p = {}
    for lvl in param_pointers(param_toks) + local_pointers(body_toks):
        p[lvl] = p.get(lvl, 0) + 1

    c = nesting_counts(body_toks)
    d = deref_counts(body_toks)

    sum_part = sum(v * (2 ** (i - 1)) for i, v in c.items())
    prod_part = 1
    for kk, vv in p.items():
        prod_part *= (kk + 1) ** vv
    deref_part = 1
    for kk, vv in d.items():
        deref_part *= (kk + 1) ** vv
    return c, p, d, sum_part, prod_part, deref_part, \
        sum_part * prod_part * deref_part


def fmt_c(c):
    if not c:
        return "0"
    return " + ".join(f"{v}*2^{i-1}" for i, v in sorted(c.items()))


def fmt_p(p):
    if not p:
        return "1"
    return " * ".join(f"{k+1}^{v}" for k, v in sorted(p.items()))


def report(path):
    with open(path) as fh:
        src = fh.read()
    funcs = find_functions(tokenize(src))
    if not funcs:
        print("no functions found")
        return
    for name, ln, ptoks, btoks in funcs:
        c, p, d, sp, pp, dp, score = metric(name, ln, ptoks, btoks)
        print(f"{name} (line {ln})")
        print(f"  c(i): {dict(sorted(c.items())) if c else {}}")
        print(f"  p(k): {dict(sorted(p.items())) if p else {}}")
        print(f"  d(j): {dict(sorted(d.items())) if d else {}}")
        print(f"  nesting term: {fmt_c(c)} = {sp}")
        print(f"  pointer term: {fmt_p(p)} = {pp}")
        print(f"  deref term:   {fmt_p(d)} = {dp}")
        print(f"  score: {score}")
        print()


SAMPLE = r"""
int *foo(char **a, int *b) {
    int **x;
    int y;
    if (y) {
        y++;
    }
    return a[0] ? *b : y;
}
"""


def selftest():
    funcs = find_functions(tokenize(SAMPLE))
    assert len(funcs) == 1, funcs
    name, ln, ptoks, btoks = funcs[0]
    assert name == 'foo' and ln == 2
    c, p, d, sp, pp, dp, score = metric(name, ln, ptoks, btoks)
    assert c == {1: 4, 2: 1}, c
    assert p == {1: 1, 2: 2}, p
    assert d == {}, d
    assert sp == 6, sp            # 4*2^0 + 1*2^1
    assert pp == 18, pp           # 2^1 * 3^2
    assert dp == 1, dp
    assert score == 108, score

    # function-pointer params are level-1 (their own star), and the real
    # params are not dropped in favour of the innermost arg
    FP = r"""
int fp(int (*cb)(void *ctx, int n, void *data), void *ctx) {
    return cb(ctx, 0, 0);
}
"""
    funcs = find_functions(tokenize(FP))
    c, p, d, sp, pp, dp, score = metric(*funcs[0])
    assert p == {1: 2}, p        # cb + ctx, both level-1 (not level-3)
    assert pp == 4, pp           # 2^2
    assert sp == 1, sp
    assert score == 4, score

    # a line-wrapped call must not inflate the nesting term
    WRAPPED = r"""
int w(char **s) {
    if (a) {
        if (b) {
            foo (s,
                 STR (x,
                      i +
                          1),
                 0);
        }
    }
    return 0;
}
"""
    COMPACT = r"""
int w(char **s) {
    if (a) {
        if (b) {
            foo (s, STR (x, i + 1), 0);
        }
    }
    return 0;
}
"""
    c1 = metric(*find_functions(tokenize(WRAPPED))[0])[3]
    c2 = metric(*find_functions(tokenize(COMPACT))[0])[3]
    assert c1 == c2 == 8, (c1, c2)  # 2 + 2 + 4, same for both layouts

    # static/const local pointers are counted too
    Q = r"""
int q(void) {
    static char *s;
    const char *t;
    return 0;
}
"""
    p = metric(*find_functions(tokenize(Q))[0])[1]
    assert p == {1: 2}, p

    # `->` chains: a->next->next has a position-2 dereference; a->next alone
    # (position 1) is ignored as the local/obvious case
    D = r"""
int chain(struct node *a, struct node *b) {
    a->next->next = b;
    return a->next ? 1 : 0;
}
"""
    d = metric(*find_functions(tokenize(D))[0])[2]
    dp = metric(*find_functions(tokenize(D))[0])[5]
    assert d == {2: 1}, d
    assert dp == 3, dp           # 3^1
    print("selftest ok")


if __name__ == '__main__':
    if len(sys.argv) >= 2 and sys.argv[1] == '--selftest':
        selftest()
    elif len(sys.argv) == 2:
        report(sys.argv[1])
    else:
        print(__doc__)
        sys.exit(1)
