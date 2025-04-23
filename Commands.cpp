#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <limits.h>

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

//--------------------OWN HELPERS--------------------//
void printError(std::string sysCallName) {
    std::string errorText = "smash error: " + sysCallName + " failed";
    perror(errorText.c_str());
}
//--------------------GIVEN HELPERS--------------------//
string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}
int parseCommandLine(const std::string& cmd_line, std::string* args) {
    FUNC_ENTRY();
    std::istringstream iss(_trim(cmd_line));
    std::string token;
    int i = 0;

    while (iss >> token && i < COMMAND_MAX_ARGS+1) {
        args[i] = token;
        i++;
        if (i < COMMAND_MAX_ARGS+1) {
            args[i] = "";
        }
    }
    return i;
    FUNC_EXIT();
}

bool _isBackgroundCommand(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}
bool isBackgroundCommand(const std::string cmd_line) {
    return cmd_line[cmd_line.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
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
std::string removeBackgroundSign(const std::string cmd_line) {
    std::string result = cmd_line;
    result = _trim(result);

    size_t idx = result.find_last_not_of(WHITESPACE);
    if (idx == std::string::npos || result[idx] != '&') {
        return result;  // unchanged
    }

    // Remove '&' and trim trailing whitespace
    result[idx] = ' ';
    size_t newEnd = result.find_last_not_of(WHITESPACE, idx);
    return result.substr(0, newEnd + 1);
}


// TODO: Add your implementation for classes in Commands.h
void ChPromptCommand::execute() {
    if (m_argc == 1) {
        return;
    }
    SmallShell::getInstance().setPrompt(m_argv[1]);
}

void ShowPidCommand::execute() {
    pid_t pid = getpid();
    string curr_prompt = SmallShell::getInstance().getPrompt();
    cout << curr_prompt << " pid is " << pid << endl;
}

void GetCurrDirCommand::execute() {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, PATH_MAX)) {
        printError("getcwd");
        return;
    }
    std::cout << cwd << std::endl;
}

void ChangeDirCommand::execute() {
    if (m_argc == 1) {
        return;
    }
    if (m_argc > 2) {
        std::cerr << "smash error: cd: too many arguments" << std::endl;
        return;
    }
    std::string newPath;
    if (m_argv[1] == "-") {
        if (SmallShell::getInstance().getLastPWD() == "") {
            std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
            return;
        }
        newPath = SmallShell::getInstance().getLastPWD();
    } else {
        newPath = m_argv[1]; //TODO: check if its relative or global, if relative need to concatenate that string
    }
    char cwd[PATH_MAX];
    if (!getcwd(cwd, PATH_MAX)) {
        printError("getcwd");
        return;
    }
    if (chdir(newPath.c_str()) == -1) {
        perror("smash error: chdir failed");
        return;
    }
    SmallShell::getInstance().setLastPWD(cwd);
}


//--------------------SMASH CLASS!!!!!--------------------//
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
SmallShell::SmallShell() {
    // TODO: add your implementation
}
SmallShell::~SmallShell() {
    // TODO: add your implementation
}

Command* SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
    std::string cmd_s = _trim(string(cmd_line));
    std::string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    firstWord = removeBackgroundSign(firstWord); //cuz we can have "kill&" != "kill"

    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_s);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_s);
    } else if (firstWord.compare("chprompt") == 0) {
        return new ChPromptCommand(cmd_s);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_s, this->getLastPWD());
    } else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_s, this->getJobsList());
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_s, this->getJobsList());
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_s, this->getJobsList());
    } else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_s, this->getJobsList());
    } else if (firstWord.compare("alias") == 0) {
        return new AliasCommand(cmd_s);
    } else if (firstWord.compare("unalias") == 0) {
        return new UnAliasCommand(cmd_s);
    } else if (firstWord.compare("unsetenv") == 0) {
        return new UnSetEnvCommand(cmd_s);
    } else if (firstWord.compare("watchproc") == 0) {
        return new WatchProcCommand(cmd_s);
    } else {
        return new ExternalCommand(cmd_s);
    }
    return nullptr;
}
void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    //TODO: maybe delete cmd is needed as CreateCommand() is "new command"

    //Please note that you must fork smash process for some commands (e.g., external commands....)
}

std::string SmallShell::getPrompt() {
    return this->m_prompt;
}
void SmallShell::setPrompt(std::string value) {
    this->m_prompt = value;
}

std::string SmallShell::getLastPWD() {
    return this->m_lastPWD;
}
void SmallShell::setLastPWD(std::string value) {
    this->m_lastPWD = value;
}

JobsList* SmallShell::getJobsList() {
    return &this->m_jobsList;
}

//--------------------JOBSLIST CLASS!!!!!--------------------//
int JobsList::calcNewID() {
    if (m_jobs.size() == 0) return 1;
    auto last = --m_jobs.end();
    return last->first + 1;
}

void JobsList::removeFinishedJobs() {
    auto iter = m_jobs.begin();
    while (iter != m_jobs.end()) {
        int status; //might need to use in the future, currently unsure if status is needed
        pid_t result = waitpid(iter->second.m_jobPID, &status, WNOHANG);

        if (result == -1) printError("waitpid");
        if (result <= 0) ++iter;
        else iter = m_jobs.erase(iter); //TODO: make sure i dont want to remove failed jobs where waitpid() returned -1
    }
}

void JobsList::addJob(Command *cmd, bool isStopped, pid_t jobPID) {
    this->removeFinishedJobs();
    int uniqueID = calcNewID();
    std::string cmdLine = cmd->getCmdLine();
    JobEntry newJob(cmdLine, jobPID, uniqueID, isStopped);
    m_jobs.insert({uniqueID, newJob});
}

void JobsList::printJobsList() {
    this->removeFinishedJobs();
    auto iter = m_jobs.begin();
    while (iter != m_jobs.end()) {
        std::cout << "[" << iter->first << "] " << iter->second.m_jobCommandString << std::endl;
        ++iter;
    }
}

void JobsList::killAllJobs() {
    this->removeFinishedJobs();
    std::cout << "smash: sending SIGKILL signal to " << m_jobs.size() << " jobs:" << std::endl;
    auto iter = m_jobs.begin();
    while (iter != m_jobs.end()) {
        std::cout << iter->second.m_jobPID << ": " << iter->second.m_jobCommandString << std::endl;
        int result = kill(iter->second.m_jobPID, SIGKILL);
        if (result == -1) printError("kill");
        ++iter; //jobs will be removed anyway on next call for any method of JobsList
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    this->removeFinishedJobs();
    auto iter = m_jobs.find(jobId);
    if (iter == m_jobs.end()) return nullptr;

    return &iter->second;
}

void JobsList::removeJobById(int jobId) {
    this->removeFinishedJobs();
    auto iter = m_jobs.find(jobId);
    if (iter == m_jobs.end()) return;

    m_jobs.erase(iter);
}

JobsList::JobEntry* JobsList::getLastJob(int *lastJobId) {
    this->removeFinishedJobs();
    if (m_jobs.size() == 0) return nullptr;
    auto last = --m_jobs.end();
    *lastJobId = last->second.m_jobID;
    return &last->second;
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
    this->removeFinishedJobs();
    if (m_jobs.size() == 0) return nullptr;
    auto iterReversed = --m_jobs.end();
    while (iterReversed->second.m_isStopped != true && iterReversed != m_jobs.begin()) {
        --iterReversed;
    }
    if (iterReversed == m_jobs.begin() && iterReversed->second.m_isStopped != true) {
        return nullptr;
    }
    *jobId = iterReversed->second.m_jobID;
    return &iterReversed->second;
}

//--------------------COMMAND CLASS!!!!!--------------------//
Command::Command(const std::string cmd_line) {
    this->m_cmdLine = removeBackgroundSign(cmd_line);
    this->m_argc = parseCommandLine(cmd_line, this->m_argv);
    this->m_isBackgroundCommand = isBackgroundCommand(cmd_line);

}

bool Command::getIsBackgroundCommand() {
    return this->m_isBackgroundCommand;
}

std::string Command::getCmdLine() {
    return this->m_cmdLine;
}
std::string Command::getCmdLineFull() {
    if (this->m_isBackgroundCommand) {
        return this->m_cmdLine + "&";
    }
    return this->m_cmdLine;
}

