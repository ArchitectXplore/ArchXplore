
all : qemuLinux_test

libqemuInterface_plugin.so:
	g++ -shared -fPIC -O3 -I../../include -o libqemuInterface_plugin.so ../../src/iss/qemu/qemuInterface_plugin.cpp ../../src/iss/qemu/qemuInterface.cpp

qemuLinux_test: clean libqemuInterface_plugin.so
	g++ -fPIE -O3 -o qemuLinux_test qemuLinux_test.cpp -I../../include -L. -L../../build/qemu -lqemuInterface_plugin -lqemu-system-riscv64

run_test: 
	./qemuLinux_test -nographic -machine virt -m 2048 -smp 4 \
		-plugin libqemuInterface_plugin.so \
		-bios /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin \
		-kernel /usr/lib/u-boot/qemu-riscv64_smode/uboot.elf \
		-drive file=ubuntu-23.04.qcow2,format=qcow2,if=virtio \
		-monitor telnet::45454,server,nowait \
		-serial telnet::45455,server,nowait \
		-loadvm start_4_icount \
		-icount shift=auto

run_qemu: 
	qemu-system-riscv64 -nographic -machine virt -m 2048 -smp 4 \
		-bios /usr/lib/riscv64-linux-gnu/opensbi/generic/fw_jump.bin \
		-kernel /usr/lib/u-boot/qemu-riscv64_smode/uboot.elf \
		-drive file=ubuntu-23.04.qcow2,format=qcow2,if=virtio \
		-monitor telnet::45454,server,nowait \
		-serial telnet::45455,server,nowait \
		-icount shift=auto

.PHONY: clean

clean:
	rm -rf libqemuInterface_plugin.so qemuLinux_test libqemuInterface.a
