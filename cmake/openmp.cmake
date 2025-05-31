if(${CMAKE_VERSION} LESS 3.30 AND ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL MSVC)
	add_library(openmp INTERFACE)

	target_compile_options(openmp INTERFACE "-openmp:llvm")
	target_link_libraries(openmp INTERFACE libomp)

	if(${CMAKE_CXX_COMPILER_ID} STREQUAL CLANG)
	endif()
else()
	set(OpenMP_RUNTIME_MSVC llvm)
	find_package(OpenMP REQUIRED)

	add_library(openmp INTERFACE)
	target_link_libraries(openmp INTERFACE OpenMP::OpenMP_CXX)
endif()
