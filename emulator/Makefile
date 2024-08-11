# compiler
CXX	:= clang++

LIBFLAGS := -static-libstdc++ -static-libgcc
LIBS := -lmingw32
LIBS += -lSDL2main -lSDL2
LIBS += -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive

CFLAGS := -Wall -Wshadow -Iinclude
CFLAGS += -O3
CFLAGS += -MD
#CFLAGS += -m32 # 32-bit program, for the 10% that aren't on x64
#CFLAGS += -MMD # オブジェクトファイルの依存関係を*.dに出力する
#CFLAGS += -MP # ヘッダファイルに依存関係がないとして依存関係を出力する

# output
OBJ_DIR := build
SRC_DIR := source
OUTPUT  := bin/fern.exe
SRCS_CPP	:= $(shell find $(SRC_DIR) -name *.cpp)

OBJS := $(subst $(SRC_DIR),$(OBJ_DIR),$(SRCS_CPP:.cpp=.o))
DEPS := $(OBJS:.o=.d)

-include $(DEPS)

all: $(OUTPUT)

# building
$(OUTPUT): $(OBJS)
	@echo -e "\tlinking..."
	$(CXX) $(LIBFLAGS) $^ $(LIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(DEPS) $(OUTPUT)

