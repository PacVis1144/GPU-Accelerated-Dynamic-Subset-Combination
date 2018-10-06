//========================================================================================
//  Copyright (c) 2018  Pacific Vis Paper Author Submission 1144  -- All rights reserved.
//  See LICENSE.md for details.
//========================================================================================


#ifndef COMBINATION_SHADER_GENERATOR_HPP
#define COMBINATION_SHADER_GENERATOR_HPP

#include <string>
#include <set>
#include <array>
#include <algorithm>
#include <iostream>

namespace TN
{

const std::string CUDA_COMBINATION_KERNEL_TEMPLATE =
    "extern \"C\" __global__                                       \n"
    "void setCombination( int * b, size_t N )                      \n"
    "{                                                             \n"
    "    const size_t IDX = blockIdx.x * blockDim.x + threadIdx.x; \n"
    "    const int COMB_MASK = 2;                                  \n"
    "    if( IDX >= N )                                            \n"
    "    {                                                         \n"
    "        return;                                               \n"
    "    }                                                         \n"
    "    const int x = b[ IDX ] & ~COMB_MASK;                      \n"
    "    const int c = b[ IDX ];                                   \n"
    "    b[ IDX ] = x | int( @ ) * COMB_MASK;                      \n"
    "}";

static const std::set< char > VALID_SUBSET_EXPRESSION_TEXT =
{
    '$', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '(', ')', '|', '&', '-','^', '!', ' '
};

const std::set< char > SET_IDS =
{
    '$', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
    'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

inline std::string getOperand( std::string & expr, int pos, const std::string & dir )
{
    const char OPEN = dir == "left" ? ')' : '(';
    const char CLSE = dir == "left" ? '(' : ')';
    const int  STEP = dir == "left" ? -1  :  1;
    const int  END = expr.size();

    std::string operand;
    int level = 0;

    for( int i = pos + STEP; i >= 0 && i < END; i += STEP )
    {
        operand.push_back( expr[ i ] );
        level += int( expr[ i ] == OPEN );
        level -= int( expr[ i ] == CLSE );
        if( ( expr[ i ] == CLSE && level == 0 )
         || ( SET_IDS.count( expr[ i ] ) && level == 0  ) )
        {
            break;
        }
    }

    if( dir == "left" )
    {
        std::reverse( operand.begin(), operand.end() );
    }

    return operand;
}

inline void convertOperation( std::string & expr, char op, std::string tmplt )
{   
    bool hasLeftOprnd  = tmplt.find( 'l' ) != std::string::npos;
    bool hasRightOprnd = tmplt.find( 'r' ) != std::string::npos;

    for( size_t i = 0; i < expr.size(); ++i )
    {
        if( expr[ i ] == op )
        {
            std::string leftOperand  = hasLeftOprnd  ? getOperand( expr, i, "left" )  : "";
            std::string rightOperand = hasRightOprnd ? getOperand( expr, i, "right" ) : "";
            std::string replacement = tmplt;

            int j;

            if( hasLeftOprnd )
            {
                while ( ( j = replacement.find( 'l' ) ) != std::string::npos )
                {
                    replacement.replace( j, 1, leftOperand );
                }
            }
            if( hasRightOprnd )
            {
                while ( ( j = replacement.find( 'r' ) ) != std::string::npos )
                {
                    replacement.replace( j, 1, rightOperand );
                }
            }

            expr.replace(
                expr.begin() + i -  leftOperand.size(),
                expr.begin() + i + rightOperand.size() + 1,
                replacement );

            i = i - leftOperand.size() + replacement.size();
        }
    }
}

inline std::string generateCombinationProgram( std::string expr, const std::string & TEMPL )
{
    const std::map< char, char > placeholderMap     = { { '!', 'n' }, { '&', 'a' }, { '|', 'o' }, { '-', 'm' }, { '^', 'x' } };
    const std::array< char, 5 > orderOfOperation    =  { 'n', 'a', 'o', 'm', 'x' };
    const std::map< char, std::string > templateMap = { { 'n', "(!r)" }, { 'a', "(l&&r)" }, { 'o', "(l||r)" }, { 'm', "(l&&!r)" }, { 'x', "(int(l)-int(r))"} };

    for( auto & c : expr )
    {
        if( placeholderMap.count( c ) )
        {
            c = placeholderMap.at( c );
        }
    }

    for( auto & op : orderOfOperation )
    {
        while( expr.find( op ) != std::string::npos  )
        {
            convertOperation( expr, op, templateMap.at( op ) );
        }
    }

    for( size_t i = 0; i < expr.size(); ++i )
    {
        if( SET_IDS.count( expr[ i ] ) )
        {
            std::string replacement;
            if( expr[ i ] == '$' )
            {
                replacement =  "bool(c&1)";
            }
            else
            {
                replacement = "bool(c&" + std::to_string( int( std::pow( 2, expr[ i ] - 'A' + 4 ) ) ) + ")";
            }

            expr.erase( expr.begin() + i );
            expr.insert( expr.begin() + i, replacement.begin(), replacement.end() );
        }
    }

    std::string vertexShaderCode = TEMPL;
    vertexShaderCode.replace( vertexShaderCode.find( '@' ), 1, expr );

    return vertexShaderCode;
}

}
#endif // COMBINATION_SHADER_GENERATOR_HPP
