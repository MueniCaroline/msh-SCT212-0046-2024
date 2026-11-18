# msh — Minimal Unix Command-Line Shell

Name: Caroline Mueni  
Registration Number: SCT212-0046/2024  

## How to Build and Run

    make
    ./msh

To remove compiled binaries:

    make clean

## Features Implemented

- [x] **Task 1 — Prompt & Input Reading:** Complete (`msh[status]>` prompt, `getline()`, clean exit on `Ctrl-D`)
- [x] **Task 2 — Tokenization:** Complete (splits whitespace into `NULL`-terminated `argv`, skips blank lines)
- [x] **Task 3 — External Commands:** Complete (`fork()`, `execvp()`, `waitpid()`, returns status `127` if not found)
- [x] **Task 4 — Built-in Commands:** Complete (`cd [dir]`, `help`, `exit`)
- [x] **Task 5 — Exit Status in Prompt:** Complete (tracks `WEXITSTATUS` and `128 + sig` for signal exits)
- [x] **Task 6 — Background Execution:** Complete (`&` support, prints PID immediately, reaps zombies with `WNOHANG`)
- [x] **Task 7 — Output Redirection:** Complete (child-only `>` using `open(..., 0644)`, `dup2()`, and `close()`)
- [x] **Task 8 — Signal Handling:** Complete (`SIGINT` / `Ctrl-C` interrupts foreground child without closing `msh`)

## Design Explanation

For external commands, `msh` follows the standard `fork()`, `execvp()`, and `waitpid()` cycle. After parsing the input line into `argv`, the parent shell calls `fork()` to spawn a child process. Inside the child (`pid == 0`), `msh` resets `SIGINT` back to `SIG_DFL` so `Ctrl-C` can stop the running program, and if `>` was given, opens the target file (`0644`) and redirects `STDOUT_FILENO` via `dup2()`. Doing redirection inside the child ensures the parent shell's own standard output stays untouched. The child then calls `execvp()` to load the requested program from `PATH`, calling `_exit(127)` if `execvp()` fails so a failed child never drops back into the shell loop. For foreground jobs, the parent waits with `waitpid(pid, &status, 0)` and updates `last_status` using `WIFEXITED` or `WIFSIGNALED` (`128 + WTERMSIG`).

`cd` and `exit` have to be built-ins rather than external programs because child processes get their own isolated environment. If `cd` ran in a forked child, `chdir()` would only change the child's working directory and vanish as soon as the child exited, leaving `msh` in the old folder. Likewise, an external `exit` binary would only terminate itself instead of ending the shell's main loop.

To prevent zombie processes from building up after background commands (`&`) finish, `msh` prints the child's PID without blocking and runs a `waitpid(-1, &status, WNOHANG)` loop before every prompt and at shell exit to reap any finished children.

## Known Limitations

- Only handles single output redirection (`>`); input redirection (`<`), append (`>>`), and pipes (`|`) are not implemented.
- Arguments containing spaces inside quotes or escaped characters are split on whitespace rather than kept as one token.

## Sources and AI-Tool Declaration

- Linux man pages (`man 2 fork`, `man 3 execvp`, `man 2 waitpid`, `man 2 dup2`, `man 7 signal`).
- Consulted Gemini to double-check `WNOHANG` reaping logic, signal behavior across `fork()`, and `README.md` structure.
