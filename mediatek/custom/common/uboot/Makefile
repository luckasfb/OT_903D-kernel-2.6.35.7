include $(TOPDIR)/config.mk


LIB	= $(obj)lib$(BOARD).a
COBJS  := $(patsubst %.c,%.o,$(wildcard *.c))
COBJS  += $(patsubst %.c,%.o,$(wildcard lcm/*.c))
SOBJS  := $(patsubst %.S,%.o,$(wildcard *.S))

MTK_INC      := -I$(MTK_ROOT_CUSTOM)/common/kernel/lcm/inc
CPPFLAGS     += $(MTK_INC)
CFLAGS       += $(MTK_INC)
AFLAGS       += $(MTK_INC)
HOSTCPPFLAGS += $(MTK_INC)

SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS    := $(addprefix $(obj),$(COBJS))
SOBJS   := $(addprefix $(obj),$(SOBJS))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:      clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
