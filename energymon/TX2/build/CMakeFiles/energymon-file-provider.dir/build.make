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
include CMakeFiles/energymon-file-provider.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/energymon-file-provider.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/energymon-file-provider.dir/flags.make

CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o: CMakeFiles/energymon-file-provider.dir/flags.make
CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o: ../src/app/energymon-file-provider.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o   -c /home/nvidia/energymon/src/app/energymon-file-provider.c

CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nvidia/energymon/src/app/energymon-file-provider.c > CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.i

CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nvidia/energymon/src/app/energymon-file-provider.c -o CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.s

CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.requires:

.PHONY : CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.requires

CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.provides: CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.requires
	$(MAKE) -f CMakeFiles/energymon-file-provider.dir/build.make CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.provides.build
.PHONY : CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.provides

CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.provides.build: CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o


CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o: CMakeFiles/energymon-file-provider.dir/flags.make
CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o: ../src/energymon-time-util.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o   -c /home/nvidia/energymon/src/energymon-time-util.c

CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nvidia/energymon/src/energymon-time-util.c > CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.i

CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nvidia/energymon/src/energymon-time-util.c -o CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.s

CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.requires:

.PHONY : CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.requires

CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.provides: CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.requires
	$(MAKE) -f CMakeFiles/energymon-file-provider.dir/build.make CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.provides.build
.PHONY : CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.provides

CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.provides.build: CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o


CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o: CMakeFiles/energymon-file-provider.dir/flags.make
CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o: ../src/ptime/ptime.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o   -c /home/nvidia/energymon/src/ptime/ptime.c

CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/nvidia/energymon/src/ptime/ptime.c > CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.i

CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/nvidia/energymon/src/ptime/ptime.c -o CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.s

CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.requires:

.PHONY : CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.requires

CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.provides: CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.requires
	$(MAKE) -f CMakeFiles/energymon-file-provider.dir/build.make CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.provides.build
.PHONY : CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.provides

CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.provides.build: CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o


# Object files for target energymon-file-provider
energymon__file__provider_OBJECTS = \
"CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o" \
"CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o" \
"CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o"

# External object files for target energymon-file-provider
energymon__file__provider_EXTERNAL_OBJECTS =

bin/energymon-file-provider: CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o
bin/energymon-file-provider: CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o
bin/energymon-file-provider: CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o
bin/energymon-file-provider: CMakeFiles/energymon-file-provider.dir/build.make
bin/energymon-file-provider: lib/libenergymon-default.so
bin/energymon-file-provider: CMakeFiles/energymon-file-provider.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/nvidia/energymon/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable bin/energymon-file-provider"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/energymon-file-provider.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/energymon-file-provider.dir/build: bin/energymon-file-provider

.PHONY : CMakeFiles/energymon-file-provider.dir/build

CMakeFiles/energymon-file-provider.dir/requires: CMakeFiles/energymon-file-provider.dir/src/app/energymon-file-provider.c.o.requires
CMakeFiles/energymon-file-provider.dir/requires: CMakeFiles/energymon-file-provider.dir/src/energymon-time-util.c.o.requires
CMakeFiles/energymon-file-provider.dir/requires: CMakeFiles/energymon-file-provider.dir/src/ptime/ptime.c.o.requires

.PHONY : CMakeFiles/energymon-file-provider.dir/requires

CMakeFiles/energymon-file-provider.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/energymon-file-provider.dir/cmake_clean.cmake
.PHONY : CMakeFiles/energymon-file-provider.dir/clean

CMakeFiles/energymon-file-provider.dir/depend:
	cd /home/nvidia/energymon/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/nvidia/energymon /home/nvidia/energymon /home/nvidia/energymon/build /home/nvidia/energymon/build /home/nvidia/energymon/build/CMakeFiles/energymon-file-provider.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/energymon-file-provider.dir/depend

