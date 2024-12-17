#ifndef WS_C_API_CHILD_PROCESSES_MANAGER_H
#define WS_C_API_CHILD_PROCESSES_MANAGER_H

#include <logger.h>
#include <signal.h>
#include <sys/wait.h>
#include <unique_ptr.h>

#include <ctime>
#include <map>
#include <vector>

namespace c_api {

class IChildDiedCb {
  public:
    virtual void Call(int fd) = 0;
    virtual ~IChildDiedCb(){};
};

class KillCallback : public IChildDiedCb {
  public:
    void Call(int fd);
};

class ChildExitedCallback : public IChildDiedCb {
  public:
    void Call(int fd);
};

struct Child {
    Child(time_t time, utils::unique_ptr<IChildDiedCb> cb) : time_to_kill(time), cb_on_exit(cb) {}
    time_t time_to_kill;
    utils::unique_ptr<IChildDiedCb> cb_on_exit;
};

class ChildProcessesManager {
  private:
    ChildProcessesManager();
    ChildProcessesManager(const ChildProcessesManager&);
    ChildProcessesManager& operator=(const ChildProcessesManager&);

  public:
    static void init()
    {
        if (instance_) {
            throw std::runtime_error("ChildProcessesManager was already initialized");
        }
        instance_.reset(new ChildProcessesManager());
    }
    static ChildProcessesManager& get()
    {
        if (!instance_) {
            throw std::runtime_error("ChildProcessesManager is not initialised");
        }
        return *instance_;
    }

    void CheckOnce()
    {
        int status = 0;
        for (PidtToCallbackMapIt it = child_processes_.begin(); it != child_processes_.end();
             ++it) {
            if (std::time(NULL) == (*it).second.time_to_kill) {
                LOG(ERROR) << "Timeout exceeded. Terminating child process...";
                kill((*it).first, SIGKILL);
            }
            if (waitpid((*it).first, &status, WNOHANG) > 0) {
                if (WIFEXITED(status)) {
                    LOG(ERROR) << "Child process exited with status " << WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
                    LOG(ERROR) << "Child process terminated by signal " << WTERMSIG(status);
                }
                (*it).second.cb_on_exit->Call();
                child_processes_.erase((*it).first);  // dont remove frome within the loop
            }
        }
    }

    // void CGIProcessor::ChildProcess::Monitor()
    // {
    //     int timeout = 5;

    //     while (true) {
    //         if (std::time(NULL) - start_time_ > timeout) {
    //             LOG(ERROR) << "Timeout exceeded. Terminating child process...";
    //             kill(child_pid_, SIGKILL);
    //         }
    //         if (waitpid(child_pid_, &status_, WNOHANG) > 0) {
    //             if (WIFEXITED(status_)) {
    //                 LOG(ERROR) << "Child process exited with status " << WEXITSTATUS(status_);
    //             } else if (WIFSIGNALED(status_)) {
    //                 LOG(ERROR) << "Child process terminated by signal " << WTERMSIG(status_);
    //             }
    //             break;
    //         }
    //     }
    // }

    int RegisterChildProcess(pid_t child_pid)
    {
        child_processes_.insert(std::make_pair(
            child_pid,
            Child(std::time(NULL) + kTimeoutSeconds(), utils::unique_ptr<IChildDiedCb>(NULL))));
    }

    void DeleteChildProcess(pid_t child_pid)
    {
        // UnRegisterChildProcess?
        child_processes_.erase(child_pid);
        // children_to_delete_.push_back();
    }

    // void    ClearCallback_(int fd)
    // {
    //     if (type & CT_READ && rd_sockets_.find(fd) != rd_sockets_.end()) {
    //         if (multiplexer_->UnregisterFd(fd, CT_READ, rd_sockets_, wr_sockets_) != 0) {
    //             LOG(ERROR) << "Could not unregister read callback for fd: " << fd;
    //         }
    //         rd_sockets_.erase(fd);
    //     }
    //     if (type & CT_WRITE && wr_sockets_.find(fd) != wr_sockets_.end()) {
    //         if (multiplexer_->UnregisterFd(fd, CT_WRITE, rd_sockets_, wr_sockets_) != 0) {
    //             LOG(ERROR) << "Could not unregister write callback for fd: " << fd;
    //         }
    //         wr_sockets_.erase(fd);
    //     }
    // }

    static inline int kTimeoutSeconds() { return 5; }
    typedef std::map<pid_t, Child> PidtToCallbackMap;
    typedef std::map<pid_t, Child>::const_iterator PidtToCallbackMapIt;

  private:
    static utils::unique_ptr<ChildProcessesManager> instance_;
    PidtToCallbackMap child_processes_;
};

}  // namespace c_api

#endif  // WS_C_API_CHILD_PROCESSES_MANAGER_H
