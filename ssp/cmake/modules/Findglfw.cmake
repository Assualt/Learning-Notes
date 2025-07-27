include(ExternalProject)

set(GLFW_INSTALL_DIR ${CMAKE_BINARY_DIR}/glfw)
if (WIN32)
    set(GLFW_PACAGE "glfw-3.4.bin.WIN32")
elseif(APPLE)
    set(GLFW_PACAGE "glfw-3.4.bin.MACOS")
else()
    set(GLFW_PACAGE "glfw-3.4")
endif()

set(GLFW_DOWNLOAD_URL "https://github.com/glfw/glfw/releases/download/3.4/${GLFW_PACAGE}.zip")

add_custom_target(glfw-download
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GLFW_INSTALL_DIR}
    COMMAND wget --no-check-certificate ${GLFW_DOWNLOAD_URL} -O ${GLFW_INSTALL_DIR}/glfw.zip && echo "Downloaded GLFW success" 
    COMMAND unzip -o ${GLFW_INSTALL_DIR}/glfw.zip -d ${GLFW_INSTALL_DIR} && echo "Unzip GLFW success"
    COMMAND ln -sf ${GLFW_INSTALL_DIR}/${GLFW_PACAGE} ${GLFW_INSTALL_DIR}/glfw
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Downloading GLFW..."
)

set(GLFW_DIR ${GLFW_INSTALL_DIR}/glfw)

find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
find_library(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)
find_library(IOKIT_LIBRARY IOKit REQUIRED)
find_library(COCOA_LIBRARY Cocoa REQUIRED)

add_library(glfw-interface INTERFACE)
target_include_directories(glfw-interface 
INTERFACE
    ${GLFW_DIR}/include
)

set(GLFW_LIBRARIES ${GLFW_DIR}/lib-x86_64/libglfw3.a ${COREFOUNDATION_LIBRARY} ${COREGRAPHICS_LIBRARY} ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
