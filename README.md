# pnml2pins
pnml2pins is a language front-end for the LTSmin model checking tool set. pnml2pins uses the dlopen interface of LTSmin.
## Dependencies
 - `make`: https://www.gnu.org/software/make
 - `Python >= 3`: https://www.python.org/download/releases/3.0
 - `LTSmin >= 2.1`: http://fmt.cs.utwente.nl/tools/ltsmin
 - `GCC >= 4.8`: https://gcc.gnu.org/. Other C compilers may work as well.
 
## Installation
To install pnml2pins run the following commands:

1. `autoreconf -i` to create the configure script
2. `./configure` to configure pnml2pins. Provide the option `--with-LTSmin=<LTSmin-prefix>` for a different LTSmin prefix. Supply the `--prefix=<prefix` for a different install prefix.
3. `make` to build the compilation script
4. `make install` to install pnml2pins

## Usage

1. `pnml2pins <petri-net.pnml>` to compile the Petri net into a shared library. The result will be an .so file produced in the PWD.
2. `pins2lts-sym <petri-net.so>` to perform reachability analysis of the Petri net with the *symbolic* back-end.

## Features
### Petri net categories:
 - Unfolded Petri nets
 - 1-safe Petri nets: By means of the [NUPN](http://mcc.lip6.fr/nupn.php) toolspecific syntax

### Reachability features:
 - Sequential explicit (`pins2lts-seq`)
 - Multi-core explicit (thread safe) (`pins2lts-mc`) 
 - (Mulit-core) Symbolic (Disjunctive Paritioned Transitions, thread safe) (`pins2lts-sym`)
 - Distributed explicit (`pins2lts-dist`)
 
### Reduction techniques:
 - Partial Order Reduction: [The state explosion problem](http://dx.doi.org/10.1007/3-540-65306-6_21) (`pins2lts-[seq|mc] --por`)

### Other features:
 - Transition labels for traces/witnesses (`pins2lts-* --action=<transition-name>`)
 - Print maximum token count over all places
 - Token count overflow detection
 - Invariants/LTL/mu-calculus formulas over places (`pins2lts-* --invariant=<place=#token-count>`)

## ToDo
 - State labels
