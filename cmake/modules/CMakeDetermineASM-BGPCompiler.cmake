# determine the compiler to use for ASM using BGP syntax

SET(ASM_DIALECT "-BGP")
SET(CMAKE_ASM${ASM_DIALECT}_COMPILER_INIT ${CCAS})
INCLUDE(CMakeDetermineASMCompiler)
SET(ASM_DIALECT)
