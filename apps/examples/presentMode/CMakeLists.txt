set(SOURCES
    presentModes.cpp
)

add_executable(presentMode ${SOURCES})
add_dependencies(examples presentMode)

find_package(PNG)
find_package(X11)

target_link_libraries(presentMode gepard ${X11_LIBRARIES} ${PNG_LIBRARIES} ${GEPARD_DEP_LIBS})
