#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <vector>
using namespace std;

void child() {
    vector<char *>argc;
    argc.push_back(strdup("python3"));
    argc.push_back(strdup("./docs/cgi/python_cgi.py"));
    execve("/usr/local/bin/python3", argc.data(), NULL);
    exit(1);
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        child();
    }
    int exit_code;
    system("ps");
    usleep(1000);
    kill(pid, SIGTERM);
    system("ps");
    waitpid(pid, &exit_code, WNOHANG);
    cerr << "exit: " << exit_code << endl;;
    system("ps");
}