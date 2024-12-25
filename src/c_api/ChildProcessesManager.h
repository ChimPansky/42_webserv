#ifndef WS_C_API_CHILD_PROCESSES_MANAGER_H
#define WS_C_API_CHILD_PROCESSES_MANAGER_H

#include <Socket.h>
#include <logger.h>
#include <time_utils.h>
#include <unique_ptr.h>

#include <map>
#include <vector>

namespace c_api {

class IChildDiedCb {
  public:
    virtual void Call(int child_exit_status) = 0;
    virtual ~IChildDiedCb() {};
};

struct ExecParams {
    ExecParams(const std::string& interpreter, const std::string& script_location,
               const std::string& script_name, std::vector<std::string> child_env,
               const std::string& redirect_input_from_file)
        : interpreter(interpreter),
          script_location(script_location),
          script_name(script_name),
          child_env(child_env),
          redirect_input_from_file(redirect_input_from_file)
    {}
    std::string interpreter;
    std::string script_location;
    std::string script_name;
    std::vector<std::string> child_env;
    std::string redirect_input_from_file;  // if empty - no redirect
};

// Move Only Class!
class ChildProcessDescription {
  public:
    ChildProcessDescription() : pid_(-1) {}
    ChildProcessDescription(pid_t pid, utils::unique_ptr<Socket> sock) : pid_(pid), sock_(sock) {}
    pid_t pid() const { return pid_; }
    Socket& sock() const { return *sock_; }

  private:
    pid_t pid_;
    utils::unique_ptr<Socket> sock_;
};

class ChildProcessesManager {
  private:
    ChildProcessesManager();
    ChildProcessesManager(const ChildProcessesManager&);
    ChildProcessesManager& operator=(const ChildProcessesManager&);

  private:
    struct Child {
        Child(UnixTimestampS time_to_kill, utils::unique_ptr<IChildDiedCb> cb)
            : time_to_kill(time_to_kill), cb_on_exit(cb)
        {}

        UnixTimestampS time_to_kill;
        utils::unique_ptr<IChildDiedCb> cb_on_exit;
    };

  private:
    static inline int kDefaultTimeoutSeconds_() { return 5; }

    void CheckOnce_();
    utils::maybe<ChildProcessDescription> TryRunChildProcess_(const ExecParams&,
                                                              utils::unique_ptr<IChildDiedCb>);
    void RegisterChildProcess_(pid_t child_pid, UnixTimestampS timeout_ts,
                               utils::unique_ptr<IChildDiedCb> cb);
    void KillChildProcess_(pid_t pid) throw();

  public:
    static void init();
    static ChildProcessesManager& get();

    static inline void CheckOnce()
    {
        if (!instance_) {
            return;
        }
        return instance_->CheckOnce_();
    }

    static inline utils::maybe<ChildProcessDescription> TryRunChildProcess(
        const ExecParams& exec_params, utils::unique_ptr<IChildDiedCb> cb)
    {
        if (!instance_) {
            return utils::maybe_not();
        }
        return instance_->TryRunChildProcess_(exec_params, cb);
    }

    // no callback will be invoked
    static inline void KillChildProcess(pid_t pid) throw()
    {
        if (!instance_) {
            return;
        }
        instance_->KillChildProcess_(pid);
    };

  private:
    typedef std::map<pid_t, Child> PidtToCallbackMap;
    typedef std::map<pid_t, Child>::iterator PidtToCallbackMapIt;
    PidtToCallbackMap child_processes_;

    static utils::unique_ptr<ChildProcessesManager> instance_;
};

}  // namespace c_api

#endif  // WS_C_API_CHILD_PROCESSES_MANAGER_H
