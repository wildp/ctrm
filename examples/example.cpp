#include <iostream>

#include "../ctrm.hpp"

int main()
{
    constexpr auto register_machine { ctrm::make<3, 5>(
            "L0 : R1- -> L1, L2\n"
            "L1 : R0+ -> L0\n"
            "L2 : R2- -> L3, L4\n"
            "L3 : R0+ -> L2\n"
            "L4 : HALT") };
    
    constexpr auto result{ register_machine.exec<unsigned int>(0, 1, 2) };
    
    std::cout << result << '\n';
    return 0;
}
