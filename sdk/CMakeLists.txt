add_subdirectory (php)
add_subdirectory (python)
add_subdirectory (cpp)

add_custom_target(sdk DEPENDS php php_test php_docs python)
