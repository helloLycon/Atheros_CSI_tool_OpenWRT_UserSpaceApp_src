#INCLUDE_DIR=$(TOOLCHAIN_DIR)/usr/include
OBJS=csi_fun.o main.o
CC=/work/Atheros_CSI_tool_OpenWRT_src/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin/mips-openwrt-linux-musl-gcc

CFLAGS= -std=gnu99

recvCSI: $(OBJS)
	$(CC) $(OBJS) -o recvCSI

csi_fun.o: csi_fun.c csi_fun.h
	$(CC) -c $(CFLAGS) -o $@ $<

recvCSI.o: main.c csi_fun.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f *.o recvCSI

