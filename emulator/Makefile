# compiler
CXX	:= clang++

LIBFLAGS := -static
LIBS := -lmingw32 -lSDL2
LIBS += -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid
# ^ should always be last

# static linking:  https://stackoverflow.com/questions/72163357/sdl-mingw-static-lib-linking-errors

CFLAGS := -Wall -Wshadow -Iinclude
CFLAGS += -O3
CFLAGS += -MMD -MP
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

.PHONY: clean
all: $(OUTPUT)

# building
$(OUTPUT): $(OBJS)
	@echo -e "\tlinking..."
	$(CXX) $(LIBFLAGS) $^ $(LIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(DEPS) $(OUTPUT)

