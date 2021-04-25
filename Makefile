# tool macros
CC := cc
CCFLAGS := -g -Wall
LIBS := -lbfd
CCOBJFLAGS := $(CCFLAGS) -c


# compile macros
TARGET := example
SRC_PATH := .
OBJ_PATH := .


# src files & obj files
SRC := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.c*)))
OBJ := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(SRC)))))


# clean files list
CLEAN_LIST := $(TARGET) $(OBJ)


# default rule
default: all


# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAGS) -o $@ $<


# phony rules
.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

