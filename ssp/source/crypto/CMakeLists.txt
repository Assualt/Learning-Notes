set(crypto_objects
    archive.cpp
    archive_core.cpp
    archive_stream.cpp
)

add_library(zip-headers-interface INTERFACE)
target_include_directories(zip-headers-interface 
INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/7zip/
)

add_library(ssp_crypto SHARED ${crypto_objects})
target_link_libraries(ssp_crypto PUBLIC
    ssp_base
    zip-headers-interface
)