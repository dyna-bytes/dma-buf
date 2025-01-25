
obj-m := exporter.o importer.o
all:
	make -C ../WSL2-Linux-Kernel M=`pwd` modules
clean:
	make -C ../WSL2-Linux-Kernel M=`pwd` clean
