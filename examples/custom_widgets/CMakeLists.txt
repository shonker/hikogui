
#-------------------------------------------------------------------
# Build Target: hikogui_demo                             (executable)
#-------------------------------------------------------------------

add_custom_target(custom_widget_example_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../../resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    DEPENDS
    hikogui
    resources/copyright.md
    resources/mars3.png
)

add_executable(minimum_custom_widget_example WIN32 MACOSX_BUNDLE)
target_sources(minimum_custom_widget_example PRIVATE minimum_custom_widget_example.cpp)
target_link_libraries(minimum_custom_widget_example PRIVATE hikogui)
target_include_directories(minimum_custom_widget_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(minimum_custom_widget_example custom_widget_example_resources)

add_executable(custom_widget_with_child_example WIN32 MACOSX_BUNDLE)
target_sources(custom_widget_with_child_example PRIVATE custom_widget_with_child_example.cpp)
target_link_libraries(custom_widget_with_child_example PRIVATE hikogui)
target_include_directories(custom_widget_with_child_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(custom_widget_with_child_example custom_widget_example_resources)

add_executable(custom_widget_command_example WIN32 MACOSX_BUNDLE)
target_sources(custom_widget_command_example PRIVATE custom_widget_command_example.cpp)
target_link_libraries(custom_widget_command_example PRIVATE hikogui)
target_include_directories(custom_widget_command_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(custom_widget_command_example custom_widget_example_resources)

add_executable(custom_widget_drawing_example WIN32 MACOSX_BUNDLE)
target_sources(custom_widget_drawing_example PRIVATE custom_widget_drawing_example.cpp)
target_link_libraries(custom_widget_drawing_example PRIVATE hikogui)
target_include_directories(custom_widget_drawing_example PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)
add_dependencies(custom_widget_drawing_example custom_widget_example_resources)

#-------------------------------------------------------------------
# Installation Rules: hikogui_demo
#-------------------------------------------------------------------

install(TARGETS minimum_custom_widget_example DESTINATION examples/custom_widgets)
install(TARGETS custom_widget_with_child_example DESTINATION examples/custom_widgets)
install(TARGETS custom_widget_command_example DESTINATION examples/custom_widgets)
install(TARGETS custom_widget_drawing_example DESTINATION examples/custom_widgets)

install(DIRECTORY resources/ DESTINATION examples/custom_widgets/resources)

# copy additional "hikogui library" resources from top-level
install(DIRECTORY ../../resources/  DESTINATION examples/custom_widgets/resources)

# copy vcpkg dependencies (experimental)
if (VCPKG_TOOLCHAIN)
    x_vcpkg_install_local_dependencies(TARGETS hikogui_demo DESTINATION examples/custom_widgets)
endif()
