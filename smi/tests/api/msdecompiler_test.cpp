#include <gtest/gtest.h>

#include <string>

#include "core/lexer.h"
#include "readfile.h"
#include "smi.h"

using namespace smi::lexer;

TEST(MSDecompilerTest, MSDecompilerTest1) {
    std::string file = readFile("files/file1.MS");

    SMIMSDecompiler* decomp = smi_msdecompiler_new();
    const char* decompiled = smi_msdecompiler_decompile(decomp, file.c_str(), file.length());
    smi_msdecompiler_destroy(decomp);

    std::string code = decompiled;
    free((char*)decompiled);

    Tokens tokens1, tokens2;

    EXPECT_EQ(tokenize(code, tokens1), LEXER_OK);
    EXPECT_EQ(tokenize(readFile("files/file1.txt"), tokens2), LEXER_OK);
    ASSERT_EQ(tokens1.size(), tokens2.size());

    std::vector<TokenType> token_types_file1, token_types_file2;
    std::vector<std::string> token_result_file1, token_result_file2;

    for (int i = 0; i < tokens1.size(); i++) {
        token_types_file1.push_back(tokens1[i].type);

        if (tokens1[i].type == TokenType::LITERAL) {
            token_result_file1.push_back(std::to_string(std::stoi(tokens1[i].value, nullptr, 16)));
        } else {
            token_result_file1.push_back(tokens1[i].value);
        }

        token_types_file2.push_back(tokens2[i].type);

        if (tokens2[i].type == TokenType::LITERAL) {
            token_result_file2.push_back(std::to_string(std::stoi(tokens2[i].value, nullptr, 16)));
        } else {
            token_result_file2.push_back(tokens2[i].value);
        }
    }

    EXPECT_EQ(token_types_file1, token_types_file2);
    EXPECT_EQ(token_result_file1, token_result_file2);
}
