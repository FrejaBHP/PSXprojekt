TARGET = PSXtest
TYPE = ps-exe

SRCS = \
third_party/nugget/common/crt0/crt0.s \
src/main.c \
src/graphics.c \
textures/woodPanel.tim \
textures/woodDoor.tim \
textures/cobble.tim \

CPPFLAGS += -Ithird_party/psyq-iwyu/include
LDFLAGS += -Lthird_party/psyq/lib
LDFLAGS += -Wl,--start-group
LDFLAGS += -lapi
LDFLAGS += -lc
LDFLAGS += -lc2
LDFLAGS += -lcard
LDFLAGS += -lcd
LDFLAGS += -lcomb
LDFLAGS += -lds
LDFLAGS += -letc
LDFLAGS += -lgpu
LDFLAGS += -lgs
LDFLAGS += -lgte
LDFLAGS += -lgun
LDFLAGS += -lhmd
LDFLAGS += -lmath
LDFLAGS += -lmcrd
LDFLAGS += -lmcx
LDFLAGS += -lpad
LDFLAGS += -lpress
LDFLAGS += -lsio
LDFLAGS += -lsnd
LDFLAGS += -lspu
LDFLAGS += -ltap
LDFLAGS += -Wl,--end-group

include third_party/nugget/common.mk

space := $(subst ,, )

define OBJCOPYME
$(PREFIX)-objcopy -I binary --set-section-alignment .data=4 --rename-section .data=.rodata,alloc,load,readonly,data,contents -O $(FORMAT) -B mips --redefine-sym _binary_$(subst $(space),_,$(subst .,_,$(subst /,_,$<)))_start=$(subst .tim,,$(notdir $<))_start --redefine-sym _binary_$(subst $(space),_,$(subst .,_,$(subst /,_,$<)))_end=$(subst .tim,,$(notdir $<))_end $< $@
endef

# convert TIM file to bin
%.o: %.tim
	$(call OBJCOPYME)

# convert VAG files to bin
#%.o: %.vag
#	$(call OBJCOPYME)
	
# convert HIT to bin
#%.o: %.HIT
#	$(call OBJCOPYME)
