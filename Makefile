ifneq ($(KERNELRELEASE),)
	obj-m := cubekernelmodule.o 
	cubekernelmodule-objs := cube_kmodule_test.o ./string/memfunc.o ./alloc/alloc_init.o ./alloc/buddy.o ./alloc/calloc.o ./alloc/galloc.o ./alloc/talloc.o ./json/json.o ./basefunc/basefunc.o ./struct_deal/struct_deal.o ./struct_deal/struct_ops.o ./struct_deal/struct_init.o ./struct_deal/enum_flag_ops.o ./crypto/crypto_func.o ./crypto/sha1.o ./crypto/sm3.o ./memdb/memdb_init.o ./memdb/memdb.o ./memdb/type_subtype_ops.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -rf	*.o *~ .*.cmd .temp_versions *.order *.symvers *.mod.c .depend 
cleanall:
	rm -rf ./string/*.o ./alloc/*.o ./json/*.o ./basefunc/*.o ./struct_deal/*.o ./crypto/*.o ./memdb/*.o
