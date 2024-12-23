#include "ChildProcessesManager.h"

#include <fcntl.h>
#include <file_utils.h>
#include <logger.h>

#include <cstdlib>
#include <cstring>

namespace c_api {

namespace {

// TODO: change exit to smth better mb
void SetUpChild(const ExecParams& params,
                utils::unique_ptr<Socket> child_socket)  // noreturn
{
    if (dup2(child_socket->sockfd(), STDOUT_FILENO) < 0) {
        LOG(ERROR) << "Dup2 failed: " << std::strerror(errno);
        exit(EXIT_FAILURE);
    }

    if (!params.redirect_input_from_file.empty()) {
        int rq_body_fd = open(params.redirect_input_from_file.c_str(), O_RDONLY);
        if (rq_body_fd < 0) {
            LOG(ERROR) << "Open failed: " << std::strerror(errno);
            exit(EXIT_FAILURE);
        }
        if (dup2(rq_body_fd, STDIN_FILENO) < 0) {
            LOG(ERROR) << "Dup2 failed: " << std::strerror(errno);
            exit(EXIT_FAILURE);
        }
    }

    // if (!utils::CloseProcessFdsButStd()) {
    //     LOG(ERROR) << "Cannot close fds, better death of 10000 children than a leak";
    //     exit(EXIT_FAILURE);
    // }

    std::vector<char*> env;
    for (size_t i = 0; i < params.child_env.size(); i++) {
        env.push_back(const_cast<char*>(params.child_env[i].c_str()));
    }
    env.push_back(NULL);

    std::vector<char*> args;
    args.push_back(const_cast<char*>(params.interpreter.c_str()));
    args.push_back(const_cast<char*>(params.script_name.c_str()));
    args.push_back(NULL);

    if (!utils::TryChangeDir(params.script_location.c_str())) {
        LOG(ERROR) << "chdir failed: " << std::strerror(errno);
        exit(EXIT_FAILURE);
    }
    // LOG(ERROR) << "\nInterpreter: " << params.interpreter
    //         << "\nScript path: " << args[1]
    //         << "\nenv: " << env[0];
    // char cwd[1024];
    // getcwd(cwd, sizeof(cwd));
    // LOG(ERROR) << "Current dir: " << cwd;
    LOG(WARNING) << "start of child ";
    execve(params.interpreter.c_str(), args.data(), env.data());
    LOG(ERROR) << "CGI failed with error " << std::strerror(errno);
    exit(EXIT_FAILURE);
}

}  // namespace

utils::unique_ptr<ChildProcessesManager> ChildProcessesManager::instance_(NULL);

ChildProcessesManager::ChildProcessesManager()
{}

void ChildProcessesManager::init()
{
    if (instance_) {
        throw std::runtime_error("ChildProcessesManager was already initialized");
    }
    instance_.reset(new ChildProcessesManager());
}

ChildProcessesManager& ChildProcessesManager::get()
{
    if (!instance_) {
        throw std::runtime_error("ChildProcessesManager is not initialised");
    }
    return *instance_;
}

void ChildProcessesManager::CheckOnce()
{
    for (PidtToCallbackMapIt it = child_processes_.begin(); it != child_processes_.end(); ++it) {
        if (std::time(NULL) >= it->second.time_to_kill) {
            LOG(ERROR) << "Timeout exceeded. Terminating child process...";
            kill(it->first, SIGKILL);
        }
    }

    std::vector<pid_t> children_to_delete;
    for (PidtToCallbackMapIt it = child_processes_.begin(); it != child_processes_.end(); ++it) {
        int child_exit_status;
        if (waitpid(it->first, &child_exit_status, WNOHANG) > 0) {
            if (WIFEXITED(child_exit_status)) {
                LOG(INFO) << "Child process exited with status " << WEXITSTATUS(child_exit_status);
            } else if (WIFSIGNALED(child_exit_status)) {
                LOG(ERROR) << "Child process terminated by signal " << WTERMSIG(child_exit_status);
            }
            it->second.cb_on_exit->Call(child_exit_status);
            children_to_delete.push_back(it->first);
        }
    }
    for (std::vector<pid_t>::const_iterator it = children_to_delete.begin();
         it != children_to_delete.end(); ++it) {
        child_processes_.erase(*it);
    }
}

void ChildProcessesManager::KillChildProcess(pid_t pid)
{
    PidtToCallbackMapIt it = child_processes_.find(pid);
    if (it != child_processes_.end()) {
        LOG(ERROR) << "Child process termination requested...";
        kill(it->first, SIGKILL);
        child_processes_.erase(it);
    }
}

void ChildProcessesManager::RegisterChildProcess_(pid_t child_pid, time_t timeout_ts,
                                                  utils::unique_ptr<IChildDiedCb> cb)
{
    // LOG(ERROR) << child_pid;
    child_processes_.insert(std::make_pair(child_pid, Child(timeout_ts, cb)));
}

utils::maybe<ChildProcessDescription> ChildProcessesManager::TryRunChildProcess(
    const ExecParams& params, utils::unique_ptr<IChildDiedCb> cb)
{
    utils::maybe<c_api::Socket::SocketPair> socket_pair =
        c_api::Socket::CreateLocalNonblockSocketPair();
    if (!socket_pair.ok()) {
        LOG(ERROR) << "Failed to create a socket_pair";
        return utils::maybe_not();
    }
    utils::unique_ptr<c_api::Socket>& parent_socket = socket_pair->first;
    utils::unique_ptr<c_api::Socket>& child_socket = socket_pair->second;

    pid_t child_pid = fork();
    if (child_pid < 0) {
        LOG(ERROR) << "Fork failed: " << std::strerror(errno);
        return utils::maybe_not();
    } else if (child_pid == 0) {
        parent_socket.reset();
        SetUpChild(params, child_socket);  // noreturn
    }
    RegisterChildProcess_(child_pid, std::time(NULL) + kDefaultTimeoutSeconds_(), cb);
    return ChildProcessDescription(child_pid, parent_socket);
}


}  // namespace c_api
