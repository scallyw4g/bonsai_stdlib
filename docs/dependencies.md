
# Bonsai Stdlib


## Dependencies

## On Windows:
Projects making use of bonsai_stdlib require `clang` and a unix-like shell to build on Windows.
Installing [MSYS2 ](https://www.msys2.org/) satisfies all the required dependencies.

## On Linux:
Ensure you have an OpenGL driver, GL headers, X11 headers, and clang

### Ubuntu
`sudo apt install freeglut3-dev libx11-dev clang-15`

### Archlinux
`sudo pacman -S freeglut libx11 clang` # NOTE(Jesse): Not sure how to specifically install clang 15 on Arch
