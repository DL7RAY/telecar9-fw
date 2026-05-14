# ====================
# TC9 Firmware Builder
# ====================

# Please set the version of the firmware to the wanted one
VERSION ?= 15

# 0: keeps all the generated files;
# 1: keeps only the final binaries
CLEAR_INTERM_FILES ?= 1

CC = sdcc
SREC = srec_cat

CFLAGS = -DVERSION=$(VERSION) --code-size 0x4000 --no-xinit-opt --xram-loc 0x8000

BUILD = build
BIN = bin
SRC = src/tc9main.c


# =========================
# Default build
# =========================

all: check-toolchain dirs TC4M TC2M TC70CM TC2MB TC70CMB


# =========================
# Toolchain detection
# =========================

check-toolchain:
	@command -v sdcc >/dev/null 2>&1 || { \
	    echo "SDCC 8051 compiler not found!"; \
	    echo "Install it: sudo apt install sdcc"; \
	    exit 1; \
	}
	@command -v srec_cat >/dev/null 2>&1 || { \
	    echo "srec_cat tool for found!"; \
	    echo "Install it: sudo apt install srecord"; \
	    exit 1; \
	}


# =========================
# Create directories
# =========================

dirs:
	mkdir -p $(BUILD) $(BIN)


# =========================
# 4m - TC9 variant
# =========================

TC4M: check-toolchain
	$(CC) -DTC4M $(CFLAGS) $(SRC) -o $(BUILD)/tc4m.ihx
	$(SREC) $(BUILD)/tc4m.ihx -intel -random-fill 0x0 0x4000 \
	    -exclude 0x26 0x28 -Checksum_Negative_Little_Endian 0x26 2 2 \
	    -o $(BIN)/tc4m-V$(VERSION).bin -binary
	cat $(BIN)/tc4m-V$(VERSION).bin $(BIN)/tc4m-V$(VERSION).bin > \
	    $(BIN)/tc4m-V$(VERSION)_double.bin || true


# =========================
# 2m - TC9 variant
# =========================

TC2M: check-toolchain
	$(CC) -DTC2M $(CFLAGS) $(SRC) -o $(BUILD)/tc2m.ihx
	$(SREC) $(BUILD)/tc2m.ihx -intel -random-fill 0x0 0x4000 \
	    -exclude 0x26 0x28 -Checksum_Negative_Little_Endian 0x26 2 2 \
	    -o $(BIN)/tc2m-V$(VERSION).bin -binary
	cat $(BIN)/tc2m-V$(VERSION).bin $(BIN)/tc2m-V$(VERSION).bin > \
	    $(BIN)/tc2m-V$(VERSION)_double.bin || true


# =========================
# 70cm - TC9 variant
# =========================

TC70CM: check-toolchain
	$(CC) -DTC70CM $(CFLAGS) $(SRC) -o $(BUILD)/tc70cm.ihx
	$(SREC) $(BUILD)/tc70cm.ihx -intel -random-fill 0x0 0x4000 \
	    -exclude 0x26 0x28 -Checksum_Negative_Little_Endian 0x26 2 2 \
	    -o $(BIN)/tc70cm-V$(VERSION).bin -binary
	cat $(BIN)/tc70cm-V$(VERSION).bin $(BIN)/tc70cm-V$(VERSION).bin > \
	    $(BIN)/tc70cm-V$(VERSION)_double.bin || true


# =========================
# 2m BUFU - TC9 variant
# =========================

TC2MB: check-toolchain
	$(CC) -DTC2M -DBUFU $(CFLAGS) $(SRC) -o $(BUILD)/tc2mB.ihx
	$(SREC) $(BUILD)/tc2mB.ihx -intel -random-fill 0x0 0x4000 \
	    -exclude 0x26 0x28 -Checksum_Negative_Little_Endian 0x26 2 2 \
	    -o $(BIN)/tc2mB-V$(VERSION).bin -binary
	cat $(BIN)/tc2mB-V$(VERSION).bin $(BIN)/tc2mB-V$(VERSION).bin > \
	    $(BIN)/tc2mB-V$(VERSION)_double.bin || true


# =========================
# 70cm BUFU - TC9 variant
# =========================

TC70CMB: check-toolchain
	$(CC) -DTC70CM -DBUFU $(CFLAGS) $(SRC) -o $(BUILD)/tc70cmB.ihx
	$(SREC) $(BUILD)/tc70cmB.ihx -intel -random-fill 0x0 0x4000 \
	    -exclude 0x26 0x28 -Checksum_Negative_Little_Endian 0x26 2 2 \
	    -o $(BIN)/tc70cmB-V$(VERSION).bin -binary
	cat $(BIN)/tc70cmB-V$(VERSION).bin $(BIN)/tc70cmB-V$(VERSION).bin > \
	    $(BIN)/tc70cmB-V$(VERSION)_double.bin || true


# =========================
# CLEAN
# =========================

clean:
	rm -rf build/*
	rm -rf bin/*


.PHONY: all dirs clean check-toolchain TC4M TC2M TC70CM TC2MB TC70CMB
