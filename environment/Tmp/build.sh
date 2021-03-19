
nasm -f macho64 asm.s
clang asm.o spec.c -o spec