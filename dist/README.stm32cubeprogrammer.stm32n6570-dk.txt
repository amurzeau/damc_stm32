# How to program using STM32CubeProgrammer

 - Open STM32CubeProgrammer.
 - On the left, click "External Loaders" and enable the STM32N6570-DK NOR-FLASH loader (its name should be "MX66UW1G45G_STM32N6570-DK").
 - In ST-LINK configuration on the right, select "Mode: Hot plug" and "Reset mode: Software Reset"
 - On the board, select the DEV MODE boot using BOOT0 and BOOT1 switches:
   - Put BOOT1 toward the outside of the PCB
   - Put BOOT0 toward the LCD screen
 - Connect to the board in STM32CubeProgrammer
 - On the left, click the second icon "Erasing & Programing"
 - In Download, File path, select the "damc_stm32n6570-dk-flash.bin" file
 - Click "Start Programm..." to flash the board

 - When programming is done, put the BOOT1 switch toward the LCD screen
 - Then reboot the board (reset or power cycle)

See also instruction here (but don't forget to enable the "External loader"):
https://docs.nanoframework.net/content/stm32/flash-cube-programmer.html
