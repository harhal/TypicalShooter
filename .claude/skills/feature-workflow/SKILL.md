---
name: feature-workflow
description: Develop a TypicalShooter feature on its own branch in small, reviewable iterations - describe the feature and its requirements first, create a feature branch, implement it in small test-first commits split into small code units, keep the requirements updated as they change, and merge into main when done. Use when the user starts, continues or finishes work on a feature ("new feature", "start feature X", "next iteration", "finish/merge the feature"), or types /feature-workflow.
argument-hint: "[start <feature> | next | status | finish]"
---

# Feature workflow

One feature = one branch = one living doc in `Docs/Features/`. Work moves in small iterations;
each iteration is one focused commit that builds and passes the tests. Talk to the user in Russian;
code, identifiers, branch names and commit messages in English (see CLAUDE.md).

Pick the phase from the argument or from the state: on `main` with no feature in progress → **Start**;
on a `feature/*` branch → **Iterate** (or **Finish** when the plan is done / the user asks).
`status` → summarise the feature doc's plan (done / next / open questions) and `git log main..HEAD --oneline`.

## Phase 1 — Start: describe, then branch

1. **Clarify the feature with the user before any code.** Ask only what changes the design
   (goal for the player, controls, scope edges, multiplayer expectations, what is explicitly out of scope).
   Read the related feature docs in `Docs/Features/` and only the code they point to.
2. **Branch** from an up-to-date `main` (working tree clean; if not, ask what to do with the changes):
   ```powershell
   git switch main; git pull --ff-only; git switch -c feature/<kebab-name>
   ```
   (`git pull` only if a remote exists.) If the feature extends an existing doc, branch the same way.
3. **Write the feature doc** `Docs/Features/<Name>.md` from `_Template.md` (or extend the existing doc),
   and add/adjust its row in `Docs/Features/README.md`. Add two sections after "Что это":
   ```markdown
   ## Требования
   - R1. <testable statement> 
   - R2. ...
   **Вне рамок:** ...
   **Открытые вопросы:** ...

   ## План итераций
   - [ ] 1. <small step> — тест: `TypicalShooter.<Area>.<Case>`
   - [ ] 2. ...
   ```
   Each requirement must be checkable (by an automation test or a described manual check).
   Each iteration should be roughly ≤ 150–200 changed lines of code, ideally touching 1–3 files,
   and leave the project compiling with all tests green.
4. Show the requirements + plan to the user and get agreement, then commit:
   `Describe <feature>: requirements and iteration plan`.

## Phase 2 — Iterate: one small step at a time

For each unchecked plan item:

1. **Test first** (CLAUDE.md "Testing"): write the failing automation test, run it, confirm it fails
   for the expected reason. Purely content/Blueprint steps get a manual check instead, written in the doc.
2. **Implement the minimum** to pass. Keep code split into small units so later reads are cheap:
   - one class / one responsibility per file pair; prefer a new focused file over growing a large one
     (aim for files under ~300–400 lines; if a file you touch is already large, put new logic in a
     helper/separate file rather than appending);
   - short functions (≈ ≤ 40 lines), named helpers instead of long inline blocks;
   - tests in their own file per area (`Private/Tests/<Area>Tests.cpp`), shared setup in fixtures;
   - no speculative code for later iterations.
3. **Verify**: build (or Live Coding), run the whole `TypicalShooter.` + `HarhalHexagonalGrid.` suites.
   Never weaken a failing test.
4. **Update the doc in the same commit**: tick the plan item, adjust "Как устроено" / "Тесты" /
   "Сверено с кодом" date. If something learned during the step changes the requirements or the plan
   (new constraint, split step, dropped scope), edit "Требования" / "План итераций" now and tell the user
   what changed and why. Requirement changes that alter scope or behaviour need the user's OK first.
5. **Commit** the iteration alone (`git add` specific paths; check `git status` for stray assets):
   imperative message describing the step, e.g. `Inventory: reject transfers to out-of-bounds cells`.
   Plugin changes follow the submodule two-commit rule in CLAUDE.md.
6. Report briefly (what was done, test result, next step) and continue with the next item unless the
   user asked to pause after each iteration.

Keep each diff readable on its own: no drive-by reformatting, no mixing refactors with behaviour changes
(a needed refactor is its own iteration).

## Phase 3 — Finish: merge into main

1. All plan items ticked (or moved to "Открытые задачи" with the user's agreement); requirements reflect
   what was actually built; status in the doc and README table updated; full test suites green.
   Update CLAUDE.md "Current state" if the feature changes it.
2. Show the user a summary: `git log main..HEAD --oneline` and `git diff main...HEAD --stat`.
   **Ask for confirmation before merging and before any push.**
3. Merge, keeping the iteration history:
   - with a remote (default here — `origin` on GitHub): push the branch, open a PR
     (`gh pr create --base main`, body = requirements summary + iteration list), then on the user's OK
     `gh pr merge --merge --delete-branch` and `git switch main; git pull --ff-only`;
   - local only: `git switch main; git merge --no-ff feature/<name> -m "Merge feature/<name>: <summary>"`,
     then `git branch -d feature/<name>`.
   If the plugin submodule changed, make sure its commits are pushed before pushing main.
