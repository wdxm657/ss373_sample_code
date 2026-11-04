# https://dev.comake.online/home/article/8614
export CROSS_COMPILE=arm-sigmastar-linux-uclibcgnueabihf-9.1.0-

export ARCH=arm
make source/ifado/audio/dog_soother
adb push /home/xh/code/star373/Ifado_IMSSV03C12/SourceCode/sdk/verify/sample_code/out/arm/app/dog_soother /customer/sample_code/bin/

