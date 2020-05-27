# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/nvidia/energymon

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/nvidia/energymon/build

# Include any dependencies generated for this target.
include msr/CMakeFiles/energymon-msr.dir/depend.make

# Include the progress variables for this target.
include msr/CMakeFiles/energymon-msr.dir/progress.make

# Include the compile flags for this target's objects.
include msr/CMakeFiles/energymon-msr.dir/flags.make

msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o: msr/CMakeFiles/energymon-msr.dir/flags.make
msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o: ../msr/energymon-msr.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o"
	cd /home/nvidia/energymon/build/msr && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/energymon-msr.dir/energymon-msr.c.o   -c /home/nvidia/energymon/msr/energymon-msr.c

msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/energymon-msr.dir/energymon-msr.c.i"
	cd /home/nvidia/energymon/build/msr && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nvidia/energymon/msr/energymon-msr.c > CMakeFiles/energymon-msr.dir/energymon-msr.c.i

msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/energymon-msr.dir/energymon-msr.c.s"
	cd /home/nvidia/energymon/build/msr && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nvidia/energymon/msr/energymon-msr.c -o CMakeFiles/energymon-msr.dir/energymon-msr.c.s

msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.requires:

.PHONY : msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.requires

msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.provides: msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.requires
	$(MAKE) -f msr/CMakeFiles/energymon-msr.dir/build.make msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.provides.build
.PHONY : msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.provides

msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.provides.build: msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o


msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o: msr/CMakeFiles/energymon-msr.dir/flags.make
msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o: ../src/energymon-util.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o"
	cd /home/nvidia/energymon/build/msr && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o   -c /home/nvidia/energymon/src/energymon-util.c

msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.i"
	cd /home/nvidia/energymon/build/msr && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nvidia/energymon/src/energymon-util.c > CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.i

msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.s"
	cd /home/nvidia/energymon/build/msr && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nvidia/energymon/src/energymon-util.c -o CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.s

msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.requires:

.PHONY : msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.requires

msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.provides: msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.requires
	$(MAKE) -f msr/CMakeFiles/energymon-msr.dir/build.make msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.provides.build
.PHONY : msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.provides

msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.provides.build: msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o


# Object files for target energymon-msr
energymon__msr_OBJECTS = \
"CMakeFiles/energymon-msr.dir/energymon-msr.c.o" \
"CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o"

# External object files for target energymon-msr
energymon__msr_EXTERNAL_OBJECTS =

lib/libenergymon-msr.so: msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o
lib/libenergymon-msr.so: msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o
lib/libenergymon-msr.so: msr/CMakeFiles/energymon-msr.dir/build.make
lib/libenergymon-msr.so: msr/CMakeFiles/energymon-msr.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C shared library ../lib/libenergymon-msr.so"
	cd /home/nvidia/energymon/build/msr && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/energymon-msr.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
msr/CMakeFiles/energymon-msr.dir/build: lib/libenergymon-msr.so

.PHONY : msr/CMakeFiles/energymon-msr.dir/build

msr/CMakeFiles/energymon-msr.dir/requires: msr/CMakeFiles/energymon-msr.dir/energymon-msr.c.o.requires
msr/CMakeFiles/energymon-msr.dir/requires: msr/CMakeFiles/energymon-msr.dir/__/src/energymon-util.c.o.requires

.PHONY : msr/CMakeFiles/energymon-msr.dir/requires

msr/CMakeFiles/energymon-msr.dir/clean:
	cd /home/nvidia/energymon/build/msr && $(CMAKE_COMMAND) -P CMakeFiles/energymon-msr.dir/cmake_clean.cmake
.PHONY : msr/CMakeFiles/energymon-msr.dir/clean

msr/CMakeFiles/energymon-msr.dir/depend:
	cd /home/nvidia/energymon/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nvidia/energymon /home/nvidia/energymon/msr /home/nvidia/energymon/build /home/nvidia/energymon/build/msr /home/nvidia/energymon/build/msr/CMakeFiles/energymon-msr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : msr/CMakeFiles/energymon-msr.dir/depend

