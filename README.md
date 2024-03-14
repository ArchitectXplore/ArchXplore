# ArchXplore

## Table of Contents

- [About](#about)
- [Getting Started](#getting_started)
- [Usage](#usage)
- [Contributing](../CONTRIBUTING.md)

## About <a name = "about"></a>

ArchXplore is a simulation infrastructure for research and development in the field of computer architecture. It provides a platform to simulate and analyze microarchitecture designs, and to explore the impact of different design choices on system performance.

Compared to traditional simulation tools, ArchXplore offers several unique features:

- **Configurability**: ArchXplore provides a flexible and powerful way to configure, composite modules and build topologies with auto-complete in IDEs. 

- **Performance**: ArchXplore leverages dynamic binary translation(QEMU) to achieve high performance. It also supports fine-grained parallel simulation for multi-threaded applications.

- **Accuracy**: ArchXplore provides the ability to trade off performance for accuracy. With sparta, a discrete event simulation framework, it is possible to simulate microarchitectures with high fidelity and high precision.

## Getting Started <a name = "getting_started"></a>

### Prerequisites

Ubuntu 22.04 is recommended.

```
sudo apt install ninja-build libglib2.0-dev
```

### Installing

* If conda is not installed, install it using the following command:

```
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh

bash Miniconda3-latest-Linux-x86_64.sh 
```

* Install jq and yq using the following commands:

```
conda install -c conda-forge jq yq
```

* Clone the repository:

```
git clone https://github.com/ArchitectXplore/ArchXplore.git

cd ArchXplore
```

* Build conda environment:

```
cd ext/map/scripts/create_conda_env.sh <Your environment name> run 

conda activate <Your environment name>
```

* Build ArchXplore:

```
mkdir build && cd build

cmake ..

make -j$(nproc)
```

## Usage <a name = "usage"></a>

* Start inter process communication server:

```
./IPCServer &
```

* Demonstrate the usage of ArchXplore by running a simple CPU simulation:

```
./ArchXplore ../configs/simpleCPU.py
```

* The output should be similar to the following:

```
Host time elapsed(s):  0.8125679176300764
Guest time elapsed(s):  0.001
Total instructions executed:  43248512
Million instructions per second:  53.22448876167542
```

## Features
- [x] Support python configuration files for simulation.
- [x] Support for multi-threads simulation.
- [x] Support for multi-processes simulation.
- [ ] Integrate DRAMSim3 for cycle accurate memory modeling.
- [ ] Design modeling framework for coherent caches.
- [ ] Design modeling framework for network on chip(NoC).
- [ ] Top-down analysis framework for microarchitecture exploration.
- [ ] Pthread/OpenMP API instrumentation for synchronization-aware multi-core simulation.