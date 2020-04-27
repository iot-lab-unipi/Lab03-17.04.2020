CONTIKI_PROJECT = sensor-interactions
all: $(CONTIKI_PROJECT)
CONTIKI = ../..
MAKE_NET = MAKE_NET_NULLNET
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
include $(CONTIKI)/Makefile.include
