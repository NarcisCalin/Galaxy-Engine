if(${CMAKE_VERSION} LESS 3.30 AND ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL MSVC)
	add_library(openmp INTERFACE)

	target_compile_options(openmp INTERFACE -openmp:llvm)
	target_link_libraries(openmp INTERFACE libomp)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang AND NOT ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL MSVC)
	# Non-VC non-CL clang on Windows just does not want to cooperate with FindOpenMP.
	add_library(openmp INTERFACE)

	target_compile_options(openmp INTERFACE -fopenmp=libomp)
	target_link_options(openmp INTERFACE -fopenmp=libomp)
else()
	set(OpenMP_RUNTIME_MSVC llvm)
	find_package(OpenMP REQUIRED COMPONENTS CXX)

	add_library(openmp INTERFACE)
	target_link_libraries(openmp INTERFACE OpenMP::OpenMP_CXX)
endif()
