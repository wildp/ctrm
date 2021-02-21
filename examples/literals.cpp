#include <cstdio>

#include "../ctrm.hpp"

int main()
{
    using namespace ctrm::literals;
    
    auto result = "L0 : R1- -> L1, L2\n"
                  "L1 : R0+ -> L0\n"
                  "L2 : R2- -> L3, L4\n"
                  "L3 : R0+ -> L2\n"
                  "L4 : HALT"_ctrm.exec(0, 3, 5);
    
    std::printf("%lu\n", result);
    return 0;
}