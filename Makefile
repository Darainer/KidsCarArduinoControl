# ------------------------------------------------------------
# User-configurable variables (override on the CLI if desired)
# ------------------------------------------------------------
BOARD      ?= arduino:avr:nano:cpu=atmega328    # FQBN
PORT       ?= /dev/ttyUSB0                      # serial port
SKETCH     ?= examples/FourWD_Demo              # path to .ino folder
LIB_DIRS   ?= libraries                         # extra libraries folder
TARGET_DIR ?= build                             # where artefacts go
# ------------------------------------------------------------

SKETCH_NAME := $(notdir $(SKETCH))
HEX         := $(TARGET_DIR)/$(SKETCH_NAME).ino.hex

# ---------- default target ---------------------------------------------------
all: $(HEX)

# ---------- compile ----------------------------------------------------------
$(HEX):
	@mkdir -p $(TARGET_DIR)
	arduino-cli compile \
	  --fqbn $(BOARD) \
	  --output-dir $(TARGET_DIR) \
	  --libraries $(LIB_DIRS) \
	  $(SKETCH)

# ---------- flash ------------------------------------------------------------
upload: $(HEX)
	arduino-cli upload \
	  --fqbn $(BOARD) \
	  -p $(PORT) \
	  --input-file $(HEX) \
	  $(SKETCH)

# ---------- serial monitor ---------------------------------------------------
monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=115200

# ---------- cleanup ----------------------------------------------------------
clean:
	rm -rf $(TARGET_DIR)

.PHONY: all upload monitor clean