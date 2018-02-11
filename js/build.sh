em++ -Wall -Wno-write-strings -Wno-char-subscripts -o tinybas.js -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' main_js.c ../core/*.c
