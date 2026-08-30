# re-tex: Embeddable TeX-inspired Layout Engine

## Core Philosophy
“Implement a small, embeddable, box-based layout engine using TeX’s proven ideas and constants.”

**Goals:**
*   Zero subprocesses
*   Predictable latency
*   No global state (clean reentrancy)
*   Tight Cairo integration
*   **Language:** C (using the `memc` library for safe memory management)

## Architecture Pipeline

1.  **Tokenizer**: UTF-8 stream → Tokens (Character codes, Command codes).
2.  **Macro Expander** (Lightweight): Handling `\def`, `\if`, expansion stacks.
3.  **Semantic Parser (The Stomach)**:
    *   **Mode Handling**: Vertical (Page building), Horizontal (Paragraph building), Math.
    *   **Grouping**: Scopes for font changes and parameter settings.
4.  **Layout Engine**:
    *   **Paragraph Builder**: Knuth-Plass line breaking (HLists).
    *   **Page Builder**: Vertical list accumulation, insertion handling (footnotes), page breaking (VLists).
    *   **Math Engine**: Math Lists (MLists) → Horizontal Lists (HLists).
    *   **Alignment**: Table layout (`\halign` logic).
5.  **Backend**:
    *   **Font Manager**: FreeType/Cairo integration.
    *   **Renderer**: Traversal of the final Box Tree → Cairo drawing commands.

## Memory Management Strategy (memc)
The project will strictly use the provided `memc` library for all dynamic memory allocation.
*   **Handle-Based Access**: All dynamic memory is referenced via integer handles (`int m`), not raw pointers.
*   **Lifecycle**: The system initializes with `m_init()` and cleans up with `m_destruct()`.
*   **Strings**: Use `s_printf`, `s_cstr` (for constant strings), and `s_app` for string manipulation.
*   **Lists**: Use `m_create`, `m_put`, and `m_foreach` for dynamic arrays (e.g., node lists).
*   **Array Access & Allocation**: Arrays must be accessed via `mls(handle, index)`. Only allocated elements are accessible. Allocation is performed using `m_put` (to append) or `m_setlen` (to set explicit length).
*   **Safety**: Leverage `m_free` for safe deallocation and `m_buf` only when raw pointer access is strictly necessary for short scopes.

## Headless Testing Strategy
To ensure layout correctness without manual visual inspection, the engine will use a three-tiered automated testing approach:

1.  **Canonical Box Tree Serialization**:
    *   Implement a `box_dump(int handle)` function that traverses the final Box Tree and produces a structured, machine-readable text representation (similar to TeX's `\showbox` or `trip.log`).
    *   This dump will include exact coordinates (x, y), dimensions (w, h, d), glue settings, and child nodes.

2.  **Golden Master Testing**:
    *   Store "known-good" serialized box trees in `tests/golden/`.
    *   Test cases will generate a dump and perform a strict `diff` against the golden file. Any discrepancy in geometry or node order will trigger a failure.

3.  **Property-Based Invariants**:
    *   Verify layout logic via assertions:
        *   **Summation**: Inner node widths + glue stretch/shrink must equal the outer hbox width.
        *   **Badness Calculation**: Assert that the calculated badness for a specific set of nodes matches expected TeX constants (reference `trip.log`).
        *   **Overflow**: Assert that `Overfull \hbox` warnings are triggered when expected.

4.  **Structured Debugging**:
    *   Use `memc`'s `TRACE` and custom `%M` printf formatters to log state transitions in the paragraph/page builders, allowing for "white-box" testing of the layout algorithms.

## Requirements & Roadmap

### Phase 1: Foundations
*   [ ] **Basic Types**: Implement `Scaled` (fixed-point arithmetic), `Glue` (shrink/stretch), `Box` (hbox/vbox), `Penalty`, `Kern`. Reference `glue.web` for exact fixed-point logic to ensure stability.
*   [ ] **Memory Management**: Tree-based node structure (no global `mem` array).
*   [ ] **Tokenizer**: Implement basic scanning states (skipping blanks, reading command sequences).

### Phase 2: The Core Engines
*   [ ] **Hyphenation**: Implement Liang’s algorithm (trie-based pattern matching).
*   [ ] **Line Breaking**: Port the Knuth-Plass algorithm. Support `\tolerance`, `\pretolerance`.
*   [ ] **Page Builder**: Implement the Output Routine logic. Accumulate vertical lists, handle `\vsplit` and insertions.
*   [ ] **Alignment**: Implement the two-pass `\halign` algorithm (calculating max widths of columns, then setting glue).

### Phase 3: Math Mode
*   [ ] **Math Parsing**: Parse math mode input into `noad` trees (Ord, Op, Bin, Rel, Open, Close, Punct, Inner).
*   [ ] **Math Layout**: Implement the `mlist_to_hlist` conversion using TeX's spacing tables and rules (Appendix G of TeXbook).

### Phase 4: Font & Backend
*   [ ] **Metric Extraction**: Use FreeType to load TTF/OTF. Convert metrics (advance, bounding box, kerns) to TeX internal scaled points.
*   [ ] **Cairo Renderer**: Recursive traversal of the box tree. Draw glyphs and rules.

## "Steal Shamelessly" (Reuse from TeX)
*   **Algorithms**: Knuth-Plass line breaking, hyphenation tries, math spacing rules.
*   **Constants**: Default penalties, glue values, and magic numbers from `tex.web`.
*   **Logic**: TFM parsing (conceptually, mapped to FreeType), Glue setting mathematics (`glue_fix` in `glue.web`).

## Key Differences from TeX
*   **Fonts**: No TFM/PK dependency. Direct use of system fonts via FreeType.
*   **Output**: Direct Cairo calls instead of DVI generation.
*   **State**: Context object passed explicitly (no global variables).