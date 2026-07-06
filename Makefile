SOURCES := $(shell find core linux -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \))
KDE_PREFIX ?= $(HOME)/kde/usr

.PHONY: format format-check lint

format:
	clang-format -i $(SOURCES)

format-check:
	clang-format --dry-run --Werror $(SOURCES)

lint:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH=$(KDE_PREFIX)
	clang-tidy -p build $(SOURCES)
