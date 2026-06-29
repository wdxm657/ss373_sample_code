cd ~/code/star373/Ifado_IMSSV03C12/SourceCode/project
cp image/output/images/boot/u-boot.xz.img.bin board/ifado/boot/usb/upgrade/uboot/
cp image/output/images/boot/IPL.ifado_nand.bin board/ifado/boot/usb/upgrade/ipl/
./image/makefiletools/script/make_usb_factory_sigmastar.sh -i IPL.ifado_nand.bin -u u-boot.xz.img.bin -f
cd ~/code/star373/Ifado_IMSSV03C12/SourceCode/sdk/verify/sample_code