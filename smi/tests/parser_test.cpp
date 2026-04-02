#include "core/parser.h"

#include <gtest/gtest.h>

#include "readfile.h"

using namespace smi::parser;

void expectEq(Program* result, Program* expected) {
    ASSERT_EQ(result->getBody().size(), expected->getBody().size());

    for (int i = 0; i < result->getBody().size(); i++) {
        EXPECT_TRUE(*(result->getBody()[i]) == *(expected->getBody()[i]));
    }
}

static Program expected_program(
    {
        new Instruction("MOV", new Identifier("ZERO", 0, 0, 0), new Identifier("I", 0, 0, 0), 0, 0, 0),
        new Label("LOOP", 0, 0, 0),
        new Instruction("CMP", new Identifier("NUM1", 0, 0, 0), new Identifier("I", 0, 0, 0), 0, 0, 0),
        new Instruction("BEQ", new Identifier("END", 0, 0, 0), nullptr, 0, 0, 0),
        new Instruction("CMP", new Identifier("NUM2", 0, 0, 0), new Identifier("I", 0, 0, 0), 0, 0, 0),
        new Instruction("BEQ", new Identifier("END", 0, 0, 0), nullptr, 0, 0, 0),
        new Instruction("ADD", new Identifier("ONE", 0, 0, 0), new Identifier("I", 0, 0, 0), 0, 0, 0),
        new Instruction("CMP", new Identifier("I", 0, 0, 0), new Identifier("I", 0, 0, 0), 0, 0, 0),
        new Instruction("BEQ", new Identifier("LOOP", 0, 0, 0), nullptr, 0, 0, 0),
        new Label("END", 0, 0, 0),
        new Instruction("MOV", new Identifier("I", 0, 0, 0), new Identifier("MIN", 0, 0, 0), 0, 0, 0),
        new Assignment("NUM1", "3", 0, 0, 0),
        new Assignment("NUM2", "5", 0, 0, 0),
        new Assignment("ZERO", "0", 0, 0, 0),
        new Assignment("ONE", "1", 0, 0, 0),
        new Assignment("I", "FFFF", 0, 0, 0),
        new Assignment("MIN", "FFFF", 0, 0, 0),
    },
    0, 0, 0);

TEST(ParserTest, ParserTest1) {
    smi::lexer::Tokens tokens;
    smi::lexer::tokenize(readFile("files/file1.txt"), tokens);

    std::unique_ptr<Program> program;
    int res = parse(tokens, program);

    EXPECT_EQ(res, PARSER_OK);

    expectEq(program.get(), &expected_program);
}
