CFLAGS += -I$(INCLUDE_PATH)
CFLAGS += -I$(PLAYER2_PATH)/linux/include/linux/stm/ 
CFLAGS += -I$(PLAYER2_PATH)/linux/drivers/media/video/stm/ 
CFLAGS += -Idrivers/media/dvb/dvb-core 
CFLAGS += -I$(STMFB_PATH)/include 
CFLAGS += -DDEBUG_DVB
CFLAGS += -I$(PLAYER2_PATH)/include 

ifeq ($(BOARD_TYPE),qboxhd)
cx24116-objs +=		cx24116/cx24116.o
avl2108-objs +=		avl2108/avl2108.o
zl10353-objs +=		zl10353/zl10353.o
tda18218-objs +=	tda18218/tda18218.o
tda10048-objs +=	tda10048/tda10048.o
tda10023-objs +=	tda10023/tda10023.o
obj-m += cx24116.o avl2108.o zl10353.o tda10048.o tda10023.o tda18218.o
else 
ifeq ($(BOARD_TYPE),qboxhd_mini)
avl2108-objs +=		avl2108/avl2108.o
zl10353-objs +=		zl10353/zl10353.o
tda10048-objs +=	tda10048/tda10048.o
tda18218-objs +=	tda18218/tda18218.o
tda10023-objs +=	tda10023/tda10023.o
obj-m += avl2108.o zl10353.o tda10048.o tda10023.o tda18218.o
endif
endif

