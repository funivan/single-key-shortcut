DIR=./out
prefix=/usr/local
all: 
	@echo "Select target"
	@echo "- build"
	@echo "- install"
	@echo "- autostart"
	@echo "- uninstall"
	@echo "- clean"

build: clean
	@echo "\n---- Build project"

	mkdir -p $(DIR)
	gcc src/singlekey.c -lXi -lX11  -o $(DIR)/singlekey
	
	cp src/singlekey_autostart $(DIR)/singlekey_autostart 
	chmod +x $(DIR)/singlekey
	
	cp src/singlekey.desktop $(DIR)/singlekey.desktop 
	chmod +x $(DIR)/singlekey_autostart
	
	echo "# Default id\n10" >  $(DIR)/democonfig.conf

clean:
	@echo "\n---- Remove all compiled files"
	rm -rf $(DIR)/*

install: build
	@echo "\n---- Install keyboard programm"
	install -m 0755 $(DIR)/singlekey $(prefix)/bin

autostart: install
	@echo "\n---- Install keyboard autostart"
	install -m 0755 $(DIR)/singlekey_autostart $(prefix)/bin
	install -m 0755 $(DIR)/singlekey.desktop /etc/xdg/autostart/

uninstall: clean
	@echo "\n---- Remove all installed files"
	rm -rf $(prefix)/bin/singlekey
	rm -rf $(prefix)/bin/singlekey_autostart
	rm -rf /etc/xdg/autostart/singlekey.desktop

