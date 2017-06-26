BIN=camera
SRC=$(wildcard src/*.cpp) .rpc/control.grpc.pb.cc .rpc/control.pb.cc
DEP=$(patsubst %.cpp,%.d,$(patsubst %.cc,%.d,$(SRC)))
OBJ=$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(SRC)))

CXX=g++
CFLAGS=-O2 -ffast-math -I. -I.rpc
LDFLAGS=-s
LIBS=-lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio \
     -lsvm -ldlib -lopenblas -lcpptorch -lTH -lTHNN \
     -lgrpc++ -lgrpc -lprotobuf

.PHONY: all clean

all: $(BIN)

clean:
	@rm -f $(BIN) $(DEP) $(OBJ)

$(BIN): $(OBJ)
	@echo [L] $@
	@$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.cpp Makefile
	@echo [C] $<
	@$(CXX) -c -MMD -MP $(CFLAGS) $< -o $@

%.o: %.cc Makefile
	@echo [C] $<
	@$(CXX) -c -MMD -MP $(CFLAGS) $< -o $@

src/http_utils.cpp: .rpc/control.grpc.pb.h .rpc/control.pb.h

.rpc/control.grpc.pb.cc .rpc/control.grpc.pb.h: .rpc/control.proto
	@protoc --grpc_out . --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` $<

.rpc/control.pb.cc .rpc/control.pb.h: .rpc/control.proto
	@protoc --cpp_out . $<

-include $(DEP)
