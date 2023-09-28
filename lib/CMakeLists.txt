
# Add a cmake binary taget (in this case, a library)
add_library(gm_lib gm_lib.cpp gm_lib.hpp)

# Make the directory known
target_include_directories(gm_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/external
)

# Link library to a binary target
target_link_libraries(gm_lib PRIVATE
    easywsclient
    json
)