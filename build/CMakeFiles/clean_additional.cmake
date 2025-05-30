# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CAD_Model_Viewer_autogen"
  "CMakeFiles/CAD_Model_Viewer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/CAD_Model_Viewer_autogen.dir/ParseCache.txt"
  )
endif()
