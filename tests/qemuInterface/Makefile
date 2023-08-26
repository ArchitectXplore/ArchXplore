
all : riscv_hello libqemuInterface_plugin.so qemuInterface_test

riscv_hello:
	riscv64-linux-gnu-g++ -fPIE -O3 -o hello hello.cpp

libqemuInterface_plugin.so:
	g++ -shared -fPIC -O3 -I../../include -o libqemuInterface_plugin.so ../../src/iss/qemu/qemuInterface_plugin.cpp ../../src/iss/qemu/qemuInterface.cpp

qemuInterface_test:
	g++ -fPIE -O3 -o qemuInterface_test qemuInterface_test.cpp -I../../include -L. -lqemuInterface_plugin -lqemu-riscv64 -D CONFIG_USER_ONLY 

run: 
	./qemuInterface_test -plugin libqemuInterface_plugin.so hello

.PHONY: clean

clean:
	rm -rf hello libqemuInterface_plugin.so qemuInterface_test libqemuInterface.a
