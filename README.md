# Programming Assignment 3
Programming assignment 3 for COP4600. (Steven, Ryan, Philip)

# Usage

Alright, so this should be pretty simple. There's a read_device.c and a write_device.c. Use the provided Makefile file to compile them both and get resulting files read_device.ko and write_device.ko. Then, install the write device first and following that the read device. The order here is important, since the write device initializes the variables shared to the read device. Similarly, you must remove the read device before removing the write device. Below is an example usage of the kernel modules.

```

cop4600@cop4600-VirtualBox:~/Documents/pa3/cop4600pa3$ make
make -C /lib/modules/4.4.0-116-generic/build M=/home/cop4600/Documents/pa3/cop4600pa3 modules
make[1]: Entering directory `/usr/src/linux-headers-4.4.0-116-generic'
  CC [M]  /home/cop4600/Documents/pa3/cop4600pa3/write_device.o
  CC [M]  /home/cop4600/Documents/pa3/cop4600pa3/read_device.o
  Building modules, stage 2.
  MODPOST 2 modules
  CC      /home/cop4600/Documents/pa3/cop4600pa3/read_device.mod.o
  LD [M]  /home/cop4600/Documents/pa3/cop4600pa3/read_device.ko
  CC      /home/cop4600/Documents/pa3/cop4600pa3/write_device.mod.o
  LD [M]  /home/cop4600/Documents/pa3/cop4600pa3/write_device.ko
make[1]: Leaving directory `/usr/src/linux-headers-4.4.0-116-generic'
cop4600@cop4600-VirtualBox:~/Documents/pa3/cop4600pa3$ sudo insmod write_device.ko
[sudo] password for cop4600: 
cop4600@cop4600-VirtualBox:~/Documents/pa3/cop4600pa3$ sudo insmod read_device.ko
cop4600@cop4600-VirtualBox:~/Documents/pa3/cop4600pa3$ dmesg | tail -10
[ 8961.480884] audit: type=1400 audit(1522899331.461:28): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="/usr/lib/cups/backend/cups-pdf" pid=14653 comm="apparmor_parser"
[ 8961.480890] audit: type=1400 audit(1522899331.461:29): apparmor="STATUS" operation="profile_replace" profile="unconfined" name="/usr/sbin/cupsd" pid=14653 comm="apparmor_parser"
[10107.282291] Initializing the FIFO write device...
[10107.282295] Registered FIFO write device with major number 246.
[10107.282308] Created FIFO write device class.
[10107.286294] Successfully created FIFO write device!
[10112.446614] Initializing the FIFO read device...
[10112.446618] Registered FIFO read device with major number 245.
[10112.446630] Created FIFO read device class.
[10112.448963] Successfully created FIFO read device!
cop4600@cop4600-VirtualBox:~/Documents/pa3/cop4600pa3$ sudo -i
root@cop4600-VirtualBox:~# echo "Hello there!" > /dev/fifowritedev
root@cop4600-VirtualBox:~# echo "How are you?" > /dev/fifowritedev
root@cop4600-VirtualBox:~# cat /dev/fiforeaddev
Hello there!
How are you?
root@cop4600-VirtualBox:~# exit
logout


```

# Authors

This module was written by Philip Rodriguez, Steven Chen, and Ryan Beck.
