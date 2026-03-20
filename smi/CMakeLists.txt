cmake_minimum_required(VERSION 3.10)

if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(SimpleMachineInterpreter)

set(LIBSMI_SOURCES
    "src/core/lexer.h"
    "src/core/lexer.cpp"
    "src/core/parser.h"
    "src/core/parser.cpp"
    "src/core/ast.h"
    "src/core/interpreter.cpp"
    "src/core/interpreter.h"
    "src/core/debugger.cpp"
    "src/core/debugger.h"
    "src/core/smierror.h"
    "src/core/smierror.cpp"
    "src/core/ms/mstypes.h"
    "src/core/ms/decompiler.h"
    "src/core/ms/decompiler.cpp"
    "src/core/ms/compiler.h"
    "src/core/ms/compiler.cpp"
    "src/core/ms/vm.h"
    "src/core/ms/vm.cpp"
)

set(EXPORTED_FUNCTIONS
    "_smi_interpreter_new"
    "_smi_interpreter_destroy"
    "_smi_interpreter_eval"
    "_smi_interpreter_memory_keys_get"
    "_smi_interpreter_memory_keys_free"
    "_smi_interpreter_memory_value_get"
    "_smi_interpreter_executed_instructions_get"
    "_smi_last_error_data_get"

    "_smi_debugger_new"
    "_smi_debugger_destroy"
    "_smi_debugger_load"
    "_smi_debugger_next"
    "_smi_debugger_next_index_get"
    "_smi_debugger_next_line_get"
    "_smi_debugger_has_next"
    "_smi_debugger_cmp_get"
    "_smi_debugger_as_interpreter"
    
    "_smi_msdecompiler_new"
    "_smi_msdecompiler_destroy"
    "_smi_msdecompiler_decompile"
    "_smi_mscompiler_new"
    "_smi_mscompiler_destroy"
    "_smi_mscompiler_compile"
    "_smi_mscopiler_linemap_get"

    "_smi_msvm_new"
    "_smi_msvm_destroy"
    "_smi_msvm_loadms"
    "_smi_msvm_execute_next"
    "_smi_msvm_pc_get"
    "_smi_msvm_ir_get"
    "_smi_msvm_zf_get"
    "_smi_msvm_memory_get"
    "_smi_msvm_memory_value_get"
    "_smi_msvm_labels_get"
    "_smi_msvm_labels_free"
    "_smi_msvm_label_get"

    "_malloc"
    "_free"
)

list(JOIN EXPORTED_FUNCTIONS "," EXPORTED_FUNCTIONS_STR)

if (NOT EMSCRIPTEN)
    add_executable(smi
        "src/main.cpp"
        ${LIBSMI_SOURCES}
    )

    target_include_directories(smi
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    if (CMAKE_VERSION VERSION_GREATER 3.12)
        set_property(TARGET smi PROPERTY CXX_STANDARD 20)
    endif()

    if (MSVC)
        set_property(TARGET smi PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    else()
        target_link_options(smi PRIVATE
            -static-libgcc
            -static-libstdc++
        )
    endif()

    install(TARGETS smi RUNTIME DESTINATION bin)
else()
    add_executable(smiwasmout "src/api/smi.cpp" "src/api/smi_cast.cpp" ${LIBSMI_SOURCES})

    target_include_directories(smiwasmout
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_options(smiwasmout
        PUBLIC -sEXPORTED_FUNCTIONS=[${EXPORTED_FUNCTIONS_STR}]
        PUBLIC -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAP8','HEAPU16','getValue','UTF8ToString']
        PUBLIC -sMODULARIZE=1
        PUBLIC -sEXPORT_ES6=1
        PUBLIC -lembind
    )
endif()

# TODO: Add tests.
