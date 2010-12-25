source /home/projects/stb7109/setenv-stfae.sh COCOREF_GOLD_7109_LINUX
sudo ./STM/builduboot sh4
sudo ./STM/buildenv sh4
sudo make
cd BUILT/u-boot/stb7100ref_27-stm20/
sudo cp u-boot u-boot.elf
cd -
