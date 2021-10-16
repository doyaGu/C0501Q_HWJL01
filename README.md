# C0501Q_HWJL01
The related resources for this device.



## Photo

![PCB Front](\doc\images\pcb-front.jpg)



## Hardware Spec

### POWER & USB

| Header Pinout | Pinout | Pin Number |
| :-----------: | :----: | :--------: |
|      D+       |  DP0   |   PIN122   |
|      D-       |  DM0   |   PIN123   |
|      GND      |  GND   |     -      |
|      RX       | GPIO23 |   PIN31    |
|      TX       | GPIO22 |   PIN30    |
|      VCC      |  VCC   |     -      |



### SPI & Debug

| Header Pinout | Debugger Pinout | Pinout | Pin Number |
| :-----------: | :-------------: | :----: | :--------: |
|      CLK      |       TCK       | GPIO0  |    PIN6    |
|      CSN      |       TMS       | GPIO1  |    PIN7    |
|     MOSI      |       TDI       | GPIO2  |    PIN8    |
|     MISO      |       TDO       | GPIO3  |    PIN9    |
|      GND      |       GND       |   -    |     -      |
|     DB_TX     |       RxD       | GPIO4  |   PIN10    |
|      3V3      |       VCC       |   -    |     -      |
|      NC       |       NC        |   -    |     -      |



### LCD: DX050H049

| No. | LCM Pinout | Pinout | Pin Number |
| :-----------: | :-----------: | :----: | :--------: |
| 1 |     LEDK      |  GND   |     -      |
| 2 |     LEDA      |   5V [Controlled by GPIO38]   |     PIN63 |
| 3 |      GND      |  GND   |     -      |
| 4 |      VDD      |  3V3 [Controlled by GPIO38]  |     PIN63 |
| 5 | R7 | LD23 (GPIO39) | PIN65 |
| 6 | R6 | LD22 (GPIO40) | PIN67 |
| 7 | R5 | LD21 (GPIO41) | PIN68 |
| 8 | R4 | LD20 (GPIO42) | PIN69 |
| 9 | R3 | LD19 (GPIO43) | PIN70 |
| 10 | R2 | LD18 (GPIO44) | PIN71 |
| 11 | R1 | LD17 (GPIO45) | PIN72 |
| 12 | R0 | LD16 (GPIO46) | PIN74 |
| 13 | G7 | LD15 (GPIO47) | PIN75 |
| 14 | G6 | LD14 (GPIO48) | PIN76 |
| 15 | G5 | LD13 (GPIO49) | PIN77 |
| 16 | G4 | LD12 (GPIO50) | PIN78 |
| 17 | G3 | LD11 (GPIO51) | PIN79 |
| 18 | G2 | LD10 (GPIO52) | PIN81 |
| 19 | G1 | LD9 (GPIO53) | PIN82 |
| 20 | G0 | LD8 (GPIO54) | PIN83 |
| 21 | B7 | LD7 (GPIO55) | PIN84 |
| 22 | B6 | LD6 (GPIO56) | PIN85 |
| 23 | B5 | LD5 (GPIO57) | PIN86 |
| 24 | B4 | LD4 (GPIO58) | PIN87 |
| 25 | B3 | LD3 (GPIO59) | PIN88 |
| 26 | B2 | LD2 (GPIO60) | PIN89 |
| 27 | B1 | LD1 (GPIO61) | PIN90 |
| 28 | B0 | LD0 (GPIO62) | PIN91 |
| 29 | GND | GND | - |
| 30 | DOTCLK | LDCLK (GPIO66) | PIN99 |
| 31 | NC | NC | - |
| 32 | HSYNC | LHSYNC (GPIO67) | PIN99 |
| 33 | VSYNC | LVSYNC (GPIO68) | PIN100 |
| 34 | DE | LDEN (GPIO65) | PIN96 |
| 35 | NC? | NC? | - |
| 36 | GND | GND | - |
| 37 | RESET | GPIO64 | PIN95 |
| 38 | SDA | GPIO69 | PIN94 |
| 39 | SCL | GPIO70 | PIN62 |
| 40 | CS | GPIO71 | PIN103 |



### CTP: HY4633

| Header Pinout | Pinout | Pin Number |
| :-----------: | :----: | :--------: |
|   IIC1_SDA    | GPIO25 |   PIN34    |
|   IIC1_SCL    | GPIO26 |   PIN35    |
|      INT      | GPIO27 |   PIN36    |
|      RST      | GPIO28 |   PIN37    |



### SPI-NAND: 5F1GQ4UBYIG

| Header Pinout | Pinout | Pin Number |
| :-----------: | :----: | :--------: |
|   SPI0_CS#    | GPIO14 |   PIN21    |
|   SPI0_DIN    | GPIO18 |   PIN25    |
|   SPI0_DOUT   | GPIO19 |   PIN26    |
|   SPI0_CLK    | GPIO20 |   PIN27    |



### WIFI: MTW38266G

| Header Pinout | Pinout | Pin Number |
| :-------: | :----: | :--------: |
| GND | GND | - |
| RST | NC | - |
|   DP/RX   | DP1 | PIN125 |
|   DM/TX   | DM1 |   PIN126    |
|   VCC  | VCC | - |



### RTC: BL5372

| Header Pinout | Pinout | Pin Number |
| :-----------: | :----: | :--------: |
|   IIC1_SDA    | GPIO25 |   PIN34    |
|   IIC1_SCL    | GPIO26 |   PIN35    |
|     INTRB     | GPIO29 |   PIN38    |

