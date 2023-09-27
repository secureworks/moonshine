MAKEFILE_PATH	:= $(abspath $(lastword $(MAKEFILE_LIST)))
ZIP_PREFIX		:= $(notdir $(patsubst %/,%,$(dir $(MAKEFILE_PATH))))

ifeq ($(OS),Windows_NT)
  TARGET = mingw
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    TARGET = linux
  endif
  ifeq ($(UNAME_S),Darwin)
    TARGET = darwin
  endif
endif

ARCH = x86_64
DOCKER = docker run --rm -it -v $$PWD:/work/ --workdir /work -e CROSS_TRIPLE=$(ARCH)-$(TARGET) sidaf/crossbuild-llvm-extra:latest
#DOCKER = docker run --rm -it -v $$PWD:/work/ --workdir /work -e CROSS_TRIPLE=$(ARCH)-$(TARGET) sidaf/crossbuild:latest
CMAKE = $(DOCKER) cmake
CTEST = $(DOCKER) ctest
DOXYGEN = doxygen
DOXYBOOK2 = doxybook2

.PHONY: docs

build: build-shared build-static build-object

build-shared:
ifeq ($(wildcard _build/$(TARGET)-$(ARCH)-shared/.),)
	$(CMAKE) -G Ninja -S . -B _build/$(TARGET)-$(ARCH)-shared -DBUILD_SHARED_LIBS=YES -DCMAKE_BUILD_TYPE=Release
endif
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-shared

build-static:
ifeq ($(wildcard _build/$(TARGET)-$(ARCH)-static/.),)
ifeq ($(TARGET),msvc)
	$(CMAKE) -G Ninja -S . -B _build/$(TARGET)-$(ARCH)-static -DCMAKE_RELEASE_POSTFIX=_static -DBUILD_SHARED_LIBS=NO -DCMAKE_BUILD_TYPE=Release
else
	$(CMAKE) -G Ninja -S . -B _build/$(TARGET)-$(ARCH)-static -DBUILD_SHARED_LIBS=NO -DCMAKE_BUILD_TYPE=Release
endif
endif
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-static

build-object:
ifeq ($(wildcard _build/$(TARGET)-$(ARCH)-object/.),)
	$(CMAKE) -G Ninja -S . -B _build/$(TARGET)-$(ARCH)-object -DBUILD_OBJECT_LIBS=YES -DCMAKE_BUILD_TYPE=Release
endif
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-object

install: install-shared install-static install-object

install-shared: build-shared
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-shared --target install/strip

install-static: build-static
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-static --target install/strip

install-object: build-object
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-object --target install/strip

clean:
ifneq ($(wildcard _build/$(TARGET)-$(ARCH)-shared/.),)
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-shared --target clean
endif
ifneq ($(wildcard _build/$(TARGET)-$(ARCH)-static/.),)
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-static --target clean
endif
ifneq ($(wildcard _build/$(TARGET)-$(ARCH)-object/.),)
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-object --target clean
endif
ifneq ($(wildcard _build/$(TARGET)-$(ARCH)-static-test/.),)
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-static-test --target clean
endif

distclean:
ifeq ($(OS),Windows_NT)
	@powershell.exe -Command "Remove-Item -LiteralPath _build -Force -Recurs -ErrorAction Ignore" || exit 0;
	@powershell.exe -Command "Remove-Item -LiteralPath _install -Force -Recurs -ErrorAction Ignore" || exit 0;
else
	@rm -rf ./_build ./_install || true
endif

debug:
ifeq ($(wildcard _build/$(TARGET)-$(ARCH)-static-test/.),)
ifeq ($(TARGET),msvc)
	$(CMAKE) -G Ninja -S . -B _build/$(TARGET)-$(ARCH)-static-test -DCMAKE_RELEASE_POSTFIX=_static -DBUILD_SHARED_LIBS=NO -DCMAKE_BUILD_TYPE=Debug
else
	$(CMAKE) -G Ninja -S . -B _build/$(TARGET)-$(ARCH)-static-test -DBUILD_SHARED_LIBS=NO -DCMAKE_BUILD_TYPE=Debug
endif
endif
	$(CMAKE) --build _build/$(TARGET)-$(ARCH)-static-test --target tests

check: debug
#	$(DOCKER) _build/$(TARGET)-$(ARCH)-static-test/tests/tests
	$(CTEST) --output-on-failure --progress --test-dir _build/$(TARGET)-$(ARCH)-static-test

dist:
	$(MAKE) distclean
	$(RM) $(ZIP_PREFIX)_$(TARGET)-$(ARCH).zip
	$(MAKE) install
	( cd _install/*-$(ARCH)/ && zip -r ../../$(ZIP_PREFIX)_$(TARGET)-$(ARCH).zip * -x lib/cmake/\* )

docs:
	@$(DOXYGEN)
	@mkdir -p docs/md
	@$(DOXYBOOK2) -q -i docs/xml -o docs/md
	@cat docs/md/indexpage.md docs/md/Files/windows_8h.md docs/md/Files/darwin_8h.md docs/md/Files/linux_8h.md docs/md/Files/init_8h.md | grep -v "^\-\-\-$$" | grep -v "^title: " | grep -v "^Updated on " | sed '/^$$/N;/^\n$$/D' | sed 's/^### /\n\n### /g' | sed 's/^## /-------------------------------\n## /' | sed 's/^# include\/\(.*\)/# Library API\n```\n#include "\1"\n```/' > README.md
