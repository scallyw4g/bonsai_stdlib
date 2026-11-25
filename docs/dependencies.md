

# Dependencies

Projects making use of bonsai_stdlib require `clang` and a unix-like shell to build.

## WSL2
`sudo apt install clang build-esential libgl-dev libx11-dev libxcb1-dev x11-apps`

## [MSYS2 ](https://www.msys2.org/)
`pacman -S mingw-w64-clang-x86_64-clang`

### NOTES on using MSYS2

When launching MSYS2, make sure to launch the `MSYS2 CLANG64` variant.  It has
`/clang64/bin` added to your path.  If you launch another variant, you'll want
to add `PATH=$PATH:/clang64/bin` to your `~/.bashrc` file.  I do this in any case.

The C drive is mounted at `/c`.  You can access your users files at `/c/users/<username>`.

## Ubuntu
`sudo apt install libx11-dev clang`

## Archlinux
`sudo pacman -S libx11 clang`
