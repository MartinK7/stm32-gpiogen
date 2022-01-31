# STM32-GPIOGEN
GPIO C-Init code generator from CSV description file

## Example
Input CSV file:
```
FMC,B,5,alt,pp,high,no,0xC,,,,,,
FMC,B,6,alt,pp,high,no,0xC,,,,,,
```
Generated C init code:
```
//                  MYPINA         MYPINB
GPIOB->AFR[0]  &= ~(0xFU<<( 5-0)*4|0xFU<<( 6-0)*4|0);
GPIOB->AFR[0]  |=  (0xCU<<( 5-0)*4|0xCU<<( 6-0)*4|0);
GPIOB->AFR[1]  &= ~(0             |0             |0);
GPIOB->AFR[1]  |=  (0             |0             |0);
GPIOB->MODER   &= ~(0x3U<<( 5*2)  |0x3U<<( 6*2)  |0);
GPIOB->MODER   |=  (0x2U<<( 5*2)  |0x2U<<( 6*2)  |0);
GPIOB->OSPEEDR &= ~(0x3U<<( 5*2)  |0x3U<<( 6*2)  |0);
GPIOB->OSPEEDR |=  (0x2U<<( 5*2)  |0x2U<<( 6*2)  |0);
GPIOB->OTYPER  &= ~(0x1U<<( 5*1)  |0x1U<<( 6*1)  |0);
GPIOB->OTYPER  |=  (0x0U<<( 5*1)  |0x0U<<( 6*1)  |0);
GPIOB->PUPDR   &= ~(0x3U<<( 5*2)  |0x3U<<( 6*2)  |0);
GPIOB->PUPDR   |=  (0x0U<<( 5*2)  |0x0U<<( 6*2)  |0);
```


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
