externals for mcp3208 + GPIO / terminal tedium
===========================================================

*usage:*


**ADC:**

```
[open /dev/spidev0.1(
|
[terminal_tedium_adc]
|  |  |  |  |  |  |  | 
[ADC0]  [ADC1]  	[etc]
```
"open /dev/spidev0.1" opens the device. reads ADC when banged.
 
**gate outputs:**

```
              o      o
              |      |
   [tedium_output 0 0]

```
left inlet: nc / second inlet: gate #1 (top) / right inlet: gate #2 (bottom). 

sending "1" turns the gate on, sending "0" off; the two arguments determine the initial state (0 = off, 1 = on).

**gate/switch inputs:** 

```
[tedium_input <GPIO_num>] 
|
o
```
where GPIO_num = 4, 17, 2, 3, 23, 24, or 25. outputs bang.

====================================================================================


*compile with:*

`gcc -O3 -Wall -c [name_of_external].c -o [name_of_external].o`

gcc -O3 -Wall -c tedium_input.c -o tedium_input.o
gcc -O3 -Wall -c tedium_output.c -o tedium_output.o


`ld --export-dynamic -shared -o [name_of_external].pd_linux [name_of_external].o -lc -lm -lwiringPi`

ld --export-dynamic -shared -o tedium_input.pd_linux tedium_input.o -lc -lm -lwiringPi

ld --export-dynamic -shared -o tedium_output.pd_linux tedium_output.o -lc -lm -lwiringPi


then move into externals folder, eg: 

`sudo mv [name_of_external].pd_linux /usr/lib/pd/extra/`
sudo cp tedium_input.pd_linux ~/pd-externals
sudo mv tedium_output.pd_linux ~/pd-externals


 sudo puredata -path ~/pd-externals -noadc -nogui -rt D_outtest.pd