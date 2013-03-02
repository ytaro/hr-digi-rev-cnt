TARGET = testA86
CFLAGS= -O -mh -mint32 -g
CC = h8300-elf-gcc
all : $(TARGET).mot

$(TARGET).mot : $(TARGET).elf
	h8300-elf-objcopy -O srec $(TARGET).elf $(TARGET).mot
	rm $(TARGET).elf

$(TARGET).elf : $(TARGET).c
	$(CC) $(CFLAGS) -T ram3664.x -nostartfiles ramcrt0.S $(TARGET).c -o $(TARGET).elf -lc -Xlinker -Map -Xlinker $(TARGET).map

clean :
	rm -f $(TARGET).mot 

rom.mot : rom.elf
	h8300-elf-objcopy -O srec rom.elf rom.mot
	rm rom.elf
	copy kern3664.mot+rom.mot+testA86.mot romA86.mot

rom.elf : $(TARGET).c
	$(CC) $(CFLAGS) -T rom3664.x -nostartfiles 3664crt0.S $(TARGET).c -o rom.elf -lc -Xlinker -Map -Xlinker rom.map
