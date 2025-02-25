# Lists of directories with source files:
#  See "DeclareMRPTLib.cmake" for explanations
# -------------------------------------------------
list(APPEND maps_EXTRA_SRCS			"${MRPT_SOURCE_DIR}/libs/maps/src/maps/*.cpp" "${MRPT_SOURCE_DIR}/libs/maps/include/mrpt/maps/*.h" "${MRPT_SOURCE_DIR}/libs/maps/include/mrpt/slam/*.h")
list(APPEND maps_EXTRA_SRCS_NAME 	"maps" "maps" "back-compat-hdrs")

list(APPEND maps_EXTRA_SRCS			"${MRPT_SOURCE_DIR}/libs/maps/src/opengl/*.cpp" "${MRPT_SOURCE_DIR}/libs/maps/include/mrpt/opengl/*.h")
list(APPEND maps_EXTRA_SRCS_NAME 	"opengl" "opengl")

list(APPEND maps_EXTRA_SRCS			"${MRPT_SOURCE_DIR}/libs/maps/src/obs/*.cpp" "${MRPT_SOURCE_DIR}/libs/maps/include/mrpt/obs/*.h")
list(APPEND maps_EXTRA_SRCS_NAME 	"obs" "obs")

if(CMAKE_MRPT_HAS_LIBLAS)
	link_directories(${LAS_LINK_DIRS})
	include_directories(${LAS_INCLUDE_DIRS})
endif()

#---------------------------------------------
# Macro declared in "DeclareMRPTLib.cmake":
#---------------------------------------------
define_mrpt_lib(
	# Lib name
	maps
	# Dependencies:
	mrpt-obs
	mrpt-graphs
	# Deps of deps:
	mrpt-opengl
	)

if(BUILD_mrpt-maps)
	if(CMAKE_MRPT_HAS_LIBLAS)
		target_link_libraries(maps PRIVATE ${LAS_LIBS})
	endif()

	if (CMAKE_MRPT_HAS_OCTOMAP)
		if (NOT "${OCTOMAP_LIBRARY_DIRS}" STREQUAL "")
			link_directories("${OCTOMAP_LIBRARY_DIRS}")
		endif()
		if(TARGET "EP_octomap")
			add_dependencies(maps EP_octomap)
		endif()
		target_link_libraries(maps PRIVATE ${OCTOMAP_LIBRARIES})
	endif()

	target_link_libraries(maps PRIVATE ${MRPT_OPENGL_LIBS})


	# Windows only: the mxplus lib internally requires linking with opencv (sigh...)
	# Perhaps there is a better way to solve this, someday.
	if (MRPT_WITH_MATLAB_WRAPPER)
		target_link_libraries(maps PRIVATE imp_opencv)
	endif()
endif()
