cmake_minimum_required(VERSION 3.8)

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
    "src/core/smierror.h"
    "src/core/smierror.cpp"
)

if (NOT EMSCRIPTEN)
    add_executable(smi
        "src/main.cpp"
        ${LIBSMI_SOURCES}
    )

    if (CMAKE_VERSION VERSION_GREATER 3.12)
        set_property(TARGET smi PROPERTY CXX_STANDARD 20)
    endif()

    install(TARGETS smi RUNTIME DESTINATION bin)
else()
    add_executable(smiwasmout "src/api/smi.cpp" ${LIBSMI_SOURCES})

    target_include_directories(smiwasmout
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_options(smiwasmout
        PUBLIC -sEXPORTED_FUNCTIONS=['_smi_new','_smi_destroy','_smi_eval','_smi_memory_keys_get','_smi_memory_keys_free','_smi_memory_value_get','_smi_last_error_get']
        PUBLIC -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','HEAP32','getValue','UTF8ToString']
        PUBLIC -sMODULARIZE=1
        PUBLIC -sEXPORT_ES6=1
        PUBLIC -lembind
    )
endif()

# TODO: Agregue pruebas y destinos de instalación si es necesario.
