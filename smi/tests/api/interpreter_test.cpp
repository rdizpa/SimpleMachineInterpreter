#include <gtest/gtest.h>

#include "readfile.h"
#include "smi.h"

TEST(InterpreterTest, InterpreterTest1) {
    SMIInterpreter* interp = smi_interpreter_new();

    std::string code = readFile("files/file1.txt");
    int res = smi_interpreter_eval(interp, code.c_str());

    EXPECT_EQ(res, SMI_OK);

    const std::vector<std::string> expected_keys = {"NUM1", "NUM2", "ZERO", "ONE", "I", "MIN"};
    std::vector<std::string> keys_result;

    const char** memory_keys = smi_interpreter_memory_keys_get(interp);
    const char** memory_keys_it = memory_keys;

    while (*memory_keys_it != NULL) {
        keys_result.push_back(*memory_keys_it);
        memory_keys_it++;
    }

    smi_interpreter_memory_keys_free(memory_keys);

    std::unordered_map<std::string, uint16_t> expected_values = {
        {"NUM1", 3}, {"NUM2", 5}, {"ZERO", 0}, {"ONE", 1}, {"I", 3}, {"MIN", 3},
    };

    EXPECT_EQ(keys_result, expected_keys);

    for (const std::string& key : expected_keys) {
        EXPECT_EQ(smi_interpreter_memory_value_get(interp, key.c_str()), expected_values[key]);
    }

    smi_interpreter_destroy(interp);
}

TEST(InterpreterTest, InterpreterTest2) {
    SMIInterpreter* interp = smi_interpreter_new();

    std::string code = readFile("files/file2.txt");
    int res = smi_interpreter_eval(interp, code.c_str());

    EXPECT_EQ(res, SMI_OK);

    const std::vector<std::string> expected_keys = {"ONE", "FIVE", "INDEX"};
    std::vector<std::string> keys_result;

    const char** memory_keys = smi_interpreter_memory_keys_get(interp);
    const char** memory_keys_it = memory_keys;

    while (*memory_keys_it != NULL) {
        keys_result.push_back(*memory_keys_it);
        memory_keys_it++;
    }

    smi_interpreter_memory_keys_free(memory_keys);

    std::unordered_map<std::string, uint16_t> expected_values = {
        {"ONE", 1},
        {"FIVE", 5},
        {"INDEX", 5},
    };

    EXPECT_EQ(keys_result, expected_keys);

    for (const std::string& key : expected_keys) {
        EXPECT_EQ(smi_interpreter_memory_value_get(interp, key.c_str()), expected_values[key]);
    }

    smi_interpreter_destroy(interp);
}
