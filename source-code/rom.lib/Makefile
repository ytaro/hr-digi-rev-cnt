TARGET = test85
CFLAGS= -O -mh -mint32 -g
CC = h8300-elf-gcc
all : $(TARGET).mot
#OBJS = $(TARGET).c
OBJS = $(TARGET).c timerA.c timerW.c lcd.c

$(TARGET).mot : $(TARGET).elf
	h8300-elf-objcopy -O srec $(TARGET).elf $(TARGET).mot
	rm $(TARGET).elf

$(TARGET).elf : $(OBJS)
	$(CC) $(CFLAGS) -T rom-lib.x -nostartfiles ramcrt0.S $(OBJS) -o $(TARGET).elf -lc -Xlinker -Map -Xlinker $(TARGET).map -Wall

clean :
	rm -f $(TARGET).mot

