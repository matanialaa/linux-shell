#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    cout << "smash: got ctrl-Z" << endl;
    SmallShell& smash = SmallShell::getInstance();
    if(smash.curr_proccess_pid != -1)
    {
        //add the foreground process to the jobs list
        Command *fg_cmd = smash.CreateCommand(smash.fg_cmd_line);
        smash.jobs->addJob(fg_cmd, smash.curr_proccess_pid,true); // with a flag that it is stopped
        //send SIGSTOP to the process in the foreground
        if(kill(smash.curr_proccess_pid,SIGSTOP)!=0)
        {
            perror("smash error: kill failed");
        }
        //print the following information
        cout<<"smash: process "<<smash.curr_proccess_pid<<" was stopped"<<endl;
        delete(fg_cmd);
        //update back the smash to be without a fg command
        smash.curr_proccess_pid = -1;
    }

}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout<<"smash: got ctrl-C"<<endl;
    SmallShell& smash = SmallShell::getInstance();
    if(smash.curr_proccess_pid != -1){
        if(kill(smash.curr_proccess_pid,SIGKILL)!=0)
        {
            perror("smash error: kill failed");
        }
        cout<<"smash: process "<<smash.curr_proccess_pid<<" was killed"<<endl;
    }
    //do we need to update back the smash to be without a fg command?? i think yes
    smash.curr_proccess_pid = -1;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

