
macro(ovm_io_find_Python)
find_package(PythonLibs 3 QUIET)
if(NOT PYTHONLIBS_FOUND)
    message(
        STATUS
        "CMake did not find Python library, 
            using default fallbacks (edit WHERE_IS... in CMakeGUI if need be)."	
    )
    set(PYTHON_INCLUDE_DIRS ${WHERE_ARE_PYTHON_INCLUDES})
    set(PYTHON_LIBRARIES ${WHERE_IS_PYTHON_LIB})
endif()
if(
    NOT "${PYTHON_INCLUDE_DIRS}" STREQUAL "" AND
    NOT "${PYTHON_LIBRARIES}" STREQUAL ""
)
    set(OVM_IO_FOUND_PYTHON TRUE)
endif()
endmacro()

if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/ext/geogram/)
set(
    GEOGRAM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ext/geogram/"
    CACHE PATH "full path to the Geogram installation"
)
set(USE_BUILTIN_GEOGRAM TRUE)
else()
message(
    SEND_ERROR
    "CMake did not find Geogram in ${CMAKE_SOURCE_DIR}/ext/geogram/"	
    )
endif()
