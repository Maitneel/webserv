#include <iostream>
#include <iomanip>
#include <bitset>
using namespace std;

#define debug(s) std::cerr << setw(20) << #s << '\'' << (s) << '\'' << std::endl;

int main() {
    int n = 1 | 2 | 4;
    int not_n = ~n;
    int random_bit = 2 | 8 | 256;

    bitset<sizeof(int) * CHAR_BIT> n_bit(n);
    bitset<sizeof(int) * CHAR_BIT> not_n_bit(not_n);
    bitset<sizeof(int) * CHAR_BIT> rb_bit(random_bit);
    bitset<sizeof(int) * CHAR_BIT> mask_rb_bit(random_bit & (~n));


    debug(n_bit);
    debug(not_n_bit);
    debug(rb_bit);
    debug(mask_rb_bit);
    return 0;
}
