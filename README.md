# ctrm
Compile-time register machines for C++20.

### Usage:
```c++
constexpr auto register_machine = ctrm::make<3, 5>(
        "L0 : R1- -> L1, L2\n"
        "L1 : R0+ -> L0\n"
        "L2 : R2- -> L3, L4\n"
        "L3 : R0+ -> L2\n"
        "L4 : HALT");

constexpr auto result{ register_machine.exec(0, 1, 2) }; //  Has value 3
```
Register machine programs can be created with the function 
`ctrm::make<i, j>(s)`, where `i` is the number of registers used by the
program, `j` is the number of instructions in the program, and `s` is the
program.

Programs can be executed with `program.exec<type>(ints...)`, where `type` is an
unsigned integral type (default: `std::size_t`) used for the values in the
registers  and the return type, and `ints...` is a list of unsigned integrals
that define the register machine's initial configuration.

Alternatively, when using a compiler that supports string literal operator
templates, the above code can be written as:
```c++
auto result = "L0 : R1- -> L1, L2\n"
              "L1 : R0+ -> L0\n"
              "L2 : R2- -> L3, L4\n"
              "L3 : R0+ -> L2\n"
              "L4 : HALT"_ctrm.exec(0, 1, 2);
```

### Program Syntax
```
program ::= { ( increment | decrement | halt ) , line_end } ;

increment ::= register , plus , arrow , register ;        
decrement ::= register , minus , arrow , register , comma , register ;
halt ::= "H" , "A" , "L" , "T" ;

line_prefix ::= { space } , location, { space } , ":" , { space } ;
line_end ::= { space } , line_separator , { line_separator } ;
line_separator ::= "\0" | ";" | "\n" ;

location ::= "L" , integer ;
register ::= "R" , integer ;
integer ::= integer , { integer } ;

comma ::= { space } , "," , { space }
plus ::= { space } , "+" , { space }
minus ::= { space } , "-" , { space }

arrow ::= "-" , ">" ; { space }
space ::= " " | "\t" ;
digit ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" ;
```


