add_executable(tldextract unit_tldextract.cpp tldextract.cpp)
target_link_libraries(tldextract
    ssp_base
    ssp_net
)