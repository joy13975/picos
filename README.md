PICOS
-----

A fast, lightweight checkpoint & restart library. PCRL?
- [x] In-RAM selective data backup
- [X] Recovery from RAM backup
- [x] On-disk selective data backup
- [X] Recovery from disk backup
- [ ] Full program state backup (involves kernel code)

![alt tag](https://github.com/joy13975/picos/blob/master/README_res/knl_overheads.png)

Installaion
-----------
* Without MPI support
```
make install PREFIX=<your_library_prefix>
```

* With MPI support
```
make install PREFIX=<your_library_prefix> CC=mpicc DEF=-DUSE_MPI
```

Note: by default, ```make install``` will NOT install to your root.
