#include <iostream>
#include <unistd.h>
using namespace std;

int main() {
    pid_t child_pid = fork();
    cout << "pid " << child_pid << endl;

    if (child_pid == 0) {
        system("sleep 1");
        execve("./cgi_script/date/date.cgi", NULL, NULL);
        exit(1);
    }
    int status;
    cout << "waiting..." << endl;
    waitpid(child_pid, NULL, 0);
    cout << "status: " << status << endl;
}