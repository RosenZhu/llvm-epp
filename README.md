[![Build Status](https://travis-ci.org/snehasish/llvm-epp.svg?branch=master)](https://travis-ci.org/snehasish/llvm-epp)

## llvm-epp 
Efficient Path Profiling using LLVM 

## Requires 

1. LLVM 5.0
2. gcc-5+

## Build 

1. `mkdir build && cd build`
2. `cmake -DCMAKE_BUILD_TYPE=Release .. && make -j 8`
3. `sudo make install`

## Test

To run the tests, install [lit](https://pypi.python.org/pypi/lit) from the python package index. 

1. `pip install lit`
2. `cd build`
3. `lit test`  

## Documentation

To generate documentation, install [graphviz](http://www.graphviz.org/) and [doxygen](http://www.stack.nl/~dimitri/doxygen/). Running `cmake` with these prerequisites will enable the `doc` target for the build system. Running `make doc` will generate html documentation of the classes.  

## Usage

```
clang -c -g -emit-llvm prog.c \
&& llvm-epp prog.bc -o path-profile.txt \
&& clang prog.epp.bc -o exe -lepp-rt \
&& ./exe \
&& llvm-epp -p=path-profile.txt prog.bc 
```

## Known Issues 

1. ~~Instrumentation cannot be placed along computed indirect branch target edges. [This](http://blog.llvm.org/2010/01/address-of-label-and-indirect-branches.html) blog post describes the issue under the section "How does this extension interact with critical edge splitting?".~~ LLVM can now split indirect jump edges. I have not tested this yet.  

## Roadmap

1. Re-enable optimal event counting  
2. Add benchmarking hooks  
3. Add unit tests  

## License 

The MIT License

