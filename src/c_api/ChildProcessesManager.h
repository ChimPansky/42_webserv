#ifndef WS_C_API_CHILD_PROCESSES_MANAGER_H
#define WS_C_API_CHILD_PROCESSES_MANAGER_H

#include <SocketWrapper.h>
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
    virtual void Call(int child_exit_status) = 0;
    virtual ~IChildDiedCb() {};
};


struct ExecParams {
    ExecParams(const std::string& interpreter, const std::string& script_path,
               std::vector<std::string> child_env, const std::string& redirect_input_from_file)
        : interpreter(interpreter),
          script_path(script_path),
          child_env(child_env),
          redirect_input_from_file(redirect_input_from_file)
    {}
    const std::string& interpreter;
    const std::string& script_path;
    std::vector<std::string> child_env;
    const std::string& redirect_input_from_file;  // if empty - no redirect
};

class ChildProcessesManager {
  private:
    ChildProcessesManager();
    ChildProcessesManager(const ChildProcessesManager&);
    ChildProcessesManager& operator=(const ChildProcessesManager&);

  private:
    struct Child {
        Child(time_t time, utils::unique_ptr<IChildDiedCb> cb) : time_to_kill(time), cb_on_exit(cb)
        {}

        time_t time_to_kill;
        utils::unique_ptr<IChildDiedCb> cb_on_exit;
    };

  private:
    typedef std::map<pid_t, Child> PidtToCallbackMap;
    typedef std::map<pid_t, Child>::iterator PidtToCallbackMapIt;

    static inline int kDefaultTimeoutSeconds_() { return 5; }
    void RegisterChildProcess_(pid_t child_pid, time_t timeout_ts,
                               utils::unique_ptr<IChildDiedCb> cb);

  public:
    static void init();
    static ChildProcessesManager& get();

    void CheckOnce();
    std::pair<bool, utils::unique_ptr<SocketWrapper> > TryRunChildProcess(
        const ExecParams&, utils::unique_ptr<IChildDiedCb>);

  private:
    static utils::unique_ptr<ChildProcessesManager> instance_;
    PidtToCallbackMap child_processes_;
};

}  // namespace c_api

#endif  // WS_C_API_CHILD_PROCESSES_MANAGER_H
