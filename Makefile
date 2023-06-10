
APP=dfxp_shm_client
SRCS-y := dfxp_shm_client_main.c dfxp_shm_config.c 

CFLAGS += g -I./
CFLAGS += -DHTTP_PARSE
CFLAGS += $(WERROR_FLAGS) -Wno-address-of-packed-member -Wimplicit-function-declaration


CFLAGS += -DHTTP_PARSE
CFLAGS += -Wno-address-of-packed-member
#LDFLAGS += -lpthread -lrt -Llib -lshmclient
LDFLAGS += -Llib -lshmclient -lpthread -lrt -lm

build/$(APP): $(SRCS-y)
	mkdir -p build
	gcc -$(CFLAGS) $(SRCS-y) -o $@ $(LDFLAGS) 

clean:
	rm -rf build/
	

