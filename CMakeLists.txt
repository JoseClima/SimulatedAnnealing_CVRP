cmake_minimum_required(VERSION 3.16)
project(CVRP_SA LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Pasta de testes: obrigatoria. Aceita "A", "/A" ou "tests/A" ----
set(TESTS_DIR "" CACHE STRING "Nome da pasta de testes (ex: A, B).")

if(TESTS_DIR STREQUAL "")
    message(FATAL_ERROR "ERRO: TESTS_DIR nao especificado. Veja README.")
endif()

string(REGEX REPLACE "^/" "" _TESTS_DIR "${TESTS_DIR}")
if(NOT _TESTS_DIR MATCHES "^instances/")
    set(_TESTS_DIR "instances/${_TESTS_DIR}")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${_TESTS_DIR}")
    message(FATAL_ERROR "ERRO: pasta '${_TESTS_DIR}' nao encontrada. Veja README.")
endif()

message(STATUS "Pasta de testes: ${_TESTS_DIR}")

set(TESTS_ROOT "instances" CACHE STRING "Pasta raiz para run-*-folders")

# ---- Biblioteca ----
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_library(cvrp_core STATIC ${CORE_SOURCES})
target_compile_options(cvrp_core PRIVATE -O2)

# ---- Executavel ----
add_executable(vrp_teste app/Main.cpp)
target_link_libraries(vrp_teste PRIVATE cvrp_core)
target_compile_options(vrp_teste PRIVATE -O2)

# ---- .vrp da pasta escolhida ----
file(GLOB_RECURSE VRP_A CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${_TESTS_DIR}/*.vrp")
list(SORT VRP_A)
list(LENGTH VRP_A VRP_A_COUNT)
message(STATUS "Encontrados ${VRP_A_COUNT} arquivos .vrp em ${_TESTS_DIR}")

# ---- .vrp de todos os subdiretorios de TESTS_ROOT ----
file(GLOB_RECURSE VRP_ALL CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/${TESTS_ROOT}/*.vrp")
list(SORT VRP_ALL)

# ---- Funcao auxiliar ----
function(add_batch_run target_name list_var flag)
    set(cmds "")
    foreach(f IN LISTS ${list_var})
        list(APPEND cmds COMMAND ${CMAKE_COMMAND} -E echo ">>> ${f}")
        if(flag)
            list(APPEND cmds COMMAND $<TARGET_FILE:vrp_teste> "${f}" ${flag})
        else()
            list(APPEND cmds COMMAND $<TARGET_FILE:vrp_teste> "${f}")
        endif()
    endforeach()
    if(NOT cmds)
        list(APPEND cmds COMMAND ${CMAKE_COMMAND} -E echo "(nenhum .vrp para rodar)")
    endif()
    add_custom_target(${target_name}
        ${cmds}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS vrp_teste
        VERBATIM)
endfunction()

add_batch_run(run-all           VRP_A   "")
add_batch_run(run-stats         VRP_A   "--statistics")
add_batch_run(run-all-folders   VRP_ALL "")
add_batch_run(run-stats-folders VRP_ALL "--statistics")

# ---- targets ----
add_custom_target(targets
    COMMAND ${CMAKE_COMMAND} -E echo "Pasta atual: ${_TESTS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Alvos: run-all, run-stats, run-all-folders, run-stats-folders, clean"
    VERBATIM)

# ---- Copia tests/ para build/ ----
add_custom_command(TARGET vrp_teste POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/${TESTS_ROOT}
        $<TARGET_FILE_DIR:vrp_teste>/${TESTS_ROOT}
)