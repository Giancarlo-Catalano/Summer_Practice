add_library(XNF XNF.cpp XNF.hpp XNF_Generic.cpp XNF_Generic.hpp)
add_subdirectory(Conjunction)
target_link_libraries(XNF Conjunction)
