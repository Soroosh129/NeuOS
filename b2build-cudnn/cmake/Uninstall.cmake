if(NOT EXISTS "/home/nvidia/caffe-build-dvfs/build-cudnn/install_manifest.txt")
  message(FATAL_ERROR "Cannot find install manifest: /home/nvidia/caffe-build-dvfs/build-cudnn/install_manifest.txt")
endif(NOT EXISTS "/home/nvidia/caffe-build-dvfs/build-cudnn/install_manifest.txt")

if (NOT DEFINED CMAKE_INSTALL_PREFIX)
  set (CMAKE_INSTALL_PREFIX "/home/nvidia/caffe-build-dvfs/build-cudnn/install")
endif ()
 message(${CMAKE_INSTALL_PREFIX})

file(READ "/home/nvidia/caffe-build-dvfs/build-cudnn/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach(file ${files})
  message(STATUS "Uninstalling $ENV{DESTDIR}${file}")
  if(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
    exec_program(
      "/usr/bin/cmake" ARGS "-E remove \"$ENV{DESTDIR}${file}\""
      OUTPUT_VARIABLE rm_out
      RETURN_VALUE rm_retval
      )
    if(NOT "${rm_retval}" STREQUAL 0)
      message(FATAL_ERROR "Problem when removing $ENV{DESTDIR}${file}")
    endif(NOT "${rm_retval}" STREQUAL 0)
  else(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
    message(STATUS "File $ENV{DESTDIR}${file} does not exist.")
  endif(IS_SYMLINK "$ENV{DESTDIR}${file}" OR EXISTS "$ENV{DESTDIR}${file}")
endforeach(file)
