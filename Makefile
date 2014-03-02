SRC_DIR = src/

.PHONY: all

all:
	$(MAKE) -C $(SRC_DIR)

clean:
	$(MAKE) -C $(SRC_DIR) clean
