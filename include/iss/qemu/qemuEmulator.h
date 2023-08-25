#ifndef QEMUEMULATOR_H
#define QEMUEMULATOR_H

extern int qemuUserEmulator(int argc, char **argv, char **envp);
extern int qemuSystemEmulator(int argc, char **argv);

#endif /* QEMU_MAIN_H */
