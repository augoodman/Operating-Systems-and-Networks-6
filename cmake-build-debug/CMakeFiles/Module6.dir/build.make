# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/124/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/124/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/andrew/CLionProjects/Module6

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/andrew/CLionProjects/Module6/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Module6.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Module6.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Module6.dir/flags.make

CMakeFiles/Module6.dir/BaseFilters.c.o: CMakeFiles/Module6.dir/flags.make
CMakeFiles/Module6.dir/BaseFilters.c.o: ../BaseFilters.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/andrew/CLionProjects/Module6/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Module6.dir/BaseFilters.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Module6.dir/BaseFilters.c.o   -c /home/andrew/CLionProjects/Module6/BaseFilters.c

CMakeFiles/Module6.dir/BaseFilters.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Module6.dir/BaseFilters.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/andrew/CLionProjects/Module6/BaseFilters.c > CMakeFiles/Module6.dir/BaseFilters.c.i

CMakeFiles/Module6.dir/BaseFilters.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Module6.dir/BaseFilters.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/andrew/CLionProjects/Module6/BaseFilters.c -o CMakeFiles/Module6.dir/BaseFilters.c.s

CMakeFiles/Module6.dir/GoodmanFilters.c.o: CMakeFiles/Module6.dir/flags.make
CMakeFiles/Module6.dir/GoodmanFilters.c.o: ../GoodmanFilters.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/andrew/CLionProjects/Module6/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/Module6.dir/GoodmanFilters.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Module6.dir/GoodmanFilters.c.o   -c /home/andrew/CLionProjects/Module6/GoodmanFilters.c

CMakeFiles/Module6.dir/GoodmanFilters.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Module6.dir/GoodmanFilters.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/andrew/CLionProjects/Module6/GoodmanFilters.c > CMakeFiles/Module6.dir/GoodmanFilters.c.i

CMakeFiles/Module6.dir/GoodmanFilters.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Module6.dir/GoodmanFilters.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/andrew/CLionProjects/Module6/GoodmanFilters.c -o CMakeFiles/Module6.dir/GoodmanFilters.c.s

# Object files for target Module6
Module6_OBJECTS = \
"CMakeFiles/Module6.dir/BaseFilters.c.o" \
"CMakeFiles/Module6.dir/GoodmanFilters.c.o"

# External object files for target Module6
Module6_EXTERNAL_OBJECTS =

Module6: CMakeFiles/Module6.dir/BaseFilters.c.o
Module6: CMakeFiles/Module6.dir/GoodmanFilters.c.o
Module6: CMakeFiles/Module6.dir/build.make
Module6: CMakeFiles/Module6.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/andrew/CLionProjects/Module6/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable Module6"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Module6.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Module6.dir/build: Module6

.PHONY : CMakeFiles/Module6.dir/build

CMakeFiles/Module6.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Module6.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Module6.dir/clean

CMakeFiles/Module6.dir/depend:
	cd /home/andrew/CLionProjects/Module6/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/andrew/CLionProjects/Module6 /home/andrew/CLionProjects/Module6 /home/andrew/CLionProjects/Module6/cmake-build-debug /home/andrew/CLionProjects/Module6/cmake-build-debug /home/andrew/CLionProjects/Module6/cmake-build-debug/CMakeFiles/Module6.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Module6.dir/depend
