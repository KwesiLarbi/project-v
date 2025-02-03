mkdir -p isodir/boot/grub
cp vos.bin isodir/boot/vos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o vos.iso isodir