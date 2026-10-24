[README.md](https://github.com/user-attachments/files/33906827/README.md)
# smash – A Small Linux Shell in C++

**A Unix-style command shell supporting built-in commands, external programs, pipes, redirection, and job control.**

## Features

**Built-in commands**

| Command | Description |
|---|---|
| `chprompt [name]` | Change the shell prompt |
| `showpid` | Print the shell's process ID |
| `pwd` | Print the current directory |
| `cd <path>` / `cd -` | Change directory, or return to the previous one |
| `jobs` | List background and stopped jobs |
| `fg [job-id]` | Bring a job to the foreground |
| `bg [job-id]` | Resume a stopped job in the background |
| `kill -<signal> <job-id>` | Send a signal to a job |
| `quit [kill]` | Exit the shell, optionally killing all jobs |
| `chmod <mode> <path>` | Change file permissions |
| `getfiletype <path>` | Show a file's type and size |
| `setcore <job-id> <core>` | Pin a job to a CPU core |

**Process management**
- Runs external programs with `fork` and `execvp`, including complex commands through `bash`
- Runs commands in the background with `&` and tracks them in a jobs list
- Removes finished jobs automatically

**Pipes and redirection**
- `cmd1 | cmd2` pipes standard output
- `cmd1 |& cmd2` pipes standard error
- `cmd > file` overwrites and `cmd >> file` appends

**Signal handling**
- **Ctrl-C** kills the foreground process
- **Ctrl-Z** stops the foreground process and adds it to the jobs list

## Tech Stack

C++ · Linux system calls (`fork`, `execvp`, `waitpid`, `pipe`, `dup2`, `kill`, `sched_setaffinity`, `chmod`, `stat`)

## Design

- `SmallShell` is a **singleton** that parses each line and creates the matching `Command` object.
- Each command type extends an abstract `Command` class, with separate subclasses for built-in commands, external commands, pipes, and redirection.
- `JobsList` stores each job's ID, process ID, command line, start time, and stopped status.

## Running

```bash
make
./smash
```

Example session:

```
smash> sleep 100 &
smash> jobs
[1] sleep 100 & : 12345 3 secs
smash> fg 1
sleep 100 & 12345
^Zsmash: got ctrl-Z
smash: process 12345 was stopped
smash> bg 1
```

## About

A course project for Operating Systems (234123) at the Technion, built by **Alaa Matani** and **Atheer Shannan**. The project skeleton was provided by the course staff.

> If you're currently taking this course, please follow your course's academic integrity policy.
