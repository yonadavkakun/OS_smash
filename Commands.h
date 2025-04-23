// Ver: 10-4-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <map>
#include <memory>
#include <utility>
#include <vector>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    // TODO: Add your data members
protected:
    std::string m_cmdLine;
    int m_argc;
    std::string m_argv[COMMAND_MAX_ARGS+1];
    bool m_isBackgroundCommand;
public:
    Command(const std::string cmd_line);
    virtual ~Command();
    virtual void execute() = 0;

    bool getIsBackgroundCommand();
    std::string getCmdLine();
    std::string getCmdLineFull();
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const std::string cmd_line) : Command(cmd_line) {};
    virtual ~BuiltInCommand() {}
};
class ExternalCommand : public Command {
public:
    ExternalCommand(const std::string cmd_line) : Command(cmd_line) {};;
    virtual ~ExternalCommand() {}
    void execute() override;
};


class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const std::string cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
};
class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const std::string cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};
class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const std::string cmd_line) : Command(cmd_line) {};
    virtual ~DiskUsageCommand() {}
    void execute() override;
};
class WhoAmICommand : public Command {
public:
    WhoAmICommand(const std::string cmd_line) : Command(cmd_line) {};
    virtual ~WhoAmICommand() {}
    void execute() override;
};
class NetInfo : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    NetInfo(const std::string cmd_line);
    virtual ~NetInfo() {}
    void execute() override;
};
//cd
class ChangeDirCommand : public BuiltInCommand {
public:
    std::string m_preChangePWD;
    ChangeDirCommand(const std::string cmd_line) = delete;
    ChangeDirCommand(const std::string cmd_line, std::string plastPwd) : BuiltInCommand(cmd_line) {m_preChangePWD = plastPwd;};
    virtual ~ChangeDirCommand() {}
    void execute() override;
};
//pwd
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {};
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};
//showpid
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {};
    virtual ~ShowPidCommand() {}
    void execute() override;
};
//chprompt
class ChPromptCommand : public BuiltInCommand
{
public:
    ChPromptCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {}
    virtual ~ChPromptCommand() {}
    void execute() override;
};

class JobsList;
//quit
class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    JobsList* m_jobsListPtr;
public:
    QuitCommand(const std::string cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), m_jobsListPtr(jobs) {};
    virtual ~QuitCommand() {}
    void execute() override;
};
class JobsList {
public:
    class JobEntry {
    public:
        std::string m_jobCommandString;
        pid_t m_jobPID;
        int m_jobID;
        bool m_isStopped;

        JobEntry(std::string commandString, pid_t PID, int ID, bool isStopped) : m_jobCommandString(commandString), m_jobPID(PID), m_jobID(ID), m_isStopped(isStopped) {};
    };

private:
    std::map<int, JobEntry> m_jobs;


public:
    int calcNewID();

    JobsList();

    ~JobsList();

    void addJob(Command* cmd, bool isStopped = false, pid_t jobPID);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry* getJobById(int jobId); //remember returns nullptr if doesnt exist.

    void removeJobById(int jobId); // TODO: ask

    JobEntry* getLastJob(int* lastJobId);

    JobEntry* getLastStoppedJob(int* jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};
//jobs
//No direct system calls needed.
//Use internal job list (vector/map with PIDs + metadata)
//waitpid(pid, ...) with WNOHANG to check if jobs are still running
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* m_jobsListPtr;
public:
    JobsCommand(const std::string cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), m_jobsListPtr(jobs) {};
    virtual ~JobsCommand() {}
    void execute() override;
};
//kill
class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* m_jobsListPtr;
public:
    KillCommand(const std::string cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), m_jobsListPtr(jobs) {};
    virtual ~KillCommand() {}
    void execute() override;
};
//fg: waitpid(pid, 0, 0)
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* m_jobsListPtr;
public:
    ForegroundCommand(const std::string cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), m_jobsListPtr(jobs) {};
    virtual ~ForegroundCommand() {}
    void execute() override;
};
//alias
class AliasCommand : public BuiltInCommand {
public:
    AliasCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~AliasCommand() {
    }

    void execute() override;
};
//unalias
class UnAliasCommand : public BuiltInCommand {
public:
    UnAliasCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};
//unsetenv
class UnSetEnvCommand : public BuiltInCommand {
public:
    UnSetEnvCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};
//watchproc
class WatchProcCommand : public BuiltInCommand {
public:
    WatchProcCommand(const std::string cmd_line) : BuiltInCommand(cmd_line) {};

    virtual ~WatchProcCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    std::string m_prompt = "smash";
    JobsList m_jobsList;
    std::string m_lastPWD;
    SmallShell();

public:
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();

    Command *CreateCommand(const char *cmd_line);
    void executeCommand(const char *cmd_line);

    // TODO: add extra methods as needed
    std::string getPrompt();
    void setPrompt(std::string value);
    JobsList* getJobsList(); //TODO: maybe reference instead of ptr - vise-versa
    std::string getLastPWD();
    void setLastPWD(std::string value);
};

#endif //SMASH_COMMAND_H_
