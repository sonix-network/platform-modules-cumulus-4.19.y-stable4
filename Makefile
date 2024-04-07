DEBARCH ?= $(shell dpkg --print-architecture)
INC = $(src)/drivers/misc/cumulus

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

ifeq ($(DEBARCH),amd64)

# Common Cumulus platform library
obj-m += \
	drivers/misc/cumulus/cumulus-platform.o \
	drivers/misc/vhwmon/vhwmon.o

# CY8
obj-m += \
	drivers/misc/cy8c3245r1/cy8c3245r1.o

# Accton AS5712-54X
obj-m += \
	drivers/misc/accton/accton-as5712-54x-platform.o

# Accton AS5812-54T
obj-m += \
	drivers/misc/accton/accton-as5812-54t-platform.o

# Accton AS5835-54X
# Accton AS5835-54T
obj-m += \
	drivers/misc/accton/accton-as5835-54x-platform.o \
	drivers/misc/accton/accton-as5835-54t-platform.o \
	drivers/misc/accton/accton-as5835-cpucpld.o \
	drivers/misc/accton/accton-as5835-cpld1.o \
	drivers/misc/accton/accton-as5835-cpld2.o \
	drivers/misc/accton/accton-as5835-cpld3.o \
	drivers/misc/accton/accton-as5835-cpld4.o

# Accton AS5912-54X
obj-m += \
	drivers/misc/accton/accton-as5912-54x-platform.o

# Accton AS5916-54XL
obj-m += \
    drivers/misc/accton/accton-as5916-54xl-platform.o

# Accton AS6712-32X
obj-m += \
	drivers/misc/accton/accton-as6712-32x-platform.o

# Accton AS7312
obj-m += \
	drivers/misc/accton/accton-as7312-54x-platform.o

# Accton AS7326
obj-m += \
	drivers/misc/accton/accton-as7326-56x-platform.o

# Accton AS7712
obj-m += \
	drivers/misc/accton/accton-as7712-32x-platform.o

# Accton AS7726
obj-m += \
	drivers/misc/accton/accton-as7726-32x-platform.o

# Accton AS7816
obj-m += \
	drivers/misc/accton/accton-as7816-64x-platform.o

# Accton Minipack
obj-m += \
       drivers/misc/accton/accton-minipack-platform.o

# Accton Wedge100
obj-m += \
	drivers/misc/accton/accton-wedge100-32x-platform.o

# Celestica Haliburton (E1031)
obj-m += \
	drivers/misc/celestica/cel-e1031-platform.o \
	drivers/misc/celestica/cel-e1031-mmc.o \
	drivers/misc/celestica/cel-e1031-smc.o

# Celestica Pebble
obj-m += \
	drivers/misc/celestica/cel-pebble-platform.o \
	drivers/misc/celestica/cel-pebble-cpld.o

# Celestica Pebble-B
obj-m += \
	drivers/misc/celestica/cel-pebble-b-platform.o \
	drivers/misc/celestica/cel-pebble-b-cpld.o

# Celestica Questone
obj-m += \
	drivers/misc/celestica/cel-questone-muxpld.o \
	drivers/misc/celestica/cel-questone-platform.o

# Celestica Redstone-XP and Smallstone-XP
obj-m += \
	drivers/misc/celestica/cel-redstone-xp-cpld.o \
	drivers/misc/celestica/cel-redstone-xp-muxpld.o \
	drivers/misc/celestica/cel-xp-i2c-bus.o \
	drivers/misc/celestica/cel-xp-platform.o \
	drivers/misc/celestica/cel-redstone-v-cpld.o \
	drivers/misc/celestica/cel-redstone-v-muxpld.o \
	drivers/misc/celestica/cel-redstone-v-platform.o \
	drivers/misc/celestica/cel-xp-b-i2c-bus.o \
	drivers/misc/celestica/cel-redstone-xp-b-muxpld.o \
	drivers/misc/celestica/cel-redstone-xp-b-platform.o \
	drivers/misc/celestica/cel-smallstone-xp-cpld.o \
	drivers/misc/celestica/cel-smallstone-xp-muxpld.o \
	drivers/misc/celestica/cel-smallstone-xp-b-muxpld.o \
	drivers/misc/celestica/cel-smallstone-xp-b-platform.o


# Celestica Seastone
obj-m += \
	drivers/misc/celestica/cel-seastone-cpld.o \
	drivers/misc/celestica/cel-seastone-muxpld.o

# Celestica Seastone2 and Questone2
obj-m += \
	drivers/misc/celestica/cel-sea2que2-platform.o \
	drivers/misc/celestica/cel-sea2que2-base-board-cpld.o \
	drivers/misc/celestica/cel-fpga-i2c.o \
	drivers/misc/celestica/cel-seastone2-fpga.o \
	drivers/misc/celestica/cel-questone2-fpga.o

# Cumulus VX
obj-m += \
	drivers/misc/cumulus-vx/cumulus-vx-platform.o

# Dell S3000
obj-m += \
	drivers/misc/dell/dell-s3000-platform.o \
	drivers/misc/dell/dell-s3000-cpld.o

# Dell S4000
obj-m += \
	drivers/misc/dell/dell-s4000-platform.o

# Dell S4048T
obj-m += \
	drivers/misc/dell/dell-s4048t-platform.o

# Dell S6000
obj-m += \
	drivers/misc/dell/dell-s6000-platform.o

# Dell S6010
obj-m += \
	drivers/misc/dell/dell-s6010-platform.o

# Dell Z9100
obj-m += \
	drivers/misc/dell/dell-z9100-platform.o \
	drivers/misc/dell/dell-z9100-smf.o

# Dell EMC N32xx
obj-m += \
	drivers/misc/dellemc/dellemc-n32xx-platform.o \
	drivers/misc/dellemc/dellemc-n32xx-system-cpld.o \
	drivers/misc/dellemc/dellemc-n32xx-cpu-cpld.o \
	drivers/misc/dellemc/dellemc-n32xx-port-cpld.o \
	drivers/misc/dellemc/dellemc-n32xx-sys-cpld-mux.o 

# Dell EMC S41xxx
obj-m += \
        drivers/misc/dellemc/dellemc-s41xx-platform.o \
        drivers/misc/dellemc/dellemc-s41xx-system-cpld.o \
        drivers/misc/dellemc/dellemc-s41xx-master-cpld.o \
	drivers/misc/dellemc/dellemc-s41xx-slave-cpld.o \
        drivers/misc/dellemc/dellemc-s41xx-sff-mux.o

# Dell EMC S4248FBL
obj-m += \
	drivers/misc/dellemc/dellemc-s4248fbl-platform.o \
	drivers/misc/dellemc/dellemc-s4248fbl-smf.o

# Dell EMC S5048F
obj-m += \
        drivers/misc/dellemc/dellemc-s5048f-smf.o \
        drivers/misc/dellemc/dellemc-s5048f-platform.o \
        drivers/misc/dellemc/dellemc-s5048f-cpld1.o \
        drivers/misc/dellemc/dellemc-s5048f-cpld2.o \
        drivers/misc/dellemc/dellemc-s5048f-cpld3.o \
        drivers/misc/dellemc/dellemc-s5048f-cpld4.o

# Dell EMC S5212F
obj-m += \
	drivers/misc/dellemc/dellemc-s5212f-platform.o \
	drivers/misc/dellemc/dellemc-s5212f-fpga.o

# Dell EMC S5224F
obj-m += \
        drivers/misc/dellemc/dellemc-s5224f-platform.o \
        drivers/misc/dellemc/dellemc-s5224f-fpga.o

# Dell EMC S5232F
obj-m += \
	drivers/misc/dellemc/dellemc-s5232f-platform.o

# Dell EMC S5248F
obj-m += \
	drivers/misc/dellemc/dellemc-s5248f-platform.o

# Dell EMC S5296F
obj-m += \
	drivers/misc/dellemc/dellemc-s5296f-platform.o \
	drivers/misc/dellemc/dellemc-s5296f-fpga.o

# Dell EMC Z9264F
obj-m += \
	drivers/misc/dellemc/dellemc-z9264f-platform.o

# Delta AG5648v1
obj-m += \
	drivers/misc/delta/delta-ag5648v1-platform.o

# Delta AG9032v1
obj-m += \
	drivers/misc/delta/delta-ag9032v1-platform.o \
	drivers/misc/delta/delta-ag9032v1-swcpld.o \
	drivers/misc/delta/delta-ag9032v1-cpupld.o \
	drivers/misc/delta/delta-ag9032v1-sffmux.o \
	drivers/misc/delta/delta-ag9032v1-psumux.o \
	drivers/misc/delta/delta-ag9032v1-fanmux.o

# Delta AG9032v2
obj-m += \
	drivers/misc/delta/delta-ag9032v2-platform.o \
	drivers/misc/delta/delta-ag9032v2-systemcpld.o \
	drivers/misc/delta/delta-ag9032v2-mastercpld.o \
	drivers/misc/delta/delta-ag9032v2-slave1cpld.o \
	drivers/misc/delta/delta-ag9032v2-slave2cpld.o \
	drivers/misc/delta/delta-ag9032v2-sffmux.o

# Delta AG9032v2
obj-m += \
	drivers/misc/delta/delta-agv848v1-platform.o \
	drivers/misc/delta/delta-agv848v1-cpupld.o \
	drivers/misc/delta/delta-agv848v1-swpld1.o \
	drivers/misc/delta/delta-agv848v1-swpld2.o \
	drivers/misc/delta/delta-agv848v1-swpld3.o \
	drivers/misc/delta/delta-agv848v1-swpld4.o

# GMS S422-SW
obj-m += \
	drivers/misc/gms/gms-s422-platform.o

# Lenovo NE0152T
obj-m += \
	drivers/misc/lenovo/lenovo-ne0152t-platform.o

# Lenovo NE2572
obj-m += \
	drivers/misc/lenovo/lenovo-ne2572-platform.o

# Lenovo NE2580
obj-m += \
	drivers/misc/lenovo/lenovo-ne2580-platform.o \
	drivers/misc/lenovo/lenovo-ne2580-cpld1.o \
	drivers/misc/lenovo/lenovo-ne2580-cpld2.o

# Lenovo NE10032
obj-m += \
	drivers/misc/lenovo/lenovo-ne10032-platform.o

# Quanta IX1
obj-m += \
	drivers/misc/quanta/quanta-ix1-rangeley-platform.o \
	drivers/misc/quanta/quanta-ix-rangeley-cpld.o

# Quanta IX2
obj-m += \
	drivers/misc/quanta/quanta-ix2-rangeley-platform.o

# Quanta IX7
obj-m += \
	drivers/misc/quanta/quanta-ix7-cpld.o \
	drivers/misc/quanta/quanta-ix7-platform.o

# Quanta IX8
obj-m += \
	drivers/misc/quanta/quanta-ix8-cpld.o \
	drivers/misc/quanta/quanta-ix8-platform.o

# Quanta LY4R
obj-m += \
	drivers/misc/quanta/quanta-ly4r-platform.o

# Quanta LY6
obj-m += \
	drivers/misc/quanta/quanta-ly6-rangeley-cpld.o \
	drivers/misc/quanta/quanta-ly6-rangeley-platform.o

# Quanta LY7
obj-m += \
	drivers/misc/quanta/quanta-ly7-cpld.o \
	drivers/misc/quanta/quanta-ly7-platform.o

# Quanta LY8
obj-m += \
	drivers/misc/quanta/quanta-ly8-rangeley-platform.o

# Quanta LY9
obj-m += \
	drivers/misc/quanta/quanta-ly9-rangeley-cpld.o \
	drivers/misc/quanta/quanta-ly9-rangeley-platform.o

# Quanta Utils
obj-m += \
	drivers/misc/quanta/quanta-utils.o

else ifeq ($(DEBARCH),armel)
obj-m += \
	drivers/misc/accton/accton-as4610-54-cpld.o \
	drivers/misc/dni/dni-3048up-cpld.o
#	drivers/misc/accton/dni-3448p-cpld.o \

else
  $(error unsupported Debian architecture)
endif

ccflags-y  += -I$(src) -I$(src)/include -I$(INC)

else
# regular makefile

MAKEFLAGS += --no-print-directory

KSRC ?= /lib/modules/$(shell uname -r)/build
PWD := $(CURDIR)

modules:
	$(MAKE) -C "$(KSRC)" M=$(PWD) V=1

modules_print:
	@echo $(patsubst %.o,%,$(obj-m))

modules_install: modules
	$(MAKE) -C "$(KSRC)" M=$(PWD) $@

clean:
	$(MAKE) -C "$(KSRC)" M=$(PWD) $@

help:
	@echo 'We support a common environment variable that allows a developer to'
	@echo 'build devel type deb packages by pointing to a kernel source tree.'
	@echo 'KSRC=<kernel-tree>  - if this variable is set the build system'
	@echo '                      will use this kernel tree to build targets.'
	@echo '                      This has a few requirements.'
	@echo 'The usage progression is:'
	@echo '  If in kernel tree top level Makefile was used then do:'
	@echo '    make KSRC=<kernel-tree>'
	@echo ''
	@echo '  If in kernel tree cl-easy-button was used, then do:'
	@echo '    make -f debian/rules KSRC=<kernel-tree> cl-easy-button'
	@echo ''
	@$(MAKE) -C "$(KSRC)" M=$(PWD) $@
	@$(MAKE) -f debian/rules help

.PHONY: modules modules_install clean help modules_print
endif
