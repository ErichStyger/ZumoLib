# set the library name. In the case below, we name it 'srcLib'
# you can use that library name in anoher CMakeLists.txt
set(THIS_LIBRARY_NAME ZumoLib)

# list all the files you want to be added to the library
file(GLOB FILES
  *.c
)

# add_library: With this declaration, you express the intent to build a library. 
# The first argument is the name of the library, 
# the second argument are the files that will be compiled to create your library.
add_library(${THIS_LIBRARY_NAME} STATIC ${FILES})

# target_link_libraries: If you link with other libraries, list them here
target_link_libraries(
  ${THIS_LIBRARY_NAME}
  PUBLIC 
    McuLib
)

# target_include_directories: Libraries need to publish their header files 
# so that you can import them in source code. This statement expresses where to find the files 
# - typically in an include directory of your projects.
target_include_directories(
  ${THIS_LIBRARY_NAME}
  PRIVATE 
  .
)
