# libcow
Automatic copy-on-write semantic memory slices for use in C (and C++)

# Usage
See `include/cow.h` for documentation on each function.

## C API 
Each function, macro, and type definition in the header will be prefixed with `cow_` or `COW_`. Internal non-prototpyed items use the namespace `_cow_` or `_COW_`.

### C++ wrapper API
The C++ interface defines the type `Cow`, a reference-counted wrapper over `cow_t` instances that supports cloning through its subtype, `Cow::Fake`, and automatically ensures the originally created `cow_t` is not destroyed until all its clones are, as well as the namespace `_cow_util` which contains memory accessor helpers `Span<T>` and `Slice<T>` (aka `Span<T>::Slice`).

There are also the following:
 * `cow/area.hpp` (namespace `_cow_util`) - The `Area` type is a copy-constructable wrapper around *both* `Cow` and `Cow::Fake`, allowing for implicit cloning.
 * `cow/slice.hpp` (namespace `_cow_util`) - Contains the definitions for `Span<T>` and `Slice<T>`. Included automatically by `cow.hpp` (*see above*).

## Building
Run `make` to build to build the `release` (optimised) target of the library.
It will create four files: `libcow-release.a`, `libcow-release.so`, `libcow.a`, and `libcow.so`.
The latter two are just symlinks to the former two.

Run `make debug` to build the debug target, which disables optimisations and includes trace messages.
It will create two files: `libcow-debug.a` and `libcow-debug.so`.

Each target compiles both a static and dynamic library. You may need to run `make clean` before switching build targets.
To build both targets, run `make all`.

To disable default target-specific (e.g. optimisation) flags, set `TARGET_SPEC_FLAGS=no` when running `make`.

Run `sudo make install` to install the libraries (static and dynamic) and header files (C and C++). 
Run `sudo make uninstall` to remove the libraries and header files.

By default, the install target is `/usr/local/`. Set the `PREFIX` variable when running `make install` / `make uninstall` to specify a different path.

### Full build and installation
```shell
$ make && sudo make install
```

### Notes
* The `release` target specifies `-march=native` by default. This may be undesirable, if so, run `make MARCH="" release` instead.
* Many optimisation flags for the `release` configuration are specific to GCC (with graphite enabled by default), if builds on other compilers (or non-graphite enabled GCC builds) complain, either set the `OPT_FLAGS` env var or remove the problem flags from the Makefile.
* `release` builds are stripped by default. run `make STRIP=: release` to prevent stripping.
* The targets are all built with `-fno-strict-aliasing`, but functions in the header file are still annotated with `restrict` needed. This is just to inform users that the function will assume the pointer is not aliased. (When included in C++, where `restrict` is not a keyword, we temporarily define it to be `__restrict__`, which is the GCC equivalent for C++).

## Using
Link to either `libcow.a` or `libcow.so` (or the debug target libraries), and include the header `include/cow.h` to your project to use this library.
The header should work in C++ projects as well.



# Requirements
Relying on the `memfd_create()` syscall, Linux >=3.17 and glibc >=2.27 (or equivalent) are required for build.
Makefile is tuned towards `gcc` but with some small modifications should work with gcc-similar compilers such as `clang`, although this is unintended.

The code itself uses GCC extensions and is targeted at the `gnu11` (C11 with GNU extensions) standard.
I have no plan on making this portable at all, either for non-Linux platforms or for ISO C compilers. 
(GNU C is superior to ISO C and thankfully the most widely used compilers accept this).

# Example
Non-propagation from clones to the origin:
``` c
#include <cow.h>

#define SIZE 4096

int main()
{
	cow_t* origin = cow_create(SIZE);
	
	strcpy(cow_ptr(origin), "Hello world");
	cow_t* fake = cow_clone(origin);
	printf("Fake (pre write): %s\n", cow_ptr_of(const char, fake));
	strcpy(cow_ptr(fake), "Hello fake!");

	printf("Real: %s\n", cow_ptr_of(const char, origin));
	printf("Fake: %s\n", cow_ptr_of(const char, fake));
	cow_free(fake);
	cow_free(origin);

	return 0;
}

```
Will print:

``` shell
$ ./test
Fake (pre write): Hello world
Real: Hello world
Fake: Hello fake!
```
Notice the first read of `fake` contains the data written to `origin`. And that the write of `Hello fake!` caused only `fake` to be updated, not `origin`.

## What is happening here?
The cloned slice, `fake`, which is created from `origin` with the `cow_clone()` function will contain all the information within `origin`. 
The cloned slice can be written to, however, those writes will only be visible to that specific instance of `cow_t`, even if that `cow_t*` is again `cow_clone()`d.

Original or cloned ('fake') slices can be cloned to produce the same effect of a memory slice that starts out containing whatever data is in the original slice created with `cow_create()`, and can be modified to produce a automatically and lazily copied slice when written to, to which that instance only will have the results of the write operation.

Each fake slice's data will first appear as the original slice that it comes from. Writing to an original slice will propagate the write to all future clones of that slice, or clones of clones of the slice.
The function `cow_is_fake()` can be used to determine if a slice will not propagate its writes to its children.

# License
MIT

(this code is not valuable enough to be GPL'd).
