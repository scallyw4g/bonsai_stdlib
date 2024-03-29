

# Dependencies

Projects making use of bonsai_stdlib require `clang` and a unix-like shell to build.

# Windows:
Installing [MSYS2 ](https://www.msys2.org/) is an easy way to satisfy the required dependencies.

## MSYS2

`pacman -S mingw-w64-clang-x86_64-clang`

### NOTES on using MSYS2

When launching MSYS2, make sure to launch the `MSYS2 CLANG64` variant.  It has
`/clang64/bin` added to your path.  If you launch another variant, you'll want
to add `PATH=$PATH:/clang64/bin` to your `~/.bashrc` file.  I do this in any case.

The C drive is mounted at `/c`.  You can access your users files at `/c/users/<username>`.




# Linux:
Ensure you have X11 headers, and clang.  I'll leave the OpenGL driver as an excercise for the reader (it depends on your hardware).

## Ubuntu
`sudo apt install libx11-dev clang-15`

## Archlinux
`sudo pacman -S libx11 clang # NOTE(Jesse): Not sure how to specifically install clang 15 on Arch`
