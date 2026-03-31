CXXFLAGS = -std=c++17 -Wall -O2
INCLUDES = -I. -I..

SRC_DIR = ..
SRCS := *.cpp $(SRC_DIR)/*.cpp

TEST_DIR = .
TEST_NAMES := $(notdir $(wildcard $(TEST_DIR)/test*))

BIN_DIR = $(TEST_DIR)/bin
OUT_DIR = $(TEST_DIR)/outputs

BINARIES := $(addprefix $(BIN_DIR)/, $(TEST_NAMES))
OUTPUTS := $(addprefix $(OUT_DIR)/, $(addsuffix .txt, $(TEST_NAMES)))


all: $(BINARIES)

$(BIN_DIR) $(OUT_DIR):
	mkdir -p $@

$(BIN_DIR)/%: $(SRCS) $(TEST_DIR)/%/* | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(TEST_DIR)/$* $(SRCS) -o $@

$(OUT_DIR)/%.txt: $(BIN_DIR)/% | $(OUT_DIR)
	./$< > $@

define COMPARE_RULE
compare-$(1): $(OUT_DIR)/$(1).txt $(TEST_DIR)/$(1)/expected.txt
	@diff -u --color=auto $(TEST_DIR)/$(1)/expected.txt $(OUT_DIR)/$(1).txt && echo "✅ $(1) passed" || echo "❌ $(1) failed"
endef

$(foreach test,$(TEST_NAMES),$(eval $(call COMPARE_RULE,$(test))))

run-all: $(addprefix compare-, $(TEST_NAMES))

run-%: compare-%
	@

clean:
	rm -rf $(BIN_DIR) $(OUT_DIR)

.PHONY: all clean run-all $(addprefix compare-, $(TEST_NAMES))
