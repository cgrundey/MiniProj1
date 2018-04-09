include Makefile.defines.in

CCFLAGS += -std=c++11

OBJFILES = $(OBJ_DIR)/WriteSet.o $(OBJ_DIR)/tm_t.o $(OBJ_DIR)/test_t.o $(OBJ_DIR)/ring.o

MPOBJFILES = $(OBJ_DIR)/WriteSet.o $(OBJ_DIR)/tm_t.o $(OBJ_DIR)/ring.o $(OBJ_DIR)/mp_test.o

.PHONY: clean

all:  $(OBJ_DIR)/test_threads $(OBJ_DIR)/mp_test
test: $(OBJ_DIR)/test_threads
custom_test: $(OBJ_DIR)/mp_test

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/test_threads: $(OBJFILES)
	$(CPP) $(CCFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)
	cp $(OBJ_DIR)/test_threads .

$(OBJ_DIR)/mp_test: $(MPOBJFILES)
	$(CPP) $(CCFLAGS) -o $@ $(OBJFILES) $(LDFLAGS)
	cp $(OBJ_DIR)/mp_test .


$(OBJ_DIR)/test_t.o: $(OBJ_DIR) $(SRC_DIR)/test_threads.cpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/test_threads.cpp -c -o $@


$(OBJ_DIR)/mp_test.o: $(OBJ_DIR) $(SRC_DIR)/custom_test_RingSTM.cpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/custom_test_RingSTM.cpp -c -o $@


$(OBJ_DIR)/WriteSet.o: $(OBJ_DIR) $(SRC_DIR)/tm/WriteSet.c $(SRC_DIR)/tm/WriteSet.hpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/WriteSet.c -c -o $@

$(OBJ_DIR)/ring.o: $(OBJ_DIR) $(SRC_DIR)/tm/ring.cpp $(SRC_DIR)/tm/ring.hpp
		$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/ring.cpp -c -o $@

$(OBJ_DIR)/tm_t.o: $(OBJ_DIR) $(SRC_DIR)/tm/tm_thread.c $(SRC_DIR)/tm/tm_thread.hpp
	$(CPP) $(CCFLAGS) $(CPPFLAGS) $(SRC_DIR)/tm/tm_thread.c -c -o $@


################
# common tasks #
################

clean:
	rm -rf $(TARGET_DIR)
	rm -f test_threads
	rm -f mp_test
