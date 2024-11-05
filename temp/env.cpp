#include <iostream>
using namespace std;

#define debug(s) cout << #s << " : " << s << endl;

int main(int c, char **v, char **env) {
    int i = 0;
    printf("env: %p\n", env);
    printf("env: %p\n", env[0]);

    while (env[i] != NULL) {
        cerr << env[i] << endl;
        i++;
    }
    return 0;
} 