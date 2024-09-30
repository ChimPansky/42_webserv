#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"

#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

// Timeout in seconds
#define TIMEOUT 1

pid_t child_pid = -1;

void handle_timeout(int sig) {
    if (sig == SIGALRM) {
        std::cout << "Test timed out! Killing child process..." << std::endl;
        if (child_pid != -1) {
            kill(child_pid, SIGKILL);  // Forcefully kill the child
        }
    }
}

//TEST(TestSuiteName, TestName)
TEST(RQBuilderTest1, Positive) {
    child_pid = fork();
    if (child_pid == 0) {
        http::RequestBuilder rq1 = http::RequestBuilder();
        rq1.buf().resize(100);
        // char* ptr = nullptr;
        // char c = *ptr;
        // (void)c;
        std::memcpy(rq1.buf().data(), "GET / HTTP/1.1\r\n", 16);
        // maybe fork() and run loop in child to catch possible infinite loop (with timeout)
        while (!rq1.IsReadyForResponse()) {
            rq1.ParseNext();
        }
        EXPECT_EQ(http::HTTP_GET, rq1.rq().method);
        exit(0);
    } // parent process
    signal(SIGALRM, handle_timeout);  // Register timeout handler
    alarm(TIMEOUT);  // Set timeout for the child process
    int status;
    pid_t result = waitpid(child_pid, &status, 0);
    if (result == -1) {
            perror("waitpid failed");
    }
    else if (WIFSIGNALED(status)) {
        int term_signal = WTERMSIG(status);
        if (term_signal == SIGSEGV) {
            std::cout << "Child terminated due to segmentation fault (SIGSEGV)." << std::endl;
            FAIL() << "Child process crashed with SIGSEGV.";
        } else if (term_signal == SIGKILL) {
            std::cout << "Child was killed (possibly due to timeout)." << std::endl;
            FAIL() << "Child process was killed (timeout or other reasons).";
        } else {
            std::cout << "Child terminated by signal: " << term_signal << std::endl;
            FAIL() << "Child process terminated by signal: " << term_signal;
        }
    } else if (!WIFEXITED(status) || !(WEXITSTATUS(status) == 0)) {
        std::cout << "Child terminated unexpectedly." << std::endl;
        FAIL() << "Child process terminated unexpectedly.";
    }

    alarm(0);  // Cancel the alarm if the child finishes on time
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
