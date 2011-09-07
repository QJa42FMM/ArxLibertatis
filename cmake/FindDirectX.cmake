
# Try to find DirectX libraries and include paths.
# Once done this will define
#
# DIRECTX_FOUND
# DIRECTX_INCLUDE_DIR
# DIRECTX_LIBRARIES
# DIRECTX_ROOT_DIR
#
# Adapted from http://code.google.com/p/gamekit/source/browse/trunk/CMake/Packages/FindDirectX.cmake

include(CompileCheck)

if(WIN32) # The only platform it makes sense to check for DirectX SDK
	
	# include dir
	find_path(DIRECTX_INCLUDE_DIR NAMES d3d9.h HINTS $ENV{DXSDK_DIR}/include)
	
	# dlls are in DIRECTX_ROOT_DIR/Developer Runtime/x64|x86
	# lib files are in DIRECTX_ROOT_DIR/Lib/x64|x86
	if(CMAKE_CL_64)
		set(DIRECTX_LIBPATH_SUFFIX "x64")
	else(CMAKE_CL_64)
		set(DIRECTX_LIBPATH_SUFFIX "x86")
	endif(CMAKE_CL_64)
	
	find_library(DIRECTX_D3D9_LIBRARY NAMES d3d9 HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_D3DX9_LIBRARY NAMES d3dx9 HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_DXERR9_LIBRARY NAMES dxerr HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_DXGUID_LIBRARY NAMES dxguid HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_DINPUT8_LIBRARY NAMES dinput8 HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_XINPUT_LIBRARY NAMES xinput HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_DXGI_LIBRARY NAMES dxgi HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	find_library(DIRECTX_D3DCOMPILER_LIBRARY NAMES d3dcompiler HINTS $ENV{DXSDK_DIR}/lib PATH_SUFFIXES ${DIRECTX_LIBPATH_SUFFIX})
	
	if(DIRECTX_INCLUDE_DIR)
		set(DIRECTX_ROOT_DIR "${DIRECTX_INCLUDE_DIR}/..") 
	endif(DIRECTX_INCLUDE_DIR)
	
	set(DIRECTX_LIBRARY 
		${DIRECTX_D3D9_LIBRARY} 
		${DIRECTX_D3DX9_LIBRARY}
		${DIRECTX_DXERR9_LIBRARY}
		${DIRECTX_DXGUID_LIBRARY}
		${DIRECTX_DINPUT8_LIBRARY}
		${DIRECTX_DXGI_LIBRARY}
		${DIRECTX_D3DCOMPILER_LIBRARY}
	)
	
	# look for D3D10.1 components
	if (DIRECTX_INCLUDE_DIR)
		find_path(DIRECTX_D3D10_INCLUDE_DIR NAMES d3d10_1shader.h HINTS ${DIRECTX_INCLUDE_DIR} NO_DEFAULT_PATH)
		get_filename_component(DIRECTX_LIBRARY_DIR "${DIRECTX_LIBRARY}" PATH)
		
		find_library(DIRECTX_D3D10_LIBRARY NAMES d3d10 HINTS ${DIRECTX_LIBRARY_DIR} NO_DEFAULT_PATH)
		find_library(DIRECTX_D3DX10_LIBRARY NAMES d3dx10 HINTS ${DIRECTX_LIBRARY_DIR} NO_DEFAULT_PATH)
		
		if (DIRECTX_D3D10_INCLUDE_DIR AND DIRECTX_D3D10_LIBRARY AND DIRECTX_D3DX10_LIBRARY)
			set(DIRECTX_D3D10_FOUND TRUE)
			set(DIRECTX_D3D10_INCLUDE_DIRS ${DIRECTX_D3D10_INCLUDE_DIR})
	  		set(DIRECTX_D3D10_LIBRARIES ${DIRECTX_D3D10_LIBRARY} ${DIRECTX_D3DX10_LIBRARY}) 
		endif ()
	endif ()
	
	# look for D3D11 components
	if (DIRECTX_INCLUDE_DIR)
		find_path(DIRECTX_D3D11_INCLUDE_DIR NAMES D3D11Shader.h HINTS ${DIRECTX_INCLUDE_DIR} NO_DEFAULT_PATH)
		get_filename_component(DIRECTX_LIBRARY_DIR "${DIRECTX_LIBRARY}" PATH)
		
		find_library(DIRECTX_D3D11_LIBRARY NAMES d3d11 d3d11_beta HINTS ${DIRECTX_LIBRARY_DIR} NO_DEFAULT_PATH)
		find_library(DIRECTX_D3DX11_LIBRARY NAMES d3dx11 HINTS ${DIRECTX_LIBRARY_DIR} NO_DEFAULT_PATH)
		find_library(DIRECTX_D3DX11COMPILER_LIBRARY NAMES d3dcompiler HINTS ${DIRECTX_LIBRARY_DIR} NO_DEFAULT_PATH)
		find_library(DIRECTX_DXGI_LIBRARY NAMES dxgi HINTS ${DIRECTX_LIBRARY_DIR} NO_DEFAULT_PATH)
		
		if (DIRECTX_D3D11_INCLUDE_DIR AND DIRECTX_D3D11_LIBRARY AND DIRECTX_D3DX11_LIBRARY)
			set(DIRECTX_D3D11_FOUND TRUE)
			set(DIRECTX_D3D11_INCLUDE_DIRS ${DIRECTX_D3D11_INCLUDE_DIR})
			set(DIRECTX_D3D11_LIBRARIES ${DIRECTX_D3D11_LIBRARY} ${DIRECTX_D3DX11_LIBRARY} ${DIRECTX_D3DX11COMPILER_LIBRARY} ${DIRECTX_DXGI_LIBRARY})
		endif ()
	endif ()
	
	# handle the QUIETLY and REQUIRED arguments and set DIRECTX_FOUND to TRUE if 
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(DirectX DEFAULT_MSG DIRECTX_LIBRARY DIRECTX_INCLUDE_DIR)
	
elseif(WINE)
	
	function(__check_wine_dx_library VARNAME LIB)
		
		try_link_library(${VARNAME} "${LIB}" ERR)
		
		if(CHECK_${VARNAME}_LINK)
			set(${VARNAME} "${LIB}" PARENT_SCOPE)
		else()
			message(STATUS "Missing DX library ${LIB}.")
			set(${VARNAME} "${VARNAME}-NOTFOUND" PARENT_SCOPE)
		endif()
		
	endfunction(__check_wine_dx_library)
	
	# wineg++ will handle this automatically
	set(DIRECTX_INCLUDE_DIR "")
	
	__check_wine_dx_library(DIRECTX_D3D9_LIBRARY d3d9)
	__check_wine_dx_library(DIRECTX_D3DX9_LIBRARY d3dx9)
	# __check_wine_dx_library(DIRECTX_DXERR9_LIBRARY dxerr) TODO wine doesn't have this
	__check_wine_dx_library(DIRECTX_DXGUID_LIBRARY dxguid)
	__check_wine_dx_library(DIRECTX_DINPUT8_LIBRARY dinput8)
	__check_wine_dx_library(DIRECTX_XINPUT_LIBRARY xinput)
	__check_wine_dx_library(DIRECTX_DXGI_LIBRARY dxgi)
	__check_wine_dx_library(DIRECTX_D3DCOMPILER_LIBRARY d3dcompiler)
	
	set(DIRECTX_LIBRARY 
		${DIRECTX_D3D9_LIBRARY} 
		${DIRECTX_D3DX9_LIBRARY}
		${DIRECTX_DXERR9_LIBRARY}
		${DIRECTX_DXGUID_LIBRARY}
		${DIRECTX_DINPUT8_LIBRARY}
		${DIRECTX_DXGI_LIBRARY}
		${DIRECTX_D3DCOMPILER_LIBRARY}
	)
	
	# handle the QUIETLY and REQUIRED arguments and set DIRECTX_FOUND to TRUE if 
	# all listed variables are TRUE
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(DirectX DEFAULT_MSG DIRECTX_LIBRARY)
	
endif()

if(DIRECTX_FOUND)
	set(DIRECTX_LIBRARIES ${DIRECTX_LIBRARY})
endif(DIRECTX_FOUND)
