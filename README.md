PICOS
-----

A fast, lightweight checkpoint & restart library. PCRL?
- [x] In-RAM selective data backup
- [X] Recovery from RAM backup
- [x] On-disk selective data backup
- [X] Recovery from disk backup
- [ ] Full program state backup (involves kernel code)

![alt tag](https://github.com/joy13975/picos/blob/master/README_res/overheads_knl.png)

Installaion
-----------

* With MPI support
```
make install PREFIX=<your_library_prefix>
```

* Without MPI support
```
make install PREFIX=<your_library_prefix> CC=gcc DEF=
```

Note: by default, ```make install``` will NOT install to your root.
