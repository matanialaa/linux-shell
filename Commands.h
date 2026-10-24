#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
protected:
    char* command;
    char* cmd_line;
    char** arg;
    int arg_num;
 public:
    explicit Command(const char *cmd_line);
    virtual ~Command() = default;
    virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed:
     char* get_cmd_line(){
      return cmd_line;
     }
};

class BuiltInCommand : public Command {
 public:
  explicit BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {
      delete this->cmd_line;
  }
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line): Command(cmd_line){}
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
  char first_command[COMMAND_ARGS_MAX_LENGTH];
    char second_command[COMMAND_ARGS_MAX_LENGTH];
    bool no_ampersand;
 public:
  explicit PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 char command[COMMAND_ARGS_MAX_LENGTH];
 char file_name[COMMAND_ARGS_MAX_LENGTH];
 char override;

 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
 // bool prepare() override;
  //void cleanup() override;
};

class ChpromptCommand : public BuiltInCommand {
public:
    explicit ChpromptCommand(const char *cmd_line):BuiltInCommand(cmd_line){};

    virtual  ~ChpromptCommand() = default;

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:

  ChangeDirCommand(const char* cmd_line): BuiltInCommand(cmd_line) {}
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;

class JobsList {
 public:
  class JobEntry {
      // TODO: Add your data members
  public:
      pid_t pid; // procces id
      int job_id;
      char *cmd_line;
      time_t time_created;
      bool stopped_mark;

      JobEntry(pid_t pid, int job_id, char *cmd_line, time_t time_created, bool stopped_mark = false);
      ~JobEntry() = default;

      //
      int SendSignal(int signal,JobsList* job_list);
      void unstop_command();
  };
 // TODO: Add your data members
 std::vector<JobEntry*> jobs_list;
 int max_job_id;
  JobsList();
  ~JobsList() = default;
  void addJob(Command* cmd,  pid_t pid,bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){}
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){}
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){}
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
    JobsList* jobs;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){}
    virtual ~QuitCommand() {}
    void execute() override;
};


class TimeoutCommand : public BuiltInCommand {
/* Optional */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class FareCommand : public BuiltInCommand {
  /* Optional */
  // TODO: Add your data members

 public:
  FareCommand(const char* cmd_line): BuiltInCommand(cmd_line){}
  virtual ~FareCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  /* Optional */
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
  /* Bonus */
 // TODO: Add your data members
 JobsList* jobs;
public:
    KillCommand(const char* cmd_line, JobsList* jobs): BuiltInCommand(cmd_line), jobs(jobs){}
    void execute() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  SmallShell();
 public:
    std::string shell_prompt;
    pid_t shell_pid;
    pid_t curr_proccess_pid;
    char  fg_cmd_line[COMMAND_ARGS_MAX_LENGTH];
    char* last_dir;
    JobsList* jobs;
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)    = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
