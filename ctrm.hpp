//  Copyright (c) 2021 Peter Wild
//
//  Permission to use, copy, modify, and/or distribute this software for any
//  purpose with or without fee is hereby granted.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
//  REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//  AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
//  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
//  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
//  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
//  PERFORMANCE OF THIS SOFTWARE.

#ifndef COMPILE_TIME_REGISTER_MACHINE_HPP
#define COMPILE_TIME_REGISTER_MACHINE_HPP

#include <array>
#include <concepts>
#include <limits>
#include <string_view>
#include <utility>

namespace ctrm
{
    namespace impl
    {
        enum instrType
        {
            HALT,
            INCR,
            DECR,
        };
        
        // Struct to hold every instruction
        struct instruction
        {
            instrType type;
            std::size_t currentRegister;
            std::size_t location1;
            std::size_t location2;
            
            //  Constructs a decrement instruction. When encountered, if reg is
            //  greater than zero, reg is decremented by 1 and the program
            //  jumps to the line specified by loc1, otherwise the program
            //  jumps to loc2. Syntax: "L{line}: R{reg}- -> L{loc1}, L{loc2}"
            constexpr instruction(std::size_t reg, std::size_t loc1, std::size_t loc2) :
                    type{ DECR },
                    currentRegister{ reg },
                    location1{ loc1 },
                    location2{ loc2 }
            {
            }
            
            //  Constructs a increment instruction. When encountered, reg is
            //  incremented by 1 and the program jumps to the line specified by
            //  loc1. Syntax: "L{line}: R{reg}+ -> L{loc1}"
            constexpr instruction(std::size_t reg, std::size_t loc1) :
                    type{ INCR },
                    currentRegister{ reg },
                    location1{ loc1 },
                    location2{ std::numeric_limits<std::size_t>::max() }
            {
            }
            
            // Constructs a halt instruction. When encountered, the program
            // will terminate properly. Syntax: "{line}: HALT"
            constexpr instruction() :
                    type{ HALT },
                    currentRegister{ 0 },
                    location1{ std::numeric_limits<std::size_t>::max() },
                    location2{ std::numeric_limits<std::size_t>::max() }
            {
            }
            
            constexpr instruction(const instruction&) = default;
            constexpr instruction& operator=(const instruction&) = default;
        };
    }
    
    template<std::size_t maxRegisters, std::size_t instrCount>
    struct program
    {
        inline static constexpr std::size_t instructionCount{ instrCount };
        const std::array<impl::instruction, instrCount> instructions;
        
        [[maybe_unused]]
        explicit constexpr program(std::array<impl::instruction, instrCount> ins) :
                instructions{ std::move(ins) }
        {
        }
        
        [[maybe_unused]]
        explicit constexpr program(const program<maxRegisters, instrCount>&) = default;
        
        //  Executes a register machine program with an initial configuration
        //  with the function arguments specifying the initial value of the
        //  first N registers and any additional registers being initialised
        //  to zero. The type template argument specifies the data type to be
        //  used for the registers. Returns the value in the first register.
        template<std::unsigned_integral IntType = std::size_t, typename... Args>
        requires ((sizeof...(Args) <= maxRegisters) && ... && std::convertible_to<Args, IntType>)
        [[maybe_unused]] [[nodiscard]]
        consteval IntType exec(Args... args) const
        {
            std::array<IntType, maxRegisters> values{ static_cast<IntType>(args)... };
            
            bool halt{ false };
            std::size_t loc{ 0 };
            
            while (!halt)
            {
                const auto& current{ instructions[loc] };
                
                if (current.type == impl::HALT)
                {
                    halt = true;
                }
                else if (current.type == impl::INCR)
                {
                    ++values[current.currentRegister];
                    loc = current.location1;
                }
                else if (current.type == impl::DECR)
                {
                    if (values[current.currentRegister] > 0)
                    {
                        --values[current.currentRegister];
                        loc = current.location1;
                    }
                    else
                    {
                        loc = current.location2;
                    }
                }
                
                if (loc > instrCount)
                {
                    halt = true;
                }
            }
            return values[0];
        }
    };
    
    //  Non constant expression function which is called by compile-time
    //  functions to generate compile errors when syntax errors are found
    //  in a register machine programs.
    template<typename T>
    void error(T)
    {
        static_assert(true, "Error");
    }
    
    template<std::size_t instrCount>
    class parser
    {
    private:
        const std::string_view input;
        const std::size_t end;
        
        [[nodiscard]]
        constexpr bool eof(std::size_t& current) const
        {
            return current >= end;
        }
        
        template<typename... Ts>
        requires(std::convertible_to<Ts, char>&& ...)
        [[nodiscard]]
        constexpr bool isChar(std::size_t& current, Ts... cs) const
        {
            return ((input[current] == cs) || ...);
        }
        
        template<typename... Ts>
        requires(std::convertible_to<Ts, char>&& ...)
        [[nodiscard]]
        constexpr bool matchChar(std::size_t& current, Ts... cs) const
        {
            if (isChar(current, cs...))
            {
                ++current;
                return true;
            }
            
            return false;
        }
        
        template<std::size_t N>
        [[nodiscard]]
        constexpr bool matchStr(std::size_t& current, const char (& str)[N]) const
        {
            bool success{ true };
            
            for (std::size_t i{ 0 }; i < N - 1; ++i)
                if (current + i == end || input[current + i] != str[i])
                    success = false;
            
            if (success)
                current += N - 1;
            
            return success;
        }
        
        [[nodiscard]]
        constexpr std::size_t parseInt(std::size_t& current) const
        {
            std::size_t result{ 0 };
            bool noIntParsed{ true };
            while (true)
            {
                if (eof(current))
                {
                    if (noIntParsed)
                        error("Error: no number specified");
                    return result;
                }
                
                if (isChar(current, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'))
                {
                    result *= 10;
                    result += input[current] - '0';
                    ++current;
                    noIntParsed = false;
                }
                else
                {
                    if (noIntParsed)
                        error("Error: no number specified");
                    return result;
                }
            }
        }
        
        [[nodiscard]]
        constexpr std::size_t parseLineNumber(std::size_t& current) const
        {
            if (eof(current))
                error("Error: encountered unexpected end of file");
            
            if (!matchChar(current, 'L'))
                error("Syntax Error: expected line number but got something else");
            
            return parseInt(current);
        }
        
        constexpr void skipLinesAndSpaces(std::size_t& current) const
        {
            while (!eof(current) && matchChar(current, '\n', '\t', ' '));
        }
    
        constexpr void skipSpaces(std::size_t& current) const
        {
            while (!eof(current) && matchChar(current, '\t', ' '));
        }
    
        constexpr void parseEOL(std::size_t& current) const
        {
            skipSpaces(current);
            
            if (!eof(current) && !matchChar(current, ';', '\n', '\0'))
                error("Syntax Error: Missing statement terminator (';' or '\\n' or '\0");
        }
    
    public:
        explicit constexpr parser(std::string_view sv) :
                input{ sv },
                end{ input.size() }
        {
        }
        
        template<std::size_t maxRegisters>
        [[nodiscard]]
        consteval program<maxRegisters, instrCount> parse() const
        {
            std::array<impl::instruction, instrCount> instr;
            
            std::size_t current{ 0 };
            
            skipLinesAndSpaces(current);
            
            for (std::size_t i{ 0 }; i < instrCount && !eof(current); ++i)
            {
                if (isChar(current, 'L'))
                {
                    if (parseLineNumber(current) != i)
                        error("Program Error: line number specified in program is incorrect");
                    
                    skipSpaces(current);
                    
                    if (eof(current) || !matchChar(current, ':'))
                        error("Syntax Error: expected ':' but got something else instead");
                    
                    skipSpaces(current);
                }
                
                if (!eof(current) && matchChar(current, 'R'))
                {
                    std::size_t registerLocation{ parseInt(current) };
                    bool incrementInstruction;
                    
                    skipSpaces(current);
                    
                    switch (input[current])
                    {
                    case '+':
                        incrementInstruction = true;
                        break;
                    case '-':
                        incrementInstruction = false;
                        break;
                    default:
                        error("Syntax Error: expected either '+' or '-' but got neither");
                    }
                    ++current;
                    skipSpaces(current);
                    
                    if (!matchStr(current, "->"))
                        error("Syntax Error: expected \"->\" but got something else instead");
                    
                    skipSpaces(current);
                    
                    std::size_t loc1{ parseLineNumber(current) };
                    
                    if (incrementInstruction)
                    {
                        parseEOL(current);
                        instr[i] = impl::instruction{ registerLocation, loc1 };
                    }
                    else
                    {
                        skipSpaces(current);
                        
                        if (!matchChar(current, ','))
                            error("Syntax Error: expected ',' but got something else instead");
                        
                        skipSpaces(current);
                        std::size_t loc2{ parseLineNumber(current) };
                        parseEOL(current);
                        instr[i] = impl::instruction{ registerLocation, loc1, loc2 };
                    }
                }
                else if (matchStr(current, "HALT"))
                {
                    parseEOL(current);
                    instr[i] = impl::instruction{};
                }
                else if (matchChar(current, ';', '\n', '\0'))
                {
                    instr[i] = impl::instruction{};
                }
                else
                {
                    error("Error: encountered unexpected characters");
                }
                
                skipLinesAndSpaces(current);
            }
            
            return program<maxRegisters, instrCount>(instr);
        }
    };
    
    //  Creates a program from a string at compile time.
    template<std::size_t maxRegisters, std::size_t instrCount>
    [[maybe_unused]] [[nodiscard]]
    consteval program<maxRegisters, instrCount> make(std::string_view input)
    {
        const parser<instrCount> p{ input };
        return p.template parse<maxRegisters>();
    }
    
    namespace impl
    {
        //  Identifies the largest location and largest register present within
        //  a program at compile time. These can the be used as the template
        //  arguments for program.
        template<char... Input>
        consteval std::pair<std::size_t, std::size_t> getMaxArgs()
        {
            enum class mode
            {
                SKIP, READ_LOC, READ_REG
            };
            
            std::array<char, sizeof...(Input)> chars{ Input... };
            mode readInt{ mode::SKIP };
            std::size_t result{ 0 };
            std::size_t maxReg{ 0 };
            std::size_t maxLoc{ 0 };
            
            for (const char& c : chars)
            {
                switch (c)
                {
                case 'L':
                    readInt = mode::READ_LOC;
                    break;
                case 'R':
                    readInt = mode::READ_REG;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    if (readInt != mode::SKIP)
                    {
                        result *= 10;
                        result += c - '0';
                    }
                    break;
                default:
                    if (result != 0)
                    {
                        if (readInt == mode::READ_LOC)
                            if (result > maxLoc)
                                maxLoc = result;
                        
                        if (readInt == mode::READ_REG)
                            if (result > maxReg)
                                maxReg = result;
                        
                        result = 0;
                        readInt = mode::SKIP;
                    }
                }
            }
            
            if (readInt == mode::READ_LOC)
                if (result > maxLoc)
                    maxLoc = result;
            
            if (readInt == mode::READ_REG)
                if (result > maxReg)
                    maxReg = result;
            
            return { maxReg + 1, maxLoc + 1 };
        }
    }
    
    //  Creates a program from a string at compile time, with the number of
    //  registers and instructions calculated by this function.
    template<char... Cs>
    consteval auto generate()
    {
        constexpr auto maxArgs{ impl::getMaxArgs<Cs...>() };
        constexpr std::array<char, sizeof...(Cs) + 1> string{ Cs..., '\0' };
        return ctrm::make<maxArgs.first, maxArgs.second>(string.data());
    }
    
    namespace literals
    {
#if (__GNUG__ || __clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
        
        //  Creates a program from a string at compile time. Wrapper for
        //  ctrm::generate(). Requires string literal operator templates.
        template<typename T, T... cs>
        consteval auto operator ""_ctrm()
        {
            return generate<cs...>();
        }

#pragma clang diagnostic pop
#endif //  __GNUG__
    }
}

#endif //  COMPILE_TIME_REGISTER_MACHINE_HPP