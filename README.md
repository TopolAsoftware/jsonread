### JSON read util

This small util reads json file, parse it and prints it
with current command string localization.

Util uses code of "faster json parser" of Serge Zaitsev.

## Build Util

Util depends only from libc and you hace not any affitional libs to be installed. To build util just run commands:

```sh
$ make clean && make
$ sudo cp jsonread /usr/local/bin/
```
