KDIR = /lib/modules/5.15.0-41-generic/build

kbuild:
	make -C $(KDIR) M=`pwd`

clean:
	make -C $(KDIR) M=`pwd` clean
