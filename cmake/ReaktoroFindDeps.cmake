if(REAKTORO_ENABLE_OPENLIBM)
    find_package(openlibm REQUIRED)
endif()

set(REAKTORO_USE_autodiff        "" CACHE PATH "Specify this option in case a specific autodiff library should be used.")
set(REAKTORO_USE_Catch2          "" CACHE PATH "Specify this option in case a specific Catch2 library should be used.")
set(REAKTORO_USE_Eigen3          "" CACHE PATH "Specify this option in case a specific Eigen3 library should be used.")
set(REAKTORO_USE_nlohmann_json   "" CACHE PATH "Specify this option in case a specific nlohmann_json library should be used.")
set(REAKTORO_USE_Optima          "" CACHE PATH "Specify this option in case a specific Optima library should be used.")
set(REAKTORO_USE_phreeqc4rkt     "" CACHE PATH "Specify this option in case a specific phreeqc4rkt library should be used.")
set(REAKTORO_USE_pybind11        "" CACHE PATH "Specify this option in case a specific pybind11 library should be used.")
set(REAKTORO_USE_reaktplot       "" CACHE PATH "Specify this option in case a specific reaktplot library should be used.")
set(REAKTORO_USE_tabulate        "" CACHE PATH "Specify this option in case a specific tabulate library should be used.")
set(REAKTORO_USE_ThermoFun       "" CACHE PATH "Specify this option in case a specific ThermoFun library should be used.")
set(REAKTORO_USE_tsl-ordered-map "" CACHE PATH "Specify this option in case a specific tsl-ordered-map library should be used.")
set(REAKTORO_USE_yaml-cpp        "" CACHE PATH "Specify this option in case a specific yaml-cpp library should be used.")

function(ReaktoroFindPackage name)
    if(DEFINED REAKTORO_USE_${ARGV0} AND NOT REAKTORO_USE_${ARGV0} STREQUAL "")
        find_package(${name} ${ARGN} QUIET PATHS ${REAKTORO_USE_${name}} NO_DEFAULT_PATH)
    else()
        find_package(${name} ${ARGN} QUIET)
    endif()
    if(${name}_FOUND)
        message(STATUS "Found ${name}: ${${name}_DIR} (found version \"${${name}_VERSION}\")")
    endif()
    set(${name}_FOUND ${${name}_FOUND} PARENT_SCOPE)  # export to parent (e.g., Eigen3_FOUND)
    set(${name}_DIR ${${name}_DIR} PARENT_SCOPE)  # export to parent (e.g., Eigen3_DIR)
endfunction()

# Required dependencies
ReaktoroFindPackage(autodiff 1.0.3 REQUIRED)
ReaktoroFindPackage(Eigen3 3.4 REQUIRED)
ReaktoroFindPackage(nlohmann_json 3.6.1 REQUIRED)
ReaktoroFindPackage(Optima 0.3.3 REQUIRED)
ReaktoroFindPackage(phreeqc4rkt 3.6.2.1 REQUIRED)
ReaktoroFindPackage(tabulate 1.4.0 REQUIRED)
ReaktoroFindPackage(ThermoFun 0.4.5 REQUIRED)
ReaktoroFindPackage(tsl-ordered-map 1.0.0 REQUIRED)
ReaktoroFindPackage(yaml-cpp 0.6.3 REQUIRED)

# Optional dependencies
ReaktoroFindPackage(reaktplot 0.4.1)
ReaktoroFindPackage(pybind11 2.10.0)
ReaktoroFindPackage(Catch2 2.6.2)

if(REAKTORO_BUILD_TESTS)
    if(NOT Catch2_FOUND)
        message(WARNING "Could not find Catch2. The C++ tests of Reaktoro will not be built!")
        set(REAKTORO_BUILD_TESTS OFF)
    endif()
endif()

if(REAKTORO_BUILD_PYTHON)
    find_program(PYBIND11_STUBGEN pybind11-stubgen)
    if(NOT pybind11_FOUND)
        message(WARNING "Could not find pybind11. The Python package reaktoro will not be built!")
        set(REAKTORO_BUILD_PYTHON OFF)
    endif()
    if(NOT PYBIND11_STUBGEN)
        message(WARNING "Could not find pybind11-stubgen (available via pip or conda). There will be no stubs generated for the python package reaktoro.")
    endif()
endif()
