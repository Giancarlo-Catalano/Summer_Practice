#declare this library
add_library(XNFCoDec XNFCoDec.cpp XNFCoDec.hpp Reader.hpp Writer.hpp)

#declare directories to look into
add_subdirectory(XNFEncoder)
add_subdirectory(XNFDecoder)
#declare which libraries are necessary for this library
target_link_libraries(XNFCoDec XNFEncoder XNFDecoder)