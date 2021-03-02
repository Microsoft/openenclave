# Copyright (c) Open Enclave SDK contributors.
# Licensed under the MIT License.

set(EDL_FILE ../test.edl)

add_custom_command(
  OUTPUT test_t.h test_t.c
  DEPENDS ${EDL_FILE} edger8r
  COMMAND
    edger8r --trusted ${EDL_FILE} --search-path ${PROJECT_SOURCE_DIR}/include
    ${DEFINE_OE_SGX} --search-path ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(quickjs
  COMMAND apkman add quickjs-dev
  DEPENDS apkman-init)

add_enclave(
  TARGET
  quickjs_enc
  UUID
  71b0822f-42a3-4543-a97c-ca491f76b82c
  SOURCES
  enc.c
  ${CMAKE_CURRENT_BINARY_DIR}/test_t.c)

add_dependencies(quickjs_enc quickjs)

enclave_include_directories(quickjs_enc
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  # quickjs include path.
  ${APKMAN_ROOT}/usr/include)


enclave_compile_options(quickjs_enc
   PRIVATE
   -Wno-implicit-int-conversion
   -Wno-bitwise-op-parentheses
   -Wno-shift-op-parentheses
   -Wno-shorten-64-to-32
   -Wno-implicit-float-conversion
   -Wno-unused-parameter
   -Wno-sign-conversion
)

enclave_link_libraries(
  quickjs_enc
  -L${APKMAN_ROOT}/usr/lib/quickjs
  -L${APKMAN_ROOT}/usr/lib  
  libquickjs.a
  libgcc.a
  oelibc
  oehostfs
  -fuse-ld=lld
  -Wl,--warn-unresolved-symbols)

