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

find_package(oatpp          0.19.9 REQUIRED)
find_package(oatpp-libressl 0.19.9 REQUIRED)

target_link_libraries(${project_name}-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
        PUBLIC oatpp::oatpp-libressl
)

#################################################################
## link libressl

include(FindPkgConfig)

pkg_check_modules(PKG_TLS REQUIRED libtls)
pkg_check_modules(PKG_SSL REQUIRED libssl)
pkg_check_modules(PKG_CRYPTO REQUIRED libcrypto)

target_include_directories(${project_name}-lib
        PUBLIC ${PKG_TLS_INCLUDE_DIRS}
        PUBLIC ${PKG_SSL_INCLUDE_DIRS}
        PUBLIC ${PKG_CRYPTO_INCLUDE_DIRS}
)

link_directories(
        ${PKG_TLS_LIBRARY_DIRS}
        ${PKG_SSL_LIBRARY_DIRS}
        ${PKG_CRYPTO_LIBRARY_DIRS}
)

target_link_libraries(${project_name}-lib
        PUBLIC ${PKG_TLS_LIBRARIES}
        PUBLIC ${PKG_SSL_LIBRARIES}
        PUBLIC ${PKG_CRYPTO_LIBRARIES}
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
