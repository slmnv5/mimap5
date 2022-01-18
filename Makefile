.PHONY:   info clean  build_debug build_run build_test

PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

TMP1 = $(shell find . -name "*.cpp" ! -name "test_*cpp")
OBJ_APP = $(TMP1:.cpp=.o)

TMP2 = $(shell find . -name "*.cpp" ! -name "mimap*cpp")
OBJ_TST =  $(TMP2:.cpp=.o)

TARGET = mimap
BUILD_MODE ?= debug


LDFLAGS += -pthread -lasound
CPPFLAGS += -std=c++11 -g
 

build_test: info $(OBJ_TST)
	@echo "build test and run all tests"
	$(CXX)  -o $@ $^  $(LDFLAGS)
	$(EXTRA_CMDS)
	./test_0

build_debug: info mimap
	@echo "build debug and run integration test"
	./start.sh

build_run: CPPFLAGS = -std=c++11 -O2
build_run: info mimap
	@echo "build release, do i need to clean first?"

mimap: $(OBJ_APP)
	@echo "build app"
	$(CXX)  -o $@ $^  $(LDFLAGS)
	$(EXTRA_CMDS)


DEPENDS = $(shell find . -name "*.d")

%.o: %.cpp 
	$(CXX) $(WARNING) $(CXXFLAGS) -MMD -MP -c $< -o $@


-include $(DEPENDS)

clean:
	rm -frv $(TARGET) $(OBJ_APP) $(OBJ_TST) $(EXTRA_CLEAN)

	
	
info:
	cd $(PROJECT_ROOT)
	@echo LDFLAGS  -- $(LDFLAGS)
	@echo CPPFLAGS -- $(CPPFLAGS)
	@echo CXXFLAGS -- $(CXXFLAGS)
	@echo LDFLAGS -- $(LDFLAGS)
	@echo LDLIBS -- $(LDLIBS)
	@echo PROJECT_ROOT -- $(PROJECT_ROOT)
	@echo BUILD_MODE -- $(BUILD_MODE)
	@echo EXTRA_CLEAN -- $(EXTRA_CLEAN)
	@echo MAKEFILE_LIST -- $(MAKEFILE_LIST)
	@echo OBJ_APP -- $(OBJ_APP)
	@echo OBJ_TST -- $(OBJ_TST)
	@echo DEPENDS -- ${DEPENDS}


 
