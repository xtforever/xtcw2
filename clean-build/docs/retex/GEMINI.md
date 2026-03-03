# Project: re-tex

## Overview
`re-tex` is a lightweight, embeddable, box-based layout engine inspired by TeX. It aims to leverage TeX's proven layout algorithms while avoiding its legacy constraints.

**For full documentation, architecture details, and usage guides, see [README.md](./README.md).**

## Key Design Decisions
*   **Font Handling**: Use FreeType + TTF for glyph metrics instead of TFM/PK files.
*   **Rendering**: Direct integration with Cairo.
*   **State Management**: No global state, allowing for zero subprocesses and clean reentrancy.
*   **Memory**: Uses `memc` (mls) handle-based memory management.

## Current Status
*   **Phase**: Core engines implemented (Linebreaking, Basic Math).
*   **Next Steps**: Implementation of Advanced Math Rules and Table Layout.

