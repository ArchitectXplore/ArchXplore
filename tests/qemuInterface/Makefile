
all : qemuInterface_user_test

riscv_hello:
	riscv64-linux-gnu-g++ -fPIE -O3 -o hello hello.cpp

libqemuInterface_plugin.so:
	g++ -shared -fPIC -g -I../../include -o libqemuInterface_plugin.so ../../src/iss/qemu/qemuInterface_plugin.cpp ../../src/iss/qemu/qemuInterface.cpp

qemuInterface_user_test: clean libqemuInterface_plugin.so
	g++ -fPIE -g -o qemuInterface_test qemuInterface_test.cpp -I../../include -L. -lqemuInterface_plugin -lqemu-riscv64 -D CONFIG_USER_ONLY 

qemuInterface_sys_test: clean libqemuInterface_plugin.so
	g++ -fPIE -g -o qemuInterface_test qemuInterface_test.cpp -I../../include -L. -lqemuInterface_plugin -lqemu-system-riscv64

run_user_test: 
	./qemuInterface_test -plugin libqemuInterface_plugin.so hello

run_sys_test: 
	./qemuInterface_test -machine spike -plugin libqemuInterface_plugin.so -nographic -bios rv32ui-p-add

.PHONY: clean

clean:
	rm -rf libqemuInterface_plugin.so qemuInterface_test libqemuInterface.a
