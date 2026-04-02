#include "core/lexer.h"

#include <gtest/gtest.h>

#include "readfile.h"

using namespace smi::lexer;

TEST(LexerTest, LexerTest1) {
    std::vector<TokenType> expected_types_result = {
        TokenType::INSTR, TokenType::OP, TokenType::COMMA, TokenType::OP,
        TokenType::LABEL,
        TokenType::INSTR, TokenType::OP, TokenType::COMMA, TokenType::OP,
        TokenType::INSTR, TokenType::OP,
        TokenType::INSTR, TokenType::OP, TokenType::COMMA, TokenType::OP,
        TokenType::INSTR, TokenType::OP,
        TokenType::INSTR, TokenType::OP, TokenType::COMMA, TokenType::OP,
        TokenType::INSTR, TokenType::OP, TokenType::COMMA, TokenType::OP,
        TokenType::INSTR, TokenType::OP,
        TokenType::LABEL,
        TokenType::INSTR, TokenType::OP, TokenType::COMMA, TokenType::OP,
        TokenType::LABEL, TokenType::LITERAL,
        TokenType::LABEL, TokenType::LITERAL,
        TokenType::LABEL, TokenType::LITERAL,
        TokenType::LABEL, TokenType::LITERAL,
        TokenType::LABEL, TokenType::LITERAL,
        TokenType::LABEL, TokenType::LITERAL,
        TokenType::_EOF,
    };

    std::vector<std::string> expected_values_result = {
        "MOV", "ZERO", ",",    "I",   "LOOP", "CMP", "NUM1", ",", "I",   "BEQ", "END", "CMP",  "NUM2", ",",    "I",
        "BEQ", "END",  "ADD",  "ONE", ",",    "I",   "CMP",  "I", ",",   "I",   "BEQ", "LOOP", "END",  "MOV",  "I",
        ",",   "MIN",  "NUM1", "3",   "NUM2", "5",   "ZERO", "0", "ONE", "1",   "I",   "FFFF", "MIN",  "FFFF", "",
    };

    Tokens tokens;

    int res = tokenize(readFile("files/file1.txt"), tokens);

    EXPECT_EQ(res, LEXER_OK);

    std::vector<TokenType> types_result;
    std::vector<std::string> values_result;

    for (const Token& token : tokens) {
        types_result.push_back(token.type);
        values_result.push_back(token.value);
    }

    EXPECT_EQ(types_result, expected_types_result);
    EXPECT_EQ(values_result, expected_values_result);
}
