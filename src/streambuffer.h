/*******************************************************************************
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************/

#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H

#define CHUNK_LIMIT 1048576

#ifdef USE_ZLIB
#include <zlib.h>
typedef size_t(*fread_op_zip)(void*, size_t, size_t, gzFile);
#endif
#include <stdio.h>
#include <iostream>
#include <limits>
#include <string>

//A = gzFile, FILE
//B = fread, gz_read
typedef size_t(*fread_op_norm)(void*, size_t, size_t, FILE*);

template<typename A, typename B, B C>
class StreamBuffer
{
    A  in;
    void assureLookahead() {
        if (pos >= size) {
            pos  = 0;
            size = C(buf, 1, sizeof(buf), in);
        }
    }
    char    buf[CHUNK_LIMIT];
    int     pos;
    int     size;

    void advance()
    {
        operator++();
    }
    int value()
    {
        return operator*();
    }

public:
    StreamBuffer(A i) : in(i), pos(0), size(0) {
        assureLookahead();
    }

    int  operator *  () {
        return (pos >= size) ? EOF : buf[pos];
    }
    void operator ++ () {
        pos++;
        assureLookahead();
    }

    void skipWhitespace()
    {
        char c = value();
        while ((c >= 9 && c <= 13 && c != 10) || c == 32) {
            advance();
            c = value();
        }
    }

    void skipLine()
    {
        for (;;) {
            if (value() == EOF || value() == '\0') return;
            if (value() == '\n') {
                advance();
                return;
            }
            advance();
        }
    }

    int32_t parseInt(size_t lineNum, bool allow_eol = false)
    {
        int32_t val = 0;
        int32_t mult = 1;
        skipWhitespace();
        if (value() == '-') {
            mult = -1;
            advance();
        } else if (value() == '+') {
            advance();
        }

        char c = value();
        if (allow_eol && c == '\n')
            return std::numeric_limits<int32_t>::max();
        if (c < '0' || c > '9') {
            std::cout
            << "PARSE ERROR! Unexpected char (dec: '" << c << ")"
            << " At line " << lineNum
            << " we expected a number"
            << std::endl;
            std::exit(3);
        }

        while (c >= '0' && c <= '9') {
            val = val*10 + (c - '0');
            advance();
            c = value();
        }
        return mult*val;
    }

    void parseString(std::string& str)
    {
        str.clear();
        skipWhitespace();
        while (value() != ' ' && value() != '\n' && value() != EOF) {
            str.push_back(value());
            advance();
        }
    }
};

#endif //STREAMBUFFER_H
