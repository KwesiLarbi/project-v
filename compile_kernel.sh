# set location for cross compiler and target
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf

# compile boot using cross compiler
$PREFIX/bin/$TARGET-as boot.s -o boot.o

# compile kernel using cross compiler
$PREFIX/bin/$TARGET-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# link kernel
$PREFIX/bin/$TARGET-gcc -T linker.ld -o vos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc