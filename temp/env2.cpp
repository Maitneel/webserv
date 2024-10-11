#include <iostream>
#include <unistd.h>
#include <stdlib.h>
using namespace std;

void child() {
    char **env = (char **)(malloc(sizeof(char *) * 3));
    env[0] = strdup("HOGE=hogehoge");
    env[1] = strdup("PATH=hogehoge/hogehoge");
    env[2] = NULL;
    execve("./a.out", NULL, env);
    exit(1);
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        child();
    }
    int n = 0;;
    waitpid(pid, &n, 0);
    cout << "end status: " << n << endl;
}