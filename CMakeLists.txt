cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 20)

#Declare the project
project(Evocom)

#declare main and other top-level cpp files
add_executable(${PROJECT_NAME} main.cpp XNF/XNF.cpp XNF/XNF.hpp XNFBreeder/XNFBreeder.cpp XNFBreeder/XNFBreeder.hpp)

#have to add -mbi2 because of pdep used in Conjunction.hpp
SET(CMAKE_COVERAGE_COMPILE_FLAGS "-march=native")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_COVERAGE_COMPILE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3") #to set the optimisation level


#declare subdirectories that will be searched for CMakeLists.txt
add_subdirectory(Utilities)
add_subdirectory(CoDec)
add_subdirectory(XNF)
add_subdirectory(XNFBreeder)
add_subdirectory(XNFEvolver)

#declare where to look for headers
#target_include_directories(${PROJECT_NAME} PUBLIC Utilities CoDec Evolution/EA)


#declare that the following directories should be used for linking
#target_link_directories(${PROJECT_NAME} PUBLIC Utilities CoDec Evolution/EA)

target_link_libraries(${PROJECT_NAME} Utilities XNFCoDec XNF XNFBreeder )




#this is the old command I used to have, it is one line after all...
#add_executable(Tree main.cpp Tree/Tree.cpp Tree/Tree.hpp Utilities/utilities.hpp Utilities/utilities.cpp FileReading/FilePresenter.cpp FileReading/FilePresenter.hpp Tree/NTBC/NTBC.hpp Tree/NTBC/NTBC.cpp CoDec/CoDec.cpp CoDec/CoDec.hpp CoDec/IdentityCoDec/IdentityCoDec.cpp CoDec/IdentityCoDec/IdentityCoDec.hpp CoDec/RLCoDec/RLCoDec.cpp CoDec/RLCoDec/RLCoDec.hpp Evolution/EA/EA.cpp Evolution/EA/EA.hpp)

