# Test Plan — `msh`

| Input | Expected behaviour | Actual result |
| :--- | :--- | :--- |
| *(blank line)* | New prompt, no error | Prompt `msh[0]>` reprinted immediately with no error. |
| `echo hello world` | Prints text; status 0 | Printed `hello world`; next prompt displayed `msh[0]>`. |
| `ls /nonexistent` | Error from `ls`; non-zero status | Printed `ls: cannot access '/nonexistent': No such file or directory`; prompt displayed `msh[2]>`. |
| `nosuchcommand` | Error message; status 127 | Printed `nosuchcommand: No such file or directory`; prompt displayed `msh[127]>`. |
| `cd /tmp` then `pwd` | Prints `/tmp` | `cd /tmp` succeeded (`msh[0]>`); `pwd` printed `/tmp`. |
| `sleep 3 &` | Prompt returns at once; PID shown | Printed `[bg] PID 1871` and returned `msh[0]>` immediately; reaped cleanly with `[bg] PID 1871 finished`. |
| `echo hi > f.txt` then `cat f.txt` | File contains `hi` | `f.txt` created with mode `0644`; `cat f.txt` printed `hi`. |
| `sleep 100` then `Ctrl-C` | Sleep stops; shell survives; status 130 | `sleep 100` terminated on `SIGINT`; `msh` remained active and displayed `msh[130]>`. |
| `Ctrl-D` at the prompt | Shell exits cleanly | Printed newline and exited cleanly without memory leaks. |
| `cd` (with no arguments) | Changes directory to `$HOME` | Changed working directory to `/home/mueni`; prompt displayed `msh[0]>`. |
| `cd /does_not_exist` | Prints error; status 1 | Printed `cd: No such file or directory`; prompt displayed `msh[1]>`. |
| `help` | Displays built-in commands; status 0 | Printed `cd`, `help`, and `exit` descriptions; prompt displayed `msh[0]>`. |
