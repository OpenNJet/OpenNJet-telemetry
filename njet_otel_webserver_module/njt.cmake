include(ExternalProject)

#set(NJET_DIR "/njt_telemetry/njet")
set(NJET_DIR "../njet_main")
#set(NJET_DIR "/root/project/njet/njet_main")

set(NJET_INCLUDE_DIRS
  ${NJET_DIR}/objs
  ${NJET_DIR}/auto/lib/tongsuo/.openssl/include
  ${NJET_DIR}/src/core
  ${NJET_DIR}/src/os/unix
  ${NJET_DIR}/src/event
  ${NJET_DIR}/src/event/quic
  ${NJET_DIR}/src/http
  ${NJET_DIR}/src/http/v2
  ${NJET_DIR}/src/http/v3
  ${NJET_DIR}/src/http/modules
  ${NJET_DIR}/modules/njet-http-kv-module/src
  ${NJET_DIR}/modules/njet-util-module/include
)
