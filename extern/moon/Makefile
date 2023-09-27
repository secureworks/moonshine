#
# Try and follow naming and behaviour as defined in
# https://www.gnu.org/prep/standards/html_node/Standard-Targets.html
#

ifeq ($(OS),Windows_NT)
	HOST = windows
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		HOST = linux
	endif
	ifeq ($(UNAME_S),Darwin)
		HOST = macos
	endif
endif

ifeq ($(TARGET),)
	TARGET = $(HOST)
	CROSS =
endif

ifneq ($(HOST),$(TARGET))
	CROSS = crosscompile-
endif

ifneq ($(CROSS),)
ifeq ($(TARGET),macos)
$(error macos target cannot be crosscompiled inside docker as mach-o binaries cannot be executed)
endif
endif

ARCH = x86_64

DOCKER = docker run --rm -it -v $$PWD:/work/ --workdir /work -e CROSS_TRIPLE=$(ARCH)-$(TARGET) sidaf/crossbuild-llvm-extra:latest

ifneq ($(CROSS),)
	CMAKE = $(DOCKER) cmake
	CTEST = $(DOCKER) ctest
else
	CMAKE = cmake
	CTEST = ctest
endif

all: debug

debug:
ifeq ($(wildcard _build/$(CROSS)$(TARGET)-$(ARCH)-debug/.),)
	$(CMAKE) -S . --preset=$(CROSS)$(TARGET)-$(ARCH)-debug
endif
	$(CMAKE) --build --preset=$(CROSS)$(TARGET)-$(ARCH)-debug-build

release: distclean
	$(CMAKE) -S . --preset=$(CROSS)$(TARGET)-$(ARCH)-release
	$(CMAKE) --build --preset=$(CROSS)$(TARGET)-$(ARCH)-release-build

install: release
	$(CMAKE) --build --target install/strip --preset=$(CROSS)$(TARGET)-$(ARCH)-release-build

dist: release
	$(CMAKE) --build --target package --preset=$(CROSS)$(TARGET)-$(ARCH)-release-build

check: debug
	$(CTEST) --preset=$(CROSS)$(TARGET)-$(ARCH)-debug-test

clean:
ifneq ($(wildcard _build/$(TARGET)-$(ARCH)-debug/.),)
	$(CMAKE) --build --target clean --preset=$(CROSS)$(TARGET)-$(ARCH)-debug-build
endif
ifneq ($(wildcard _build/$(TARGET)-$(ARCH)-release/.),)
	$(CMAKE) --build --target clean --preset=$(CROSS)$(TARGET)-$(ARCH)-release-build
endif

distclean:
ifeq ($(OS),Windows_NT)
	@powershell.exe -Command "Remove-Item -LiteralPath _build -Force -Recurs -ErrorAction Ignore" || exit 0;
	@powershell.exe -Command "Remove-Item -LiteralPath _install -Force -Recurs -ErrorAction Ignore" || exit 0;
else
	@rm -rf ./_build ./_install || true
endif
