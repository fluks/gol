SRC_DIR = src/

.PHONY: all

all:
	$(MAKE) -C $(SRC_DIR)

ncurses:
	$(MAKE) -C $(SRC_DIR) ncurses

clean:
	$(MAKE) -C $(SRC_DIR) clean
