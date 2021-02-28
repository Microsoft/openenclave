# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

# Create an imported executable for use by tests.
add_executable(apkman IMPORTED)
set_target_properties(apkman PROPERTIES
  IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/tools/apkman/apkman)

# Fetch apkman root folder for accessing includes and libraries.
execute_process(
  COMMAND  ${PROJECT_SOURCE_DIR}/tools/apkman/apkman root
  COMMAND_ERROR_IS_FATAL ANY
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE APKMAN_ROOT)

# Ensure that apkman is initialized.
add_custom_target(apkman-init
  # Setup so that packages can be built from source.
  COMMAND apkman build-init
)

# Testing utilities.
add_subdirectory(utils)

# C library tests.
add_subdirectory(libs)

# C++ library tests.
add_subdirectory(cpp)
