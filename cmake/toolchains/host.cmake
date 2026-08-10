# CMake toolchain for the host machine (macOS / Linux native gcc or clang).
#
# Used by the "host" board to build the same application against mock drivers
# for unit testing on the developer's machine. Deliberately minimal: leaves
# CMAKE_C_COMPILER unset so CMake picks whatever's in PATH.
