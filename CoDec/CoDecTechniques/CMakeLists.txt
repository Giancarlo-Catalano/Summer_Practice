#declare directories to look into
add_library(CoDecTechniques HuffmanCoder/HuffmanCoder.hpp HuffmanCoder/HuffmanCoder.cpp ArithmeticCoder/ArithmeticCoder.hpp ArithmeticCoder/ArithmeticCoder.cpp CoDecUtilities/CoDecUtilities.cpp CoDecUtilities/CoDecUtilities.hpp)

#declare which libraries are necessary for this library
target_link_libraries(CoDecTechniques)


