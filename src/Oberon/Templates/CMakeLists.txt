find_package(Magnum REQUIRED
    GlfwApplication)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_executable(GlfwTemplate MACOSX_BUNDLE GlfwTemplate.cpp GlfwTemplate.h)
target_link_libraries(GlfwTemplate PRIVATE
    Magnum::Application
    Oberon::Core)
