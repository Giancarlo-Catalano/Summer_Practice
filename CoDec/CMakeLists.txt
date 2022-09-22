#declare this library
#add_library(CoDec CoDec.cpp CoDec.hpp )

#declare directories to look into
#add_subdirectory(IdentityCoDec)
#add_subdirectory(RLCoDec)
add_subdirectory(XNFCoDec)
add_subdirectory(CoDecTechniques)


# target_include_directories(CoDec PUBLIC IdentityCoDec RLCoDec)

#declare which libraries are necessary for this library
target_link_libraries(XNFCoDec CoDecTechniques)


