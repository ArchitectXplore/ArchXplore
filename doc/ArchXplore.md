# ArchXplore : A hierarchical simulation infrastructure for quantified microarchitecture design, analysis and optimization

## Introduction
RISC-V, as a open source standard instruction set architecture(ISA), has gained popularity because the architecture provides simplified instructions to accomplish various tasks and is flexible to create custom extensions to satisfy different kinds of requirements. After one decade's development, various of RISC-V processor IPs have emerged along with establishment of plenty of companies and laboratories aiming at processors design and research based on RISC-V. 

However, since slowing down of the Moore law, it's increasingly difficult to improve processors' performance via development of semi-conductor technology. In this scenario, quality of microarchitecture plays a more important role in performance boosting. To competing with processors based on other ISAs like ARM, X86 and so on, a new methodology is need to be leveraged to boost creation and optimization for RISC-V processors, especially in open-source community. 

In this work, we present ArchXplore, a hierarchical simulation infrastructure with quantified methods for processors' creation. The novel methodology and strategies are leveraged in processors development flow for workload characterization microarchitecture design, performance analysis and design space exploration(DSE) with direction of data. 

## Methodology

Designing of Modern processor, aims at finding the optimal microarchitecture within the limitation of power, area, thermal and time to achieve best performance for the target workloads.

In this process, some kinds of tools are employed to make different levels of prototype of RISC-V processors :



| Type                        | Tools               |
| --------------------------- | ------------------- |
| ISA Emulator                | QEMU, RV8           |
| ISA Emulator                | QEMU, RV8           |
| ISA Simulator               | Spike, Dromajo      |
| Cycle Approximate Simulator | Sniper, ESESC, ZSim |
| Cycle Accurate Simulator    | Gem5, SST           |

In the process, we have three observations that make microarchitecture challengeable :

**Observation 1** :

**Observation 2** : 

**Observation 3** : 



**Observation 1** : 

## Methods

### ISA Simulation

### Cycle Approximate Simulation

#### Microarchitecture-Independent Characterizations

### Cycle Accurate Simulation

### Workload Characterizations
#### Microarchitecture-Independent Characterizations
#### Microarchitecture-Dependent Characterizations

## Putting All Together

## Conclusion


| Component                                | Status                  |
|------------------------------------------|-------------------------|
| Simulation Framework                     | Bridge QEMU and Timing Simulator (READY)     |
|                                           | Parallel simulation framework                    |
| Multicore (Core simulation) accelerating  | (READY)                 |
| Multicore (Uncore simulation) accelerating | (Planning)              |
| Timing simulation framework               | Runtime configuring       |
|                                           | Sparta-Python support (Working in Progress)   |
| Core simulation                           | Cycle approximate model (Working in Progress) |
|                                           | Cycle accurate model (Planning)               |
| Uncore simulation                         | Gem5 cache/memory model (READY)                |
|                                           | Sparta cache/memory model (Working in Progress) |
| Characterization & Performance Monitoring Architecture |
| Microarchitecture-dependent Characterizations | (Ready)            |
| Workload-dependent Characterizations       | (Working in Progress)   |
| Documentation                            | (Working in Progress)   |
