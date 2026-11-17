# Dialog Style Guide (LLM Edition)

This is the operational companion to `style-guide.md` §10. It is written to be
pasted into an LLM context window so the model can design and implement dialogs
that fit this toolkit instead of inventing its own conventions.

**Source of truth.** `style-guide.md` defines the tokens, colour roles, widget
recipes and general layout rules. This file only makes the dialog part
executable. If the two disagree, `style-guide.md` wins; then fix this file.

**Rule keywords.** `MUST` = non-negotiable. `SHOULD` = default; deviate only
with a stated reason. `MAY` = optional.

**At a glance.** Prefer inline > modeless > sheet > modal > alert. One accent
primary per dialog; Cancel always available; destructive is never the default.
Title = task, body = why, every field labelled, errors inline. Return = safe
default, Escape = cancel, focus starts safe and is restored. Theme roles and the
spacing scale only; `font_ui` for controls. Fit the smallest window, keep the
width stable, and design every state. If you paste only one block into a prompt,
use §16.

**Read order for the model:** §1 brief → §2 type → §3 hard rules → §4 anatomy →
§5 widgets → §6 keyboard → §7 validation → §8 microcopy → §9 patterns →
§10 sizing → §11 theme → §13 self-check → §14 output contract → §15 examples.

---

## 1. The Dialog Brief (fill this in before designing)

### 1.1 Template

Copy this template and complete it first. If a field is missing from the user
request, use the defaults in §1.2 and list the assumption in the final answer.

```
goal:            What must the user accomplish or decide?
trigger:         What opens the dialog? (menu, button, shortcut, system event)
scope:           app | window | document
blocking:        yes | no
title:           Short task/question, no app name
body:            1–2 sentences of context; may be empty for trivial input
fields:          label, widget, initial value, validation (per field)
primary:         Verb label for the main action
secondary:       Usually Cancel
destructive:     yes | no; what is lost and whether it is reversible
default_action:  What Return does
cancel_action:   What Escape does
initial_focus:   First safe control
states:          default | invalid | busy | error | success
size:            default width, min height, scrolling behaviour
async:           yes | no; cancellable?
```

### 1.2 Defaults when information is missing

| Field | Default |
|-------|---------|
| `scope` | `window` if a parent window exists, otherwise `app` |
| `blocking` | `yes` for modal/sheet, `no` for modeless |
| `title` | Primary verb + object ("Rename “Report.txt”") |
| `body` | Omit for simple input; include for destructive, irreversible or error cases |
| `primary` | Derive from the goal; if destructive, see D4 (no accent primary) |
| `secondary` | `Cancel` |
| `default_action` | Primary, except destructive confirmations (default = Cancel) |
| `cancel_action` | `Cancel` / close without applying |
| `initial_focus` | First editable field, else the first safe action |
| `states` | `default`, `invalid`, `busy`, `error`, `success` |
| `size` | 360 px alert, 420 px form, 520 px list; never wider than the parent |
| `async` | `no` unless the action can take longer than ~1 s |

### 1.3 Ask before inventing

The model SHOULD ask only when a missing answer changes the safety or data model
of the dialog, for example: "Is deleting this reversible?", "Is this dialog
window-scoped or app-wide?", "Which fields are required?". Otherwise choose the
documented default and state it.

---

## 2. Choose the dialog type

| Situation | Type | Blocking | Typical buttons | Notes |
|-----------|------|----------|-----------------|-------|
| Task cannot continue without an answer, no sensible parent | **Modal dialog** | App | Cancel + primary | Last resort; prefer a sheet |
| Dialog belongs to one window/document (open, save, rename) | **Sheet** | Parent window | Cancel + primary | Default for document/window work |
| Short message or confirmation, no input | **Alert** | Parent | Cancel + safe action | At most 2–3 buttons |
| Inspector, palette or properties used while editing | **Modeless panel** | None | Close | Follows selection; never steals focus |
| Long-running work | **Progress dialog** or modeless progress | Depends | Cancel (if safe) | Prefer inline/status if work can continue |
| Dependent multi-step setup | **Wizard** in a sheet/modal | Parent | Back / Next / Finish | Only if a single form is genuinely insufficient |

### 2.1 Prefer less interruption

Use the lightest mechanism that works, in this order:

1. **Inline** — edit in place, inline validation, status text.
2. **Modeless panel** — properties, inspector, palette.
3. **Sheet** — window/document-scoped task.
4. **Modal dialog** — only when there is no sensible parent.
5. **Alert** — only for a blocking message or a destructive confirmation.

This ordering is about interruption depth, not the type of a document task.
Open/save/rename belong in a sheet even though a modeless panel appears earlier
in the list, because they are scoped to one window and must block that window.

`MUST NOT` use a dialog for information that belongs in the main window (empty
states, status, hints, success messages that are already visible).

### 2.2 Never stack

`MUST NOT` open a dialog from a dialog. If a second view is needed, either close
the first, replace the dialog body in place, or use a sheet on the same parent.
No nested modals.

---

## 3. Hard rules

These are the rules a reviewer or an automated check can verify.

### Structure and hierarchy

- **D1 — One primary action.** Exactly one button is visually primary (accent).
  Everything else is secondary. Destructive confirmations are the documented
  exception: they have no accent primary (see D4).
- **D2 — Always a way out.** Every dialog has a visible Cancel/Close action and
  Escape works. If closing is unsafe, explain why instead of hiding the button.
- **D3 — Safe default.** Return triggers the primary only when that action is
  non-destructive and the form is valid. Otherwise Return triggers the safe
  action or validates.
- **D4 — Destructive is never default or accent.** A destructive action uses
  danger styling, is never the Return default, and requires an explicit click
  (or focus + Space). Name the object and consequence.
- **D5 — Title states the task.** One short question or verb phrase. No app name,
  no "Dialog", no trailing period. Use `font_heading2` (default).
- **D6 — Body gives the why before the controls.** One or two sentences. Skip it
  for trivial input; never use it to repeat the title.
- **D7 — Every field has a visible label.** A placeholder is a hint, not a label.
- **D8 — Theme roles only.** No hard-coded colours, no `XtDefaultForeground`, no
  literal white/black inputs. Resolve every colour from the theme.
- **D9 — Spacing scale only.** Use XS/S/M/L/XL (4/8/12/16/20) and the theme keys
  `window_margin`, `control_gap`, `group_gap`, `action_gap`. No ad-hoc pixels.
- **D10 — One UI font.** Controls use `font_ui`; headings use `font_heading2`;
  secondary text uses `font_small`. Monospace only for code/editor content.
- **D11 — Fit and adapt.** The dialog fits the smallest supported window; text
  wraps and long content scrolls. Nothing clips or horizontal-scrolls.
- **D12 — Stable width.** Validation, busy states and help text do not resize the
  dialog horizontally. Height may grow; width stays fixed.

### Actions and keyboard

- **D13 — Return = default, Escape = cancel.** Multiline editors are the
  exception: Return inserts a newline; use Ctrl+Enter or the primary button.
- **D14 — Reading-order tab chain.** Tab/Shift+Tab follow top → bottom,
  leading → trailing. Modal dialogs trap focus; modeless panels do not.
- **D15 — Initial focus is safe.** First editable field or the content list; never
  a destructive button. Focus the first thing the user must act on, not merely
  the first control in the widget tree.
- **D16 — Restore focus.** On close, return focus to the control that opened the
  dialog.
- **D17 — Visible focus.** The focused control has a clear indicator (accent or
  the toolkit's native ring) with at least 3:1 contrast against its surroundings.

### Content and state

- **D18 — Inline validation.** Errors appear next to the field that failed, not
  in a distant status bar. Keep the dialog open and focus the first invalid field.
- **D19 — Preserve input.** Never clear the form on error. Keep typed values and
  selection.
- **D20 — Disable only when obvious.** Prefer allowing the click and explaining
  the error. If a control is disabled, the reason must be visible without a
  tooltip.
- **D21 — Busy state is explicit.** While working: disable actions that would
  duplicate the work, show progress/status, keep the UI responsive, and offer
  Cancel if cancellation is safe.
- **D22 — Handle all outcomes.** Default, invalid, busy, error and success states
  are designed. No dead ends, no silent failures, no auto-close that hides the
  result.
- **D23 — Verb labels.** Buttons use Title Case verbs ("Save", "Replace",
  "Delete"). Avoid "OK" when a verb exists. Use "…" only when the action opens
  another view or needs more input ("Choose…", "Open…"). The button that opens
  a dialog gets the ellipsis; the dialog's primary that completes the task does
  not ("Open", "Save", "Rename").
- **D24 — Sentence case for prose.** Body text, help text and error messages use
  sentence case. Titles and buttons use Title Case. Be consistent per dialog.
- **D25 — No stacked dialogs.** See §2.2.

### SHOULD

- **S1** Prefer a sheet over a modal; prefer inline editing over a dialog.
- **S2** Keep body text to ≤ 2 sentences and ~45–65 characters per line.
- **S3** Use one form layout per dialog (stacked labels or inline labels, not both).
- **S4** Offer Undo instead of confirmation for reversible actions.
- **S5** Show determinate progress for work longer than ~1 s.
- **S6** Put a help button bottom-left, opposite the dismissal row, if needed.
- **S7** Use progressive disclosure for advanced options instead of crowding.
- **S8** Remember modeless panel size/position per window kind.
- **S9** Verify light and dark themes and the smallest supported window.
- **S10** Center a modal on its parent; attach a sheet to the parent's top edge.

---

## 4. Anatomy and layout

```
┌─────────────────────────────────────────────────────┐
│  Title (font_heading2)                              │  L margin, S below
│                                                     │
│  Body text, wraps; optional help (font_small)       │  S between blocks
│                                                     │
│  Label                                              │  form rows, S gap
│  [ input                                          ] │
│  inline error (font_small, hidden by default)       │
│                                                     │
│  Label                                              │
│  [ input                                          ] │
│                                                     │
│                              [Cancel]  [Primary]    │  action bar, L inset
└─────────────────────────────────────────────────────┘
```

### 4.1 Spacing recipe

| Gap | Token | px |
|-----|-------|----|
| Outer dialog margin (all sides) | `window_margin` | 16 |
| Title → body | `S` | 8 |
| Body → first field / group | `group_gap` | 12 |
| Label → field | `S` | 8 |
| Field → inline error | `XS` | 4 |
| Field row → next field row | `group_gap` | 12 |
| Last field → action bar | `L` | 16 |
| Between action buttons | `action_gap` | 8 |
| Action bar → bottom/right edge | `window_margin` | 16 |

If the toolkit's `Gridbox` forces one `defaultDistance` for outer margin and
inter-cell gaps, wrap the content in a nested `VBox`/`Gridbox` so the outer
margin can stay at 16 px while inner gaps use 8/12 px.

### 4.2 Width and height

- Alerts: **360 px** default.
- Forms: **420 px** default.
- Lists/file choosers: **520 px** default.
- Hard maximum: **80% of the parent width**, or the screen work area minus
  margins; never force the parent to resize.
- Height is content-driven. If the body exceeds ~70% of the parent height, keep
  the title and action bar fixed and scroll the body.
- Reserve a stable error area or allow vertical growth; never shift the action
  bar off-screen when an error appears.

### 4.3 Form layouts

Pick **one** per dialog:

**Stacked (default for narrow dialogs and long labels)**

```
Label
[ field........................ ]
error / help
```

**Inline (default for short labels and wide dialogs)**

```
Label            [ field........................ ]
Label            [ field........................ ]
```

For inline rows, the label column width is the widest label + `S`; fields share
a common leading edge. Labels are top-aligned with their fields when a field is
taller than one line.

### 4.4 Action bar

- Bottom, right-aligned, `window_margin` inset.
- Secondary actions leading, primary trailing, `action_gap` between.
- All buttons share `control_height` and the same font.
- Cancel is always left of the primary in this toolkit.
- A help button, if present, sits bottom-left on the same row.
- Do not put critical controls anywhere else near the bottom edge.

---

## 5. Widget recipes for dialogs

| Element | Surface / text | Metrics and notes |
|---------|----------------|-------------------|
| **Primary button** | `accent` / `accent_fg` | `control_height`, `control_padding`, `font_ui`; flat, no bevel; one per dialog |
| **Secondary button** | `button_bg` / `button_fg` | Same size as primary; hover/armed `bg_hi` / `fg_hi` |
| **Destructive button** | danger role if defined, else `button_bg` / `button_fg` + warning icon | Never `accent`; explicit object in label; not the Return default |
| **Text input** | `input_bg` / `input_fg` | `control_height`, `font_ui`, 1 px width (`control_border`) in the `input_border` colour, `cornerRoundPercent = 0` |
| **Label** | `fg_norm` | `font_ui`; sentence case; visible for every field |
| **Help text** | `fg_norm` | `font_small`; below the field/group; optional |
| **Inline error** | error role if defined, else `fg_norm` + warning glyph | `font_small`; below the field; `XS` gap; never hard-coded red |
| **Checkbox** | label `fg_norm`; checked `accent` | Same font as buttons; whole row is the hit target |
| **List** | `list_bg` / `list_fg`; selection `list_sel_bg` / `list_sel_fg` | Header `list_header_height` (24), rows `list_row_height` (26), cell pad 8/2, icons 16 |
| **Separator** | `list_line` | 1 px; use sparingly to group, not around every row |
| **Progress** | track `bg_hi`, fill `accent` | 8 px (`S`) determinate bar; status text `font_small`; indeterminate if unknown |
| **Busy label** | `fg_norm` | `font_small`; verb + object ("Saving…"), not "Please wait" |
| **Window surface** | `bg_norm`; containers `gridbox_bg` | No busy backgrounds behind controls |

### 5.1 Optional theme extensions

The core role map has no dedicated danger/error/disabled/focus roles. If the
theme defines them, use them; otherwise use the fallback below. Do not hard-code
colours and do not invent keys in widget code — add them to `themes.hdf` and
`utils/theme.h` first, with a fallback chain for partial themes.

| Role | Use | Fallback if undefined |
|------|-----|-----------------------|
| `danger` / `danger_fg` | Destructive button surface/text | `button_bg` / `button_fg` + warning icon |
| `error` / `error_fg` | Inline/error text | `fg_norm` + warning glyph |
| `disabled_bg` / `disabled_fg` | Disabled controls | Avoid disabling; validate on submit instead |
| `focus_ring` | Focus outline | `accent` |

### 5.2 Widget mapping

Use only widgets that exist in this toolkit:

| Need | Widget / mechanism |
|------|--------------------|
| Dialog/sheet frame | `transientShellWidgetClass` popup shell with `XtNtransientFor`; use `XtGrabExclusive` when the sheet must block the app (Xt has no per-window grab). Fall back to `topLevelShellWidgetClass` only if the toolkit cannot create a transient shell. |
| Form/action layout | `Gridbox`, `VBox` |
| Buttons | `Wbutton` |
| Text field | `Wedit` |
| Label | `Wlabel` |
| Checkbox/toggle | `Wcheckbox` |
| List/file chooser | `WListView`, `WfileChooser`, `WfileSelector` |
| Menu button | `WmenuButton` + `WmenuPopup` |
| Tabs | `WtabWidget` |
| Scrollable body | `Wviewport` |
| Theme metrics/colours | `font_ui`, `control_*`, `window_margin`, `group_gap`, `action_gap`, `accent`, `input_*`, `button_*`, `list_*` |

If a needed control does not exist, compose it from these widgets or state the
gap; do not reference fictional APIs.

---

## 6. Keyboard and focus

| Key | Behaviour |
|-----|-----------|
| `Return` / `Enter` | Default action (primary, or Cancel in destructive confirmations). In a multiline editor: insert newline; use Ctrl+Enter for the primary. |
| `Escape` | Cancel/close without applying. If there are unsaved changes, confirm discard in place or with a follow-up alert. |
| `Tab` / `Shift+Tab` | Move through controls in reading order; modal traps focus, modeless does not. |
| `Space` | Activate the focused button/checkbox. |
| Arrow keys | Move within lists, radio groups and combo boxes; do not use them to move focus between unrelated controls. |
| Mnemonics | Optional, only if the toolkit supports them; unique per dialog; show in the label. |

Focus rules:

- Initial focus: first editable field, else the content list, else the first safe
  action. Never a destructive action.
- On open, select the prefill value where that speeds up the task (rename: select
  the basename, not the extension).
- On validation failure, focus the first invalid field and keep the caret/selection.
- On close, restore focus to the invoker.
- The focus ring must be visible on both light and dark themes.
- Do not steal focus from a modeless panel while the user edits the main window.

---

## 7. Validation and error messages

### 7.1 When to validate

- Validate on submit (primary click or Return).
- After the first submit, re-validate on change/blur so the user sees the error
  clear as they fix it.
- Do not validate on every keystroke before the first submit; it punishes typing.
- Async validation (name availability, server checks): show a small busy state,
  keep the input editable, and resolve the error inline.

### 7.2 Where to show errors

- **Field error:** directly below the field, `XS` gap, `font_small`.
- **Cross-field error:** attach it to the group or the second field; explain the
  relationship ("End date must be after start date.").
- **Dialog-level/blocking error:** at the top of the body, above the form, with a
  recovery action. Use this only for system failures, not for a single bad field.
- **Never** put field errors in a distant status bar or a modal-on-modal.

### 7.3 Message format

A good error says **what is wrong** and **how to fix it**, in sentence case, with
no codes, no blame and no "Error:" prefix.

| Bad | Good |
|-----|------|
| Invalid input (E42) | Enter a name for the file. |
| Error! | The name can’t contain “/”. Choose another name. |
| Duplicate name. | A file named “report.txt” already exists. Choose a different name. |
| Permission denied. | This folder is read-only. Choose another location. |
| Network error 503. | Couldn’t connect to the server. Check your network and try again. |
| You did something wrong. | The end date must be after the start date. |

Rules:

- Keep the message to one or two lines; wrap rather than truncate.
- Do not clear the field or reset the form.
- Do not disable the primary with no explanation; allow the click and show the
  error, or show the reason next to the disabled control.
- If there are several errors in a long form, add a short summary at the top with
  links/focus targets, plus inline errors at each field.
- Technical details (paths, error codes) belong in an expandable "Details"
  section, not in the main message.

---

## 8. Microcopy

Tone: direct, calm, specific, second person, present tense. No humour in errors,
no exclamation marks, no "Oops", no jargon, no blame.

| Element | Rule | Good | Avoid |
|---------|------|------|-------|
| Title | Task or question, Title Case, no period, no app name | Rename “Report.txt” | Rename File Dialog |
| Body | Why before controls, sentence case, 1–2 sentences | This will delete the file and its 2 versions. This can’t be undone. | Are you sure? |
| Primary | Verb + object, Title Case | Delete Project | OK |
| Secondary | Cancel or Close | Cancel | No |
| Destructive | Explicit object, danger style | Delete 3 Items | Yes |
| Further input | Trailing ellipsis | Choose… | Choose |
| Field label | Sentence case noun/phrase | File name | FILE NAME: |
| Placeholder | Hint, never the label | e.g. report.txt | Name |
| Inline error | What + fix, sentence case | Enter a name. | Invalid input |
| Busy | What is happening | Saving… | Please wait… |
| Success | Result + next step, only if not visible elsewhere | Saved to Documents. | Done! |
| Count | Numerals, correct plural | 3 items / 1 item | item(s) |

Additional rules:

- Use curly quotes around user content: “Report.txt”.
- Use "…" (U+2026), not "...".
- Do not put keyboard shortcuts in button labels; show them in menus/tooltips.
- Do not use "OK" as the primary when a verb exists ("Save", "Replace", "Delete").
- Do not use "Yes/No" buttons; use the action verb.
- "Cancel" abandons changes; "Close" dismisses information/modeless panels.
- For irreversible actions, name the object and the consequence; for reversible
  actions, prefer Undo over confirmation.
- Localisation: use full sentences with placeholders, never concatenate fragments.

---

## 9. Patterns

### 9.1 Destructive confirmation (alert)

Use only for destructive, irreversible or expensive actions. Reversible actions
get Undo instead.

- Title: question naming the object — "Delete “Q3 Report”?"
- Body: consequence + irreversibility — "This permanently deletes the file and
  its 2 versions. This can’t be undone."
- Buttons: `Cancel` (default, left) + `Delete` (destructive, right, danger style).
- Keyboard: Return/Escape = Cancel. The destructive action requires an explicit
  click or focus + Space.
- Focus: Cancel.
- Multi-item: name up to three items, otherwise show the count and "including
  “X”"; never list an unbounded set.
- Do not add a "Don’t ask again" option for destructive actions.

### 9.2 Input dialog (rename, new folder, new name)

- Title: "Rename “Report.txt”" / "New Folder".
- Body: usually omitted; add one sentence only if the rule is non-obvious.
- Field: prefill the current value; select the basename, not the extension, for
  rename.
- Validation: empty, invalid characters, duplicates, length; inline and on submit.
- Buttons: Cancel + Rename/Create.
- Keyboard: Return submits when valid; Escape cancels.
- On error: keep the dialog open, keep the value, focus/select the invalid part.
- Prefer inline rename in a list if the toolkit supports it.

### 9.3 Open / Save (file chooser sheet)

Use the file-chooser recipe from `style-guide.md` §10 inside a sheet.

- Toolbar: navigation (`Up`, `Home`) leading; current path; filter/search and view
  toggle trailing.
- Content: `WListView` with Name/Size/Modified/Type, folders first, fixed header,
  alternating rows, selection highlight, folder/file icons.
- Action bar: Cancel + `Open` / `Save`. The invoker that opens the dialog is
  labelled `Open…`; the dialog's primary completes the task and therefore has
  no ellipsis (D23).
- Open: focus the list; Return and double-click open; Escape cancels.
- Save: filename field at the top of the action area, focused by default, basename
  selected, extension visible and selectable for replacement.
- Overwrite: confirm inline in the same dialog (replace the body or use the
  status area); never stack an alert (D25) and never silently overwrite.
- Put locations/shortcuts in a sidebar when the dialog is large enough; hide it in
  compact sizes instead of shrinking the list to an unusable width.
- Preserve the last location and the selection across opens where appropriate.

### 9.4 Progress / long-running work

- Start immediately; do not wait for a second confirmation unless the action is
  destructive.
- Show a determinate bar when progress is known; otherwise an indeterminate
  indicator plus a status verb ("Exporting 42 of 100 pages…").
- Keep the UI responsive; never block the event loop.
- Cancel: offer it if cancellation is safe. If not, say so ("This can’t be
  cancelled.") and do not show a dead Cancel button.
- On success: close and show the result inline/status, or keep the dialog open
  with a success state if the user needs the output.
- On failure: keep the dialog open, show the error and a recovery action
  (`Try Again` primary, `Cancel` secondary). Preserve the settings.
- If the user closes during work: confirm, or continue in the background and
  report completion; state which behaviour applies.

### 9.5 Preferences / settings

- Choose **one** model and stick to it:
  - **Immediate apply:** no OK/Cancel; `Close`; changes apply instantly; provide
    `Reset to Defaults`.
  - **Staged apply:** `Cancel` + `OK` (and optional `Apply`); changes commit on OK
    and revert on Cancel.
- Do not mix immediate and staged behaviour in one panel.
- Group by task, not by widget type; use tabs/sections for large panels.
- Labels are visible; defaults are clear; destructive resets require confirmation.
- If closing with unsaved staged changes, confirm discard.

### 9.6 Error / recovery alert

- Title: what failed — "Couldn’t save “Report.txt”".
- Body: reason + next step — "The disk is full. Free some space, then try again."
- Buttons: Cancel + recovery (`Try Again` primary, or `Open Settings`).
- Technical details in an expandable "Details" section.
- Do not use alerts for field validation; keep that inline.
- Return triggers the recovery action when it is non-destructive; Escape cancels.

### 9.7 Wizard (only when truly multi-step)

- Use for 2–4 dependent steps; otherwise use a single form.
- Buttons: `Back` (secondary), `Next`/`Finish` (primary), `Cancel` always.
- Validate per step; do not advance with an invalid step.
- Preserve data when going back; show a step indicator.
- No branching complexity unless the user explicitly asks for it.
- Finish is the only primary on the last step; no dead ends.

### 9.8 Modeless inspector / panel

- No OK/Cancel; changes apply immediately; `Close` dismisses.
- Follows the current selection without stealing focus.
- Minimum width (e.g. 240 px); remember size/position.
- Edits are undoable in the main document.
- Do not block the main window.

---

## 10. Sizing and adaptivity

- Default widths: alert 360, form 420, list 520; stay ≤ 80% of the parent.
- Minimum size: keep the title and action bar fully visible; no clipped buttons.
- Height: content-driven; scroll the body above ~70% of the parent height.
- Keep width stable across states; allow height to grow for errors/help.
- Center modals on the parent; attach sheets to the parent's top edge.
- Clamp to the screen work area on multi-monitor setups.
- Narrow layout: stack labels above fields, hide optional help, keep the primary
  action visible; do not shrink the list below a usable width.
- Long strings: wrap in body text; truncate with an ellipsis in list cells and
  expose the full value via tooltip/inspector.
- Do not resize the dialog per state or per validation message.

---

## 11. Theme, contrast and polish

- Surfaces: `bg_norm` / `gridbox_bg`; controls: `button_*`; inputs: `input_*`;
  content: `list_*`; borders: `input_border` (colour) / `control_border` (1 px
  width) / `list_line`; accent: `accent` /
  `accent_fg`.
- No literal colours anywhere. No hard-coded white inputs or black text.
- Every new role goes into **all** themes in `themes.hdf` with a fallback chain.
- Contrast targets: body text ≥ 4.5:1; large text ≥ 3:1; non-text UI boundaries,
  focus rings and icons ≥ 3:1.
- Verify light and dark themes; selection text must contrast with the selection
  surface.
- Icons: monochrome, `icon_size` (16), flat silhouettes that read on both themes.
- Avoid disabling controls unless the reason is obvious; if disabled roles exist,
  keep the label readable and explain why.
- Flat style: no bevels, no rounded corners on inputs/buttons, 1 px borders.
- Keep hit targets ≥ `control_height` (24 px) and do not overlap hover/focus
  effects.

---

## 12. Anti-patterns

| Don’t | Do |
|-------|----|
| "OK / Cancel" for everything | Verb labels: "Save", "Replace", "Delete" |
| Destructive action is the default/accent | Safe default; destructive is explicit and danger-styled |
| "Are you sure?" with no object | Name the object and consequence |
| Placeholder used as the label | Visible label + optional placeholder hint |
| Disabled primary with no reason | Allow the click and show an inline error |
| Clearing the form on error | Preserve input and focus the invalid field |
| Red error text hard-coded | Theme error role or `fg_norm` + warning glyph |
| Modal stacked on modal | Sheet on the same parent, or replace the body |
| Dialog for a success message already visible | Inline/status confirmation |
| Dialog for reversible actions | Inline edit + Undo |
| Giant dialog with everything | Sections, progressive disclosure, scroll body |
| Tiny 20 px buttons | `control_height` (≥ 24 px) |
| Icon-only mystery buttons | Label or tooltip; keep the icon box fixed |
| Return triggers Delete | Return = Cancel/safe action in destructive confirmations |
| Vague "Invalid input" | Specific what + fix |
| Title repeats the app name | Task-specific title |
| Multiple accent buttons | One primary; everything else secondary |
| Auto-close that hides the result | Show success/result before closing |
| Field errors in the status bar | Inline below the field |
| Dialog width jumps on error | Stable width, vertical growth/scroll |

---

## 13. Self-check (run before returning code)

If any `MUST` fails, fix it before answering. Report remaining assumptions.

**Structure**
- [ ] D1 one primary; D2 Cancel/Close + Escape; D3 safe default; D4 destructive
      not default/accent
- [ ] D5 title is the task, no app name; D6 body gives the why
- [ ] D11 fits the smallest window; D12 width stable across states
- [ ] The sheet is transient for its parent and blocks it (§2, §5.2)
- [ ] No stacked dialogs (§2.2)

**Actions and keyboard**
- [ ] D13 Return/Escape; multiline editor exception handled
- [ ] D14 reading-order tab chain; modal traps focus
- [ ] D15 safe initial focus; D16 focus restored on close; D17 visible focus
- [ ] D23 verb labels, Title Case, "…" only for further input
- [ ] D22 default/invalid/busy/error/success all designed; no dead ends

**Content**
- [ ] D7 visible labels; D18 inline validation; D19 input preserved
- [ ] D20 disable only with an obvious reason
- [ ] D24 sentence case prose, Title Case titles/buttons
- [ ] D25 no nested dialogs
- [ ] D21 busy state explicit and responsive

**Theme and layout**
- [ ] D8 all colours from theme roles; no literals
- [ ] D9 spacing from the scale; D10 `font_ui` for controls
- [ ] Light and dark themes checked; contrast targets met
- [ ] Hit targets ≥ `control_height`; no clipped/wrapped buttons
- [ ] Destructive/danger/error styling uses a defined role or documented fallback

**Microcopy**
- [ ] Title/body/buttons/errors follow §8
- [ ] Destructive confirmation names the object and consequence
- [ ] Counts/plurals correct; no "item(s)"; curly quotes; single "…"

---

## 14. Output contract for the LLM

Before writing widget code, emit a **Dialog Spec**. Then emit the **widget tree**
and the **state matrix**. Use only the widgets and theme roles listed in §5. If
information is missing, use the §1.2 defaults and list assumptions.

```
DIALOG SPEC
type:            sheet | modal | alert | modeless
scope:           app | window | document
trigger:         ...
title:           ...
body:            ...
fields:
  - label:       ...
    widget:      Wedit | Wcheckbox | WListView | ...
    value:       ...
    validation:  ...
primary:         ...
secondary:       ...
destructive:     yes/no — object, consequence, reversible?
default_action:  ...
cancel_action:   ...
initial_focus:   ...
states:
  default:       ...
  invalid:       ...
  busy:          ...
  error:         ...
  success:       ...
size:            width, min height, scroll behaviour
assumptions:     ...

WIDGET TREE
Gridbox (window_margin 16, gap group_gap)
  Wlabel "Title" (font_heading2)
  VBox (gap S)
    Wlabel "Field label"
    Wedit (input_bg/input_fg, control_height, 1px input_border)
    Wlabel error (font_small, hidden)
  Gridbox action row (right, action_gap 8)
    Wbutton "Cancel" (button_bg/button_fg)
    Wbutton "Primary" (accent/accent_fg)
```

Rules for the output:

- Include every user-visible string; do not leave "TODO" placeholders.
- Annotate each control with its theme roles and metrics.
- Include the hidden/invalid/busy/error/success states, not just the happy path.
- State the keyboard behaviour (Return, Escape, focus, tab order).
- State what happens on cancel, close, success and failure.
- Do not invent widgets, theme keys or APIs.
- If a requirement conflicts with a `MUST`, follow the `MUST` and explain the
  conflict in the assumptions.

---

## 15. Worked examples

### 15.1 Rename file (sheet)

```
Title: Rename “Report.txt”
Body: (omitted — the task is obvious)
Field: Name [Report.txt]  (basename selected)
Primary: Rename
Secondary: Cancel
Validation: empty, invalid characters, duplicate name
Keyboard: Return = Rename (when valid), Escape = Cancel
Initial focus: Name field
Size: 420 px wide
```

```
Gridbox (window_margin 16, group_gap 12)
  Wlabel "Rename “Report.txt”" (font_heading2)
  VBox (gap 8)
    Wlabel "Name"
    Wedit (input_bg/input_fg, control_height, 1px input_border)
    Wlabel "A file named “Report.txt” already exists. Choose a different name."
      (font_small, hidden until invalid)
  Gridbox action row (right, action_gap 8)
    Wbutton "Cancel" (button_bg/button_fg)
    Wbutton "Rename" (accent/accent_fg)
```

Error state: keep “Report.txt” in the field, focus/select the basename, show the
duplicate message below the field, keep the dialog open.

### 15.2 Delete confirmation (alert)

```
Title: Delete “Q3 Report”?
Body: This permanently deletes the file and its 2 versions. This can’t be undone.
Buttons: Cancel (default) | Delete (destructive, danger style)
Keyboard: Return = Cancel, Escape = Cancel; Delete requires an explicit click
Initial focus: Cancel
Size: 360 px wide
```

```
Gridbox (window_margin 16, group_gap 12)
  Wlabel "Delete “Q3 Report”?" (font_heading2)
  Wlabel "This permanently deletes the file and its 2 versions. This can’t be undone."
    (font_ui, wraps)
  Gridbox action row (right, action_gap 8)
    Wbutton "Cancel" (button_bg/button_fg, default)
    Wbutton "Delete" (danger/danger_fg or button_* + warning icon)
```

### 15.3 Open file (sheet)

```
Toolbar: [Up] [Home]  Documents / Projects        [Search] [View]
Content: WListView — Name | Size | Modified | Type
         folders first, fixed header, alternating rows, selection highlight
Action bar: Cancel | Open
Focus: list
Keyboard: Return/double-click = Open, Escape = Cancel, arrows move selection
Size: 520 px wide, body scrolls under the header
```

The button that opens this sheet is labelled `Open…`; the sheet's primary is
`Open` because it completes the task (D23).

Empty state: "This folder is empty." plus a hint, not a blank list.

### 15.4 Save failed (alert)

```
Title: Couldn’t save “Report.txt”
Body: The disk is full. Free some space, then try again.
Buttons: Cancel | Try Again (primary)
Details: expandable path/error code
Keyboard: Return = Try Again, Escape = Cancel
Initial focus: Try Again
Size: 360 px wide
```

Keep the user’s edits and the dialog state; do not close on failure.

### 15.5 Export progress (modal or modeless)

```
Title: Exporting “Project”
Body: 42 of 100 pages…
Progress: determinate bar (accent fill on bg_hi track, 8 px)
Buttons: Cancel (secondary) — only if cancellation is safe
Keyboard: Escape = Cancel (if cancellable)
Success: close and show “Exported to Documents.” inline/status
Failure: keep open; title “Export failed”; body reason; Try Again | Cancel
```

If the user can continue working, use a modeless progress panel instead of a
modal.

---

## 16. Prompt block (paste with this guide)

```
You are implementing dialogs for the XTCW desktop GUI.

Read dialog-guide.md and style-guide.md. Follow every MUST rule. Use only the
toolkit widgets and theme roles listed in the guides. Before writing code, emit a
Dialog Spec (dialog-guide.md §14). If information is missing, use the documented
defaults and list assumptions. After writing code, run the self-check in §13 and
report any MUST failures. Prefer sheets over modals, inline editing over dialogs,
and one accent primary per dialog. Never make a destructive action the default.
```

---

## 17. Reference

- `style-guide.md` §1–§11 — principles, spacing, metrics, typography, colour
  roles, dialog anatomy, widget recipes, theming.
- `style-guide.md` §10 — dialog types, open/save, checklist.
- `style-guide.md` §11 — main-window anatomy, adaptivity, menus, focus.
- `themes.hdf` — the actual values for every role/metric named here.
- `utils/theme.h` — theme query API (`theme_int_defined`, `theme_get_int`,
  `colors_changed`).
- `wbuild_widgets/CONVENTIONS.md` — widget syntax, geometry, theming.
- Canonical implementations: `WfileChooser` + `WfileSelector` (Open sheet
  layout, toolbar/list/status/action bar, focus and Escape), `WListView` (list
  surface), `Wbravo` (editor surface). The Save flow is specified above but is
  not implemented yet; do not treat the Open sheet as covering it.
