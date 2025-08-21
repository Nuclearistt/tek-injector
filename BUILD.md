# Building tek-injector

tek-injector must be built in [MSYS2](https://www.msys2.org/), preferably in its CLANG64 environment, which is assumed to be used in the following steps

## 1. Install requirements

```sh
pacman -S base-devel git mingw-w64-clang-x86_64-cc mingw-w64-clang-x86_64-meson
```

## 2. Get source code

Clone this repository:
```sh
git clone https://github.com/teknology-hub/tek-injector.git
cd tek-injector
```
, or download a point release e.g.
```sh
curl -LOJ https://github.com/teknology-hub/tek-injector/releases/download/v2.0.0/tek-injector-2.0.0.tar.gz`
tar -xzf tek-injector-2.0.0.tar.gz
cd tek-injector-2.0.0
```

## 3. Setup build directory

At this stage you can set various build options, which are described in [Meson documentation](https://mesonbuild.com/Commands.html#setup). Release builds use the following setup:
```sh
CXXFLAGS="-pipe -fomit-frame-pointer" meson setup build --buildtype debugoptimized --prefix /clang64 --default-library=both --default-both-libraries=static -Dprefer_static=true -Db_lto=true -Db_lto_mode=thin -Db_ndebug=true -Dstrip=true
```

## 4. Compile and install the project

```sh
meson install -C build
```
This will produce the binaries in /clang64/bin, library files in /clang64/lib, and install the header in /clang64/include. Build directory will also contain the PDB files for the binaries
