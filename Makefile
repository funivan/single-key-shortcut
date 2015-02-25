DIR=./out
prefix=/usr/local
all:
	mkdir -p $(DIR)
	gcc src/singlekey.c -lXi -lX11  -o $(DIR)/singlekey
	
	chmod +x $(DIR)/singlekey
	cp src/singlekey_autostart $(DIR)/singlekey_autostart 
	
	chmod +x $(DIR)/singlekey_autostart
	cp src/singlekey.desktop $(DIR)/singlekey.desktop 
	
install:
	install -m 0755 $(DIR)/singlekey $(prefix)/bin
	install -m 0755 $(DIR)/singlekey_autostart $(prefix)/bin
	install -m 0755 $(DIR)/singlekey.desktop /etc/xdg/autostart/