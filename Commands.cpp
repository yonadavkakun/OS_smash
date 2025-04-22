#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <sys/syslimits.h>

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

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
    // TODO: add your implementation
}

SmallShell::~SmallShell() {
    // TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("chprompt") == 0) {
        return new ChpromptCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    } else if (firstWord.compare("jobs") == 0) {
        //return
        return new JobsCommand(cmd_line);
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    } else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line);
    } else if (firstWord.compare("alias") == 0) {
        return new AliasCommand(cmd_line);
    } else if (firstWord.compare("unalias") == 0) {
        return new UnAliasCommand(cmd_line);
    } else if (firstWord.compare("unsetenv") == 0) {
        return new UnSetEnvCommand(cmd_line);
    } else if (firstWord.compare("watchproc") == 0) {
        return new WatchProcCommand(cmd_line);
    } else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}

void ChpromptCommand::execute() {
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
        perror("smash error: getcwd failed");
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
        newPath = m_argv[1];
    }
    char cwd[PATH_MAX];
    if (!getcwd(cwd, PATH_MAX)) {
        perror("smash error: getcwd failed");
        return;
    }
    if (chdir(newPath.c_str()) == -1) {
        perror("smash error: chdir failed");
        return;
    }
    SmallShell::getInstance().setLastPWD(cwd);
}


//--------------------SMASH!!!!!--------------------//
void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

std::string SmallShell::getPrompt() {
    return this->m_prompt;
}

void SmallShell::setPrompt(std::string value) {
    this->m_prompt = value;
}

JobsList &SmallShell::getJobsList() {
    return this->m_jobsList;
}

std::string SmallShell::getLastPWD() {
    return this->m_lastPWD;
}

void SmallShell::setLastPWD(std::string value) {
    this->m_lastPWD = value;
}

//--------------------JOBS LIST!!!!!--------------------//
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
        else if (result == 0) ++iter;
        else iter = m_jobs.erase(iter);
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
        iter = m_jobs.erase(iter);
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    this->removeFinishedJobs();
    auto iter = m_jobs.begin();
    while (iter != m_jobs.end()) {
        if (iter->first == jobId) return &iter->second;
        ++iter;
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    this->removeFinishedJobs();
}

JobsList::JobEntry* JobsList::getLastJob(int *lastJobId) {
    this->removeFinishedJobs();
}

JobsList::JobEntry* JobsList::getLastStoppedJob(int *jobId) {
    this->removeFinishedJobs();
}

//--------------------COMMAND CLASS!!!!!--------------------//
std::string Command::getCmdLine() {
    return this->m_cmd_line;
}



