cmake_minimum_required(VERSION 3.1)

set(project_name example-libressl) ## rename your project here

project(${project_name})

set(CMAKE_CXX_STANDARD 11)

add_library(${project_name}-lib
        src/AppComponent.hpp
        src/client/MyApiClient.hpp
        src/controller/MyController.cpp
        src/controller/MyController.hpp
        src/dto/MyDTOs.hpp
)

target_include_directories(${project_name}-lib PUBLIC src)

## link libs

find_package(oatpp          0.19.10 REQUIRED)
find_package(oatpp-libressl 0.19.10 REQUIRED)

include(FindPkgConfig) # <-- include pkg-config needed by FindLibreSSL.cmake script
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake") # <-- use FindLibreSSL.cmake in /cmake folder

find_package(LibreSSL 3.0.0 REQUIRED)

target_link_libraries(${project_name}-lib

        # Oat++ libraries
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
        PUBLIC oatpp::oatpp-libressl # <-- oatpp-libressl adapter

        # LibreSSL libraries
        PUBLIC LibreSSL::TLS
        PUBLIC LibreSSL::SSL
        PUBLIC LibreSSL::Crypto

)

#################################################################
## define certificates path

add_definitions(
        -DCERT_PEM_PATH="${CMAKE_CURRENT_LIST_DIR}/cert/test_key.pem"
        -DCERT_CRT_PATH="${CMAKE_CURRENT_LIST_DIR}/cert/test_cert.crt"
)

#################################################################
## add executables

add_executable(${project_name}-exe
        src/App.cpp
)
target_link_libraries(${project_name}-exe ${project_name}-lib)
add_dependencies(${project_name}-exe ${project_name}-lib)

add_executable(${project_name}-test
        test/tests.cpp
)
target_link_libraries(${project_name}-test ${project_name}-lib)
add_dependencies(${project_name}-test ${project_name}-lib)

enable_testing()
add_test(project-tests ${project_name}-test)
