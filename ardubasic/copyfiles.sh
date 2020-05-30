for x in ../core/*.[ch]; do ln -s "$x"; done
for f in *.c; do mv "$f" "${f%.c}.cpp"; done
