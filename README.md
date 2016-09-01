PICOS
-----

A fast, lightweight Checkpoint-Restart library.
- [x] In-RAM selective data backup
- [X] Recovery from RAM backup
- [x] On-disk selective data backup
- [X] Recovery from disk backup
- [ ] Full program state backup (involves kernel code)

Installaion
-----------
```
git clone git@github.com:joy13975/picos.git
cd picos
make install PREFIX=<your_library_prefix>
```

Note that by default, ```make install``` will NOT install to your root.
