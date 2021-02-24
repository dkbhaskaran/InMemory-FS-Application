In Memory Application
=====================

An application to evaluate performances of in-house and upstream patch
(proposed at https://lists.llvm.org/pipermail/cfe-dev/2021-January/067576.html).


Build
-----

Build LLVM with IN_HOUSE_PATCH/UPSTREAM_PATCH and enable macro
IN_HOUSE_PATCH or UPSTREAM_PATCH in InMemoryApp.cpp accordingly. Then

$ mkdir build; cd build
$ cmake -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="<path_to_llvm_build>" ..
$ make
