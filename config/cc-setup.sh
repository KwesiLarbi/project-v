#!/bin/bash

####################################
echo Stage 1 - Building Dependencies
####################################


mkdir $HOME/opt
mkdir $HOME/opt/cross

cd $HOME/opt

cd cross

# install or update all apt-get dependencies
sudo apt-get update
sudo apt-get upgrade -y
sudo apt-get install gcc -y
sudo apt-get install g++ -y
sudo apt-get install make -y
sudo apt-get install bison -y
sudo apt-get install flex -y
sudo apt-get install gwk -y
sudo apt-get install libgmp3-dev -y
sudo apt-get install libmpfr-dev -y
sudo apt-get install mpc -y
sudo apt-get install texinfo -y
sudo apt-get install libcloog-isl-dev -y
sudo apt-get install build-essential -y
sudo apt-get install glibc-devel -y
sudo apt-get -y install gcc-multilib libc6-i386 -y

wget https://ftp.gnu.org/gnu/binutils/binutils-2.43.1.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.gz
wget https://ftp.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz
for f in *tar*; do tar zvxf $f; done

# create installation directory
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"


########################################
echo Stage 2 - Installing Cross Compiler
########################################


# install mpc
cd $HOME/opt/cross

mkdir build-mpc 
cd build-mpc
../mpc-1.3.1/configure --prefix="$PREFIX"
make -j2
make -j2 check
make -j2 install

# install binutils
cd $HOME/opt/cross

mkdir build-binutils
cd build-binutils
../binutils-2.43.1/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j2
make -j2 install


# install gcc
cd $HOME/opt/cross

which -- $TARGET-as || echo $TARGET-as is not in the PATH

mkdir build-gcc
cd build-gcc
../gcc-14.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --with-mpc="$PREFIX" --disable-hosted-libstdcxx
make -j2 all-gcc
make -j2 all-target-libgcc
make -j2 all-target-libstdc++-v3
make -j2 install-gcc
make -j2 install-target-libgcc
make -j2 install-target-libstdc++-v3

# Run '$HOME/opt/cross/bin/$TARGET-gcc --version' to test new compiler
# Had to export TARGET=i686-elf after the fact for some reason