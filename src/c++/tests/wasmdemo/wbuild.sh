emcc -v \
  wasmdemo.cpp \
  types.cpp \
  crypto.cpp \
  imod.cpp \
  matrix.cpp \
  poly.cpp \
  berlwelch.cpp \
  input.cpp \
  params.cpp \
  secret.cpp \
  fuzzy.cpp \
  parsing.cpp \
  utils.cpp \
  json.c \
  -Wno-deprecated-declarations \
  -I${EMSCRIPTEN}/system/include \
  -L${EMSCRIPTEN}/system/lib \
  -lssl \
  -lcrypto \
  -s USE_PTHREADS=1 \
  -s EXIT_RUNTIME=1 \
  -s DISABLE_EXCEPTION_CATCHING=0
