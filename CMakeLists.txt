cmake_minimum_required(VERSION 3.10)
project(KashShell VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Provide the paths to the Readline include and library directories
set(READLINE_INCLUDE_DIR "/usr/local/opt/readline/include")
set(READLINE_LIBRARY_DIR "/usr/local/opt/readline/lib")
set(READLINE_LIBRARY "/usr/local/opt/readline/lib/libreadline.a") # Explicitly specify the library file

# Ensure that CMake uses the library path we provided for linking
set(CMAKE_EXE_LINKER_FLAGS "-L${READLINE_LIBRARY_DIR} -lreadline")

# Add the executable with all the source files
add_executable(kash kash.cpp AST.cpp parse_commands.cpp)

# Specify include directories for compiling
target_include_directories(kash PRIVATE ${READLINE_INCLUDE_DIR})

# Link the Readline library statically
target_link_libraries(kash PRIVATE ${READLINE_LIBRARY})
