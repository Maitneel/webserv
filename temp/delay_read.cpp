#include <unistd.h>
#include <iostream>
using namespace std;

#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    int read_size;

    do {
        read_size = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        cout << string(buffer, read_size) << endl;
        sleep(1);
    } while (0 < read_size);
}