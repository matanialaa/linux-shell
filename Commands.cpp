#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include "time.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "limits.h"
#include "../skeleton_smash/skeleton_smash/Commands.h"


using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

inline bool isInteger(const std::string & str)
{
    char * _long;
    if(str.empty() || ((!isdigit(str[0])) && (str[0] != '+') && (str[0] != '-'))) return false;
    strtol(str.c_str(), &_long, 10);
    if(!(*_long)) return true;
    return false;
}
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

Command::Command(const char *cmd_line) {
    this->cmd_line = new char[COMMAND_ARGS_MAX_LENGTH];
    strcpy(this->cmd_line,cmd_line);
    this->arg = new char*[COMMAND_MAX_ARGS];
    this->arg_num = _parseCommandLine(cmd_line, this->arg);
    if(this->arg_num >0){
        this->command = new char[COMMAND_ARGS_MAX_LENGTH];
        strcpy(this->command, arg[0]);
    }
}
/*
Command::~Command()
{
    for(int i=0 ; i<arg_num;i++)
    {
        if(arg[i])
            free(arg[i]);///cause in every arg[i] we have malloc
    }
    delete[] this->arg;
    delete[] this->cmd_line;
}
*/
BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line){


    //if the command given contains & (background) we should erase it
    if(_isBackgroundComamnd(cmd_line)) {
        _removeBackgroundSign(this->cmd_line);

            this->arg_num = _parseCommandLine(this->cmd_line, this->arg);

    }
}

SmallShell::SmallShell() :shell_prompt("smash") , shell_pid(getpid()), curr_proccess_pid(-1),last_dir(nullptr){///get pid or get ppid?
// TODO: add your implementation
this->jobs = new JobsList();

}

SmallShell::~SmallShell() {
// TODO: add your implementation
if(last_dir)
    delete last_dir;// we use free cause last_dir is null or arg[1] that is located by malloc

  delete this->jobs;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  if(cmd_s.find('>') != string::npos || cmd_s.find(">>")  != string::npos ){
      return new RedirectionCommand(cmd_line);
  }
    if(cmd_s.find('|') != string::npos  || cmd_s.find("|&") != string::npos ){
        return new PipeCommand(cmd_line);
    }
    if ((firstWord == "chprompt") || (firstWord == "chprompt&")) {
        return new ChpromptCommand(cmd_line);
  }else if(firstWord == "pwd" || (firstWord == "pwd&")) {
    return new GetCurrDirCommand(cmd_line);
  }else if(firstWord == "showpid" || (firstWord == "showpid&")) {
    return new ShowPidCommand(cmd_line);
  }else if( firstWord == "cd" || (firstWord == "cd&")) {
      return new ChangeDirCommand(cmd_line);
  }else if( firstWord == "jobs" || (firstWord == "jobs&")){
        return new JobsCommand(cmd_line, this->jobs);
  }else if( firstWord == "fg" || (firstWord == "fg&")){
        return new ForegroundCommand(cmd_line, this->jobs);
  }else if( firstWord == "bg" || (firstWord == "bg&")){
        return new BackgroundCommand(cmd_line, this->jobs);
  }else if( firstWord == "quit" || (firstWord == "quit&")){
        return new QuitCommand(cmd_line,this->jobs);
  }else if( firstWord == "fare" || (firstWord == "fare&")){
        return new FareCommand(cmd_line);
    }else if(firstWord == "kill" || (firstWord == "kill&")){
        return new KillCommand(cmd_line,this->jobs);
    }else {
    return new ExternalCommand(cmd_line);
  }
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
   Command* cmd = CreateCommand(cmd_line);
   if(cmd)
     cmd->execute();

  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

/*******************************-------JOB_LIST------***************************************************/

JobsList::JobEntry::JobEntry(pid_t pid, int job_id, char * cmd_line, time_t time_created,bool stopped_mark):pid(pid)
,job_id(job_id),cmd_line(cmd_line),time_created(time_created),stopped_mark(stopped_mark)
{}

JobsList::JobsList() :jobs_list() , max_job_id(1){}

void JobsList::addJob(Command *cmd,pid_t pid,bool isStopped) {
    removeFinishedJobs();
// we should check if it is duplicated
    for (JobEntry* job : this->jobs_list)
    {
        if(job->pid == pid)///not sure if this is right//////////
        {
            job->stopped_mark = isStopped;
            // we should reset the time as well cause the job is added again
            time(&job->time_created);
            return;
        }
    }
    this->jobs_list.push_back(new JobEntry(pid,this->max_job_id,cmd->get_cmd_line() ,time(nullptr),isStopped));
    this->max_job_id++;
}

void JobsList::printJobsList() {
    removeFinishedJobs();

    for (JobEntry* job : this->jobs_list)
    {
        time_t now;
        time(&now);
        if(now == (time_t)-1)
        {
            perror("smash error: time failed");
            return;
        }
        std::cout<<"["<<job->job_id<<"]"<<" "<<job->cmd_line<<" : "<<job->pid<<" "<<difftime(now,job->time_created)<<" secs";
        if(job->stopped_mark)
        {
            std::cout<<" (stopped)";
        }
        std::cout << std::endl;

    }

}

void JobsList::killAllJobs() {
    this->removeFinishedJobs();
    for (JobEntry* job : this->jobs_list){
        cout << job->pid << ": " << job->cmd_line << endl;
        if(kill(job->pid, SIGKILL) == -1)
        {

            perror("smash error: kill failed");
        }
    }

}

void JobsList::removeFinishedJobs() {

    if (this->jobs_list.empty()) {
        max_job_id = 1;
        return;
    }
    int init_status;
    for(std::vector<JobEntry*> :: iterator itr = this->jobs_list.begin(); itr != this->jobs_list.end(); ++itr ){
       int return_val = waitpid((*itr)->pid, &init_status, WNOHANG);
        if ( return_val == (*itr)->pid || return_val ==-1) {
          //  if WNOHANG was specified and one or more child(ren) specified by pid exist, but have not yet changed state, then 0 is returned
          //so if it returned value !=0 this means that the child has finished or an error accured so we need to erase it
            this->jobs_list.erase(itr);
            --itr;
        }
    }

   // now we gotta update the max_job_id cause it might change
    int max_id =0;
    for(JobEntry *job : this->jobs_list) {
        if(job->job_id > max_id)
            max_id = job->job_id;
    }
    this->max_job_id = max_id +1;

}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    for (JobEntry* job : this->jobs_list)
    {
        if(job->job_id == jobId)
            return job;
    }
    return nullptr;

}

void JobsList::removeJobById(int jobId) {
    for(std::vector<JobEntry*> :: iterator itr = this->jobs_list.begin(); itr != this->jobs_list.end(); ++itr )
    {
        if((*itr)->job_id==jobId)
        {
            this->jobs_list.erase(itr);
           break;
        }
    }
    if(jobId == (max_job_id-1))
    max_job_id--;
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
//we are saving the max id thet we have in the vector in max_job_id-1
*lastJobId = max_job_id-1;
return getJobById(max_job_id-1);

}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    if (this->jobs_list.empty())
    {
        *jobId =-1;
        return nullptr;
    }
    for (JobEntry* job : this->jobs_list) {
        if (job->stopped_mark) {
            *jobId = job->job_id;
        }
    }

    return getJobById(*jobId);
}


/*******************************-------BUILD_IN_COMMANDS------**********************************************************/
void ChpromptCommand::execute() {

    SmallShell &shell = SmallShell::getInstance();
    if(this->arg_num==1){
        shell.shell_prompt = "smash";
    }
    else{
        shell.shell_prompt = this->arg[1];
    }
}

void ShowPidCommand::execute() {
    SmallShell &shell = SmallShell::getInstance();
    std::cout << "smash pid is " << shell.shell_pid << std::endl;

}

void GetCurrDirCommand::execute() {
    char *buffer = new char[PATH_MAX];
//If unsuccessful, getcwd() returns a NULL pointer
    if(getcwd(buffer , PATH_MAX) == NULL)
    {
        perror("smash error: getcwd failed");
        delete[] buffer;
        return;

    }
    cout << buffer << endl;
    delete[] buffer;

}

void ChangeDirCommand::execute() {

    SmallShell & my_shell = SmallShell::getInstance();
    //if arg_num >2 : too many argument
    if (this->arg_num > 2) {
        std::cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
    else if (this->arg_num == 1){
        std::cerr << "smash error: " <<  "\"" <<this->command << "\"" << ": too few arguments" << endl;
    return;
    }else if(strcmp(arg[1],"-")==0) {
        //If the last working directory is empty and “cd -“ was called with no last dirictory : OLDPWD not set

            if (!(my_shell.last_dir)) { //when we change the plastpwd we change the last_dir at the shell
                cerr << "smash error: cd: OLDPWD not set" << endl;
                return;
            }
            //putting the curr path into the shell->last_dir and then change the path using chdir
            char *buffer = new char[PATH_MAX]; // don't forget to delete it
            if(getcwd(buffer , PATH_MAX) == NULL)
            {
                perror("smash error: getcwd failed");
                delete[] buffer;
                return;
            }
            if(chdir(my_shell.last_dir)!=0)// changing the cur dir to be the last dir
            {
                perror("smash error: chdir failed");
                delete[] buffer;
                return;
            }
         if(my_shell.last_dir) delete my_shell.last_dir;
        my_shell.last_dir = new char[PATH_MAX];
        strcpy(my_shell.last_dir , buffer); // changing the last dir to be the curr before changing
            delete[] buffer;
            return;

    }else{
        //putting the curr path into the shell->last_dir and then change the path using chdir
        char *buffer = new char[PATH_MAX]; // don't forget to delete it
        if(getcwd(buffer , PATH_MAX) == NULL)
        {
            perror("smash error: getcwd failed");
            delete[] buffer;
            return;
        }
        if(chdir(arg[1])!=0)// changing the cur dir to be the given path that is saved at arg[1]
        {
            perror("smash error: chdir failed");
            return;
        }
        if(my_shell.last_dir) delete my_shell.last_dir;
        my_shell.last_dir = new char[PATH_MAX];
        strcpy(my_shell.last_dir , buffer); // changing the last dir to be the curr before changing
        delete[] buffer;

    }
}

void JobsCommand::execute() {

    jobs->printJobsList();
}

void ForegroundCommand::execute() {
   /* SmallShell & my_shell = SmallShell::getInstance();
    my_shell.jobs->printJobsList();*/
    JobsList::JobEntry* job;
   // cout<< cmd_line << endl;
    if(this->arg_num == 1 )
    {
        if(this->jobs->jobs_list.empty()) {
            std::cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
                int last_job_id=0;
                 job = jobs->getLastJob(&last_job_id);// bringing the last job

    }else{
        if(this->arg_num >2 || !isInteger(this->arg[1]))
        {
            std::cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }

         job = jobs->getJobById(std::stoi(this->arg[1]));// bringing the job with arg[1] id
        if(job == nullptr)
        {
            std::cerr <<"smash error: fg: job-id " << arg[1] << " does not exist" << endl;
            return ;
        }
    }
    // if job is stopped then we should use SIGCONT to make it continue
    if(job->stopped_mark)
    {
        job->stopped_mark = false;
        if (kill(job->pid, SIGCONT) !=0) {/// job_pid
            perror("smash error: kill failed");
            return;
        }
    }
    cout << job->cmd_line << " : " << job->pid<< endl;
    //this->jobs->removeJobById(job->pid);
     SmallShell::getInstance().curr_proccess_pid = job->pid;
    strcpy( SmallShell::getInstance().fg_cmd_line,job->cmd_line);
    if(waitpid(job->pid, nullptr,WUNTRACED) == -1)
    {
        perror("smash error: waitpid failed");
    }
    SmallShell::getInstance().curr_proccess_pid =-1;

}

void BackgroundCommand::execute() {
    JobsList::JobEntry* job;
    if(this->arg_num == 1 )
    {
        if(this->jobs->jobs_list.empty()) {
            std::cerr<< "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
        int last_stopped_job_id=0;
        job = jobs->getLastStoppedJob(&last_stopped_job_id);// bringing the last stopped job
        if(job == nullptr) {
            std::cerr<< "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }


    }else{
        if(this->arg_num >2 || !isInteger(this->arg[1]))
        {
            std::cerr << "smash error: bg: invalid arguments" << endl;
            return;
        }

        job = jobs->getJobById(std::stoi(this->arg[1]));// bringing the job with arg[1] id

        if(job == nullptr)
        {
            std::cerr << "smash error: bg: job-id " << arg[1] << " does not exist" << endl;
            return ;
        }
        if(job->stopped_mark == false)
        {
            std::cerr << "smash error: bg: job-id " << arg[1] << " is already running in the background" << endl;
            return;
        }
    }

    job->stopped_mark = false;
    cout << job->cmd_line << " : " << job->pid << endl;
    if (kill(job->pid, SIGCONT) !=0) {/// job_pid
        perror("smash error: kill failed");
        return;
    }

}

void QuitCommand::execute() {
    if(arg_num>1 && !strcmp(arg[1],"kill"))
    {

        std::cout<<"smash: sending SIGKILL signal to "<< jobs->jobs_list.size() <<" jobs:"<<std::endl;
        jobs->killAllJobs();
    }
    delete this;
    exit(0);
}
void FareCommand::execute() {
    if (arg_num != 4) {
        cerr << "smash error: fare: invalid arguments" << endl;
        return;
    }
    //open the file in arg[1] seek pointer for read

    int source_fd = open(arg[1], O_RDWR);
    if (source_fd == -1) {
        perror("smash error: open failed");
        return;
    }
    //open the file in arg[1] seek pointer for write
    int tmp_fd = open("tmp", O_RDWR | O_CREAT | O_TRUNC, 0665);
    if (tmp_fd == -1) {
        perror("smash error: open failed");
        return;
    }
    string source = string(arg[2]), destination = string(arg[3]);

    int source_len = source.length();
    char read_letter;
    char *buff = new char[source_len + 1]();
    buff[0] = arg[2][0];
    int counter = 0;
    while (read(source_fd, &read_letter, sizeof(char)) > 0) {
        if (read_letter == buff[0]) {
            if (source_len == 1) {
                if (write(tmp_fd, arg[3], destination.length() * sizeof(char)) == -1) {
                    perror("smash error: write failed");
                    return;
                }

                counter++;

            } else {
                if (read(source_fd, buff + 1, source_len - 1) > 0) {
                    if (!source.compare(buff)) {
                        if (write(tmp_fd, arg[3], destination.length() * sizeof(char)) == -1) {
                            perror("smash error: write failed");
                            return;
                        }
                        counter++;

                    } else {
                        if (write(tmp_fd, buff, source_len) == -1) {
                            perror("smash error: write failed");
                            return;
                        }
                    }
                }
            }
        } else {
            if (write(tmp_fd, &read_letter, sizeof(char)) == -1) {
                perror("smash error: write failed");
                return;
            }
        }
    }

    cout << "replaced " << counter <<" instances of the string " << "\""<<arg[2] << "\"" << endl;
    int fd = open(arg[1],O_TRUNC | O_RDWR);
    if (fd == -1) {
        perror("smash error: open failed");
        return;
    }
    int fd_to_copy = open("tmp",O_RDWR);
    if (fd_to_copy == -1) {
        perror("smash error: open failed");
        return;
    }
    while(read(fd_to_copy,&read_letter,sizeof(char))>0){
        if(write(fd, &read_letter, sizeof(char)) == -1)
        {
            perror("smash error: write failed");
            return;
        }
    }

    delete [] buff;
    close(source_fd);
    close(tmp_fd);
    close(fd);
    close(fd_to_copy);
}

void KillCommand::execute() {

    //invalid arg situations:
    // 1- num of arg is not 3 , 2- the signum and job id are not numbers ,
    // 3- signum is not  0 < abs(signum) < 32 because it should be in range 1->31 except SIGKILL (9) and -SIGSTOP (17) and SIGCONT
    if(arg_num !=3 || !isInteger(arg[1]) || !isInteger(arg[2]) || stoi(arg[1])>=0  ||   stoi(arg[1])< -31  ){
        std::cerr << "smash error: kill: invalid arguments" << endl;
        return ;
    }
    int signal=stoi(arg[1]) *-1;
    JobsList::JobEntry* job=  jobs->getJobById(stoi(arg[2]));
    if (job == nullptr)
    {
        std::cerr << "smash error: kill: job-id " << arg[2] << " does not exist" << endl;
        return ;
    }
    if(signal == SIGSTOP)
    {
        job->stopped_mark = true;
    }
    if(signal == SIGCONT)
    {
        job->stopped_mark = false;
    }
    if(kill(job->pid, signal)!=0) // sending the signal
    {
        perror("smash error: kill failed");
    }
    if(signal==SIGKILL)
    {
        jobs->removeJobById(job->job_id);
    }
    cout << "signal number " << signal <<" was sent to pid " << job->pid << endl;
}

/*******************************-------EXTERNAL_COMMANDS------**********************************************************/
static bool _isComplexCommand(const char* cmd_line){
    string str = string(cmd_line);
    if(str.find('?') != std::string::npos || str.find('*') != std::string::npos )
        return true;
    return false;
}


void ExternalCommand::execute() {

    char cmd_line_copy[COMMAND_ARGS_MAX_LENGTH] = "" ;
    strcpy(cmd_line_copy, _trim(string(cmd_line)).c_str());

    bool is_background = _isBackgroundComamnd(cmd_line);
    if (is_background)
        _removeBackgroundSign(cmd_line_copy);

   // const char* cmd = _trim(string(cmd_line_copy)).c_str();
    bool is_complex = _isComplexCommand(cmd_line);
    //fork
    pid_t new_pid = fork();

    char executable[] = "/bin/bash",executable_name[] = "/bin/bash",flags[] = "-c";
    char *fork_args[] = {executable_name, flags, cmd_line_copy, nullptr};

    //son proccess
    if(new_pid == 0) {
        //setpgrp
        if (setpgrp() == -1) {
            perror("smash error: setpgrp failed");
            return;
        }

        if (!is_complex) {//simple, check if background
           char **simple_fork_args = new char*[COMMAND_MAX_ARGS];
            _parseCommandLine(cmd_line_copy, simple_fork_args);
            if (execvp(arg[0], simple_fork_args) == -1) {
                perror("smash error: execvp failed");
                return;
            }
        } else {//complex
            if (execvp(executable, fork_args) == -1) {
                perror("smash error: execvp failed");
                return;
            }
        }
    }else{ //father

        if (is_background) {
            //////add to job

            SmallShell::getInstance().jobs->addJob(this, new_pid,false );


        } else {//wait
            SmallShell::getInstance().curr_proccess_pid = new_pid;
            strcpy(SmallShell::getInstance().fg_cmd_line,cmd_line);
            if (waitpid(new_pid, nullptr, WUNTRACED) == -1) {
                perror("smash error: waitpid failed");
                return;
            }
            SmallShell::getInstance().curr_proccess_pid = -1;

        }
    }


}
/*******************************-------SPECIAL_COMMANDS------**********************************************************/
RedirectionCommand::RedirectionCommand(const char *cmd_line): Command(cmd_line) {
    string my_str=string (cmd_line);
     size_t index = my_str.find(">>");

    if(index == std::string::npos) { // ">"
        int override_index = string(cmd_line).find(">");
        this->override = true;
        strcpy(this->command, _trim(my_str.substr(0, override_index)).c_str());
        strcpy(this->file_name , _trim(my_str.substr(override_index+1, my_str.size() - override_index - 1)).c_str());
    }
    else{
        this->override = false;
        strcpy(this->command, _trim(my_str.substr(0, index)).c_str());
        strcpy(this->file_name , _trim(my_str.substr(index+2, my_str.size() - index - 1)).c_str());
    }

}

void RedirectionCommand::execute() {
///preparing
    //first we have to call open syscall
    int fd = -1;
    if(override)
    {
        fd=open(file_name,O_RDWR|O_CREAT|O_TRUNC,0655);
    }
    else
    {
        fd=open(file_name,O_RDWR|O_CREAT|O_APPEND,0655);
    }
    if(fd==-1)
    {
        perror("smash error: open failed");
        return ;
    }

    //then we have to duplicate what is in the fd stdout into tmp_stdout
    int tmp_stdout = dup(1);
    if (tmp_stdout == -1) {
        perror("smash error: dup failed");
        return;
    }

    // then we have to close FDT[1] // do we?
    if (close(1) == -1) {
        perror("smash error: close failed");
        return;
    }

    //then we have to duplicate the returned fd from open into the stdout FDT[1]
    if (dup2(fd, 1) == -1) {
        perror("smash error: dup2 failed");
        return ;
    }

    SmallShell::getInstance().executeCommand(command);

    ///cleanup
    //close the returned fd from open
    if (close(fd) == -1) {
        perror("smash error: close failed");
        return;
    }
    //duplicate the tmp_stdout into the stdout FDT[1] back
    if (dup2(tmp_stdout, 1) == -1) {
        perror("smash error: dup2 failed");
        return ;
    }

    //close the tmp_stdout
    if (close(tmp_stdout) == -1) {
        perror("smash error: close failed");
        return;
    }

}

PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line) {


    string my_str=string (cmd_line);
    size_t index = my_str.find("|&");
    if(index == std::string::npos) { // "|"
        int no_ampersand_index = string(cmd_line).find("|");
        this->no_ampersand = true;
        strcpy(this->first_command, _trim(my_str.substr(0, no_ampersand_index)).c_str());
        strcpy(this->second_command, _trim(my_str.substr(no_ampersand_index+1, my_str.size() - no_ampersand_index - 1)).c_str());
    } else{
        this->no_ampersand = false;
        strcpy(this->first_command, _trim(my_str.substr(0, index)).c_str());
        strcpy(this->second_command , _trim(my_str.substr(index+2, my_str.size() - index - 1)).c_str());
    }
}

void PipeCommand::execute() {
    int fd[2]; //fd[0] read , fd[1] write
    pipe(fd);
    pid_t first_child = fork();// first child for the write
    if (first_child == -1) { // check if fork success
        perror("smash error: fork failed");
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
        }
        return;
    }
    if (first_child == 0) { //first son
        if (setpgrp() == -1) { //check if setpgrp success
            perror("smash error: setpgrp failed");
            if (close(fd[0]) == -1) {
                perror("smash error: close failed");
            }
            if (close(fd[1]) == -1) {
                perror("smash error: close failed");
            }
            return;
        }
        if (no_ampersand) {
            if (dup2(fd[1], 1) == -1) {//fd[1] is in stdout now
                perror("smash error: dup2 failed");
                if (close(fd[0]) == -1) {// close fd[0] cause we want to write from this child not read
                    perror("smash error: close failed");
                }
                if (close(fd[1]) == -1) {// close fd[1] cause we duplicated it to stdout
                    perror("smash error: close failed");
                }
                return;
            }
        } else {
            if (dup2(fd[1], 2) == -1) {//fd[1] is in stderr now
                perror("smash error: dup2 failed");
                if (close(fd[0]) == -1) {// close fd[0] cause we want to write from this child not read
                    perror("smash error: close failed");
                }
                if (close(fd[1]) == -1) {// close fd[1] cause we duplicated it to stderr
                    perror("smash error: close failed");
                }
                return;
            }
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
        }
        SmallShell::getInstance().executeCommand(first_command);
        exit(0);
    }
    if (first_child != 0) {
        if (waitpid(first_child, nullptr, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
            return;
        }
    }

    pid_t second_child = fork(); // second child for the read
    if (second_child == -1) {
        perror("smash error: fork failed");
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
        }
        return;
    }
    if (second_child == 0) { //second son
        if (setpgrp() == -1) { //check if setpgrp success
            perror("smash error: setpgrp failed");
            if (close(fd[0]) == -1) {
                perror("smash error: close failed");
            }
            if (close(fd[1]) == -1) {
                perror("smash error: close failed");
            }
            return;
        }
        if (dup2(fd[0], 0) == -1) {///fd[1] is in stdin now
            perror("smash error: dup2 failed");
            if (close(fd[0]) == -1) {// close fd[0] cause we duplicated it to stdin
                perror("smash error: close failed");
            }
            if (close(fd[1]) == -1) {// close fd[1] cause we want to read from this child not write
                perror("smash error: close failed");
            }
            return;
        }
        if (close(fd[0]) == -1) {
            perror("smash error: close failed");
        }
        if (close(fd[1]) == -1) {
            perror("smash error: close failed");
        }
        SmallShell::getInstance().executeCommand(second_command);
        exit(0);
    }
    if (close(fd[0]) == -1) {
        perror("smash error: close failed");
    }
    if (close(fd[1]) == -1) {
        perror("smash error: close failed");
    }
    if(second_child!=0)
    {
        if (waitpid(second_child,nullptr, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
            return;
        }
    }

}


