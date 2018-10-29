docker run --rm -v $(pwd)/..:/src -w /src/js trzeci/emscripten em++ \
    -Wall -Wno-write-strings -Wno-char-subscripts -o tinybas.js \
    -s WASM=0 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    main_js.c ../core/*.c

