## CREATING COMMIT PLANS ##

### Core Principles
- A commit MUST NOT include all changed files.
- Each commit MUST represent a single, meaningful sub-feature, fix, or change.
- Commits should be small, focused, and logically grouped.
- Make sure there is no staged or unstaged changes on the source control after the commits
---

### 1. Commit Scope Rules
- Each commit should address ONLY ONE of the following:
  - A sub-feature implementation
  - A bug fix
  - A refactor (no behavior change)
  - A test addition/update
  - A documentation update
- DO NOT mix multiple concerns in one commit (e.g., feature + refactor).

---

### 2. File Selection Rules
- Only include files directly related to the sub-feature.
- DO NOT include unrelated modified files.
- If multiple files are required for the same sub-feature → include them together.
- If a file contains changes for multiple purposes → split into multiple commits if possible.

---

### 3. Commit Size Guidelines
- Keep commits small enough to:
  - Be easily reviewed
  - Be easily reverted
- Avoid:
  - Massive commits
  - “dump” commits (e.g., “update everything”)

---

### 4. Commit Ordering (IMPORTANT)
Commits should follow logical development order:
1. Setup / scaffolding
2. Core implementation
3. Supporting logic
4. Tests
5. Cleanup / refactor

---

### 5. Commit Message Format
Use clear, consistent messages:
5