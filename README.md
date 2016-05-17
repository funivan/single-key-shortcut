# One push - one command

Programs for second keyboard. 

# How to build?

- Install dependencies:
```sh
sudo apt-get install libx11-dev libxi-dev
```
- run make
```sh
make
```
 
 
# How to install?
- build package (See how to build) 
- run `sudo make install`

# How to find keyboard id ?
Run 
```sh
xinput
```

You will see following output:
```
⎡ Virtual core pointer                        	id=2	[master pointer  (3)]
⎜   ↳ Virtual core XTEST pointer              	id=4	[slave  pointer  (2)]
⎜   ↳ Logitech USB Optical Mouse              	id=8	[slave  pointer  (2)]
⎜   ↳ SIGMACHIP USB Keyboard                  	id=12	[slave  pointer  (2)]
⎣ Virtual core keyboard                       	id=3	[master keyboard (2)]
    ↳ Virtual core XTEST keyboard             	id=5	[slave  keyboard (3)]
    ↳ Power Button                            	id=6	[slave  keyboard (3)]
    ↳ Power Button                            	id=7	[slave  keyboard (3)]
    ↳ Sennheiser USB headset	                  id=9	[slave  keyboard (3)]
    ↳ SIGMACHIP USB Keyboard                  	id=11	[slave  keyboard (3)]
    ↳ Dell Dell USB Entry Keyboard            	id=10	[slave  keyboard (3)]

```
Find you keyboard. I use `Dell Dell USB Entry Keyboard` as second keyboard.
`↳ Dell Dell USB Entry Keyboard            	id=10	[slave  keyboard (3)]`
So id is `10`


# How to find keyboard key id?
First of all you need to build this program. Then find keyboard id.
Open default config file `./out/democonfig.conf` and replace default keyboard id `10` with your id.
Run program
```sh
 ./out/singlekey -c out/democonfig.conf

```

You should see something like this:
```
> ./out/singlekey -c out/democonfig.conf

[display_name] :0.0 
Loaded device ID: 10
Loaded commands:
Detect device input file path: /dev/input/event4
DEVICE_PATH: /dev/input/event4
start open file
Disable device with command: xinput disable 10
Executing: xinput disable 10

```

Hit some keys and you will find ids.
I hit right arrow and here is the code of this key: `106`
```
KEY fire 106
Empty Command (null)

```

# How to add command to config?
Configuration is very simple. Format is following: 
`<key id> : <command>`
So open demo config `out/democonfig.conf` and add for example
`106 : zenity --info --text hi`


`106` - id of key (right arrow)

`zenity --info --text Hello` - command

Run program `./out/singlekey -c out/democonfig.conf` and hit right arrow. 

# How to disable/enable keyboard?

```sh
 id=12
 xinput set-int-prop $id "Device Enabled" 8 0 // disable
 xinput set-int-prop $id "Device Enabled" 8 1 // enable
```
