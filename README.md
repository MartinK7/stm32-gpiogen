# STM32-GPIOGEN
GPIO C-Init code generator from CSV description file

## Compilation

```console
mkdir build
cd build
cmake ..
make
```

or simple one:

Unix(like) OS: 
```console
gcc main.c -o stm32_gpiogen
```
Windows:
```console
gcc main.c -o stm32_gpiogen.exe
```

## Usage

```console
./stm32_gpiogen [INPUT] [OUTPUT]
```

Unix(like) OS: 
```console
./stm32_gpiogen easy.csv my_config.c
```
Windows:
```console
stm32_gpiogen.exe easy.csv my_config.c
```
