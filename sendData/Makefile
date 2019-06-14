CC=/work/Atheros_CSI_tool_OpenWRT_src/staging_dir/toolchain-mips_24kc_gcc-7.3.0_musl/bin/mips-openwrt-linux-musl-gcc
target=sendData

${target}: sendData.c
	${CC} -o $@ sendData.c

clean:
	rm -f ${target}
