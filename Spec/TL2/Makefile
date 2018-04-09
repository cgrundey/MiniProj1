include Makefile.defines.in

CCFLAGS += -std=c++11

OBJFILES = $(OBJ_DIR)/WriteSet.o $(OBJ_DIR)/tm_t.o $(OBJ_DIR)/test_t.o

.PHONY: clean

all:  $(OBJ_DIR)/test_threads

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/test_threads: $(OBJFILES)
	$(CPP) $(CCFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)
	cp $(OBJ_DIR)/test_threads .


$(OBJ_DIR)/test_t.o: $(OBJ_DIR) $(SRC_DIR)/test_threads.cpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test_threads.cpp -c -o $@


$(OBJ_DIR)/WriteSet.o: $(OBJ_DIR) $(SRC_DIR)/tm/WriteSet.c $(SRC_DIR)/tm/WriteSet.hpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/WriteSet.c -c -o $@

$(OBJ_DIR)/tm_t.o: $(OBJ_DIR) $(SRC_DIR)/tm/tm_thread.c $(SRC_DIR)/tm/tm_thread.hpp $(SRC_DIR)/tm/BitFilter.h $(SRC_DIR)/tm/ring.c
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/tm_thread.c -c -o $@


################
# common tasks #
################

clean:
	rm -rf $(TARGET_DIR)
	rm -f test_threads
