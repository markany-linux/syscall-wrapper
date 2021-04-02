obj-m :=		syscall-wrapper.o
syscall-wrapper-objs :=	module.o	\
			syscalls.o	\
			utils.o		\
			wrapper.o

KBUILD =		/lib/modules/$(shell uname -r)/build
PWD =			$(shell pwd)

all:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean
