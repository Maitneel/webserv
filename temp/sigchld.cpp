#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
using namespace std;


void sig_hand(int) {
    exit(0);
}

void child() {
    system("sleep 1");
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        child();
        exit(0);
    }
    signal(SIGCHLD, sig_hand);
    while (1) {
        cerr << "waiting..." << endl;
    }
}