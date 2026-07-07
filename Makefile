UNAME_S := $(shell uname -s)

# The Linux/KDE UI is only built on Linux, so only lint/format its sources there.
ifeq ($(UNAME_S),Linux)
    SOURCE_DIRS := core linux
    CMAKE_CONFIGURE_FLAGS := -DCMAKE_PREFIX_PATH=$(KDE_PREFIX)
else
    SOURCE_DIRS := core
    CMAKE_CONFIGURE_FLAGS :=
endif

SOURCES := $(shell find $(SOURCE_DIRS) -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \))
KDE_PREFIX ?= $(HOME)/kde/usr

.PHONY: format format-check lint

format:
	clang-format -i $(SOURCES)

format-check:
	clang-format --dry-run --Werror $(SOURCES)

lint:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $(CMAKE_CONFIGURE_FLAGS)
	clang-tidy -p build $(SOURCES)
