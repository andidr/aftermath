ACLOCAL_AMFLAGS=-I m4

template_verbose = $(template_verbose_@AM_V@)
template_verbose_ = $(template_verbose_@AM_DEFAULT_V@)
template_verbose_0 = @echo "  TEMPLATE" $@;

WHEEL_FILE_NAME=aftermath.core-$(PACKAGE_VERSION).whl

src/cffi_cdefs.h: \
	src/templates/cffi_cdefs.tpl.h \
	src/templates/array_element_types.py \
	src/templates/array_idents.py \
	src/tmp/base_types.purified.h \
	src/tmp/in_memory.purified.h

src/cffi_source.c: \
	src/templates/cffi_source.tpl.c \
	src/templates/array_element_types.py

package/aftermath/core/types.py: \
	src/templates/types.tpl.py \
	src/templates/array_element_types.py \
	$(srcdir)/src/templates/filters.py

package/aftermath/core/array.py: \
	src/templates/array.tpl.py \
	src/templates/array_element_types.py \
	$(srcdir)/src/templates/filters.py

package/aftermath/core/array.py: \
	src/templates/array.tpl.py \
	src/templates/array_element_types.py

src/cffi_source.c:
	$(MKDIR_P) src
	$(template_verbose)$(PYTHON) $(AFTERMATH_CORE_SOURCEDIR)/scripts/template.py \
		-m $(AFTERMATH_CORE_SOURCEDIR)/src/defs/aftermath/templates/toplevel/defs.py \
		-m src/templates/array_element_types.py \
		-m src/templates/array_idents.py \
		-t $< \
		-o $@

src/cffi_cdefs.h:
	$(MKDIR_P) src
	$(template_verbose)$(PYTHON) $(AFTERMATH_CORE_SOURCEDIR)/scripts/template.py \
		-m $(AFTERMATH_CORE_SOURCEDIR)/src/defs/aftermath/templates/toplevel/defs.py \
		-m src/templates/array_element_types.py \
		-m src/templates/array_idents.py \
		-t $< \
		-o $@.tmp
	cat src/tmp/base_types.purified.h \
		src/tmp/in_memory.purified.h \
		$@.tmp > $@.tmp.fused
	rm $@.tmp
	mv $@.tmp.fused $@

package/aftermath/core/array.py package/aftermath/core/types.py:
	$(MKDIR_P) package/aftermath/core/
	$(template_verbose)$(PYTHON) $(AFTERMATH_CORE_SOURCEDIR)/scripts/template.py \
		-m $(AFTERMATH_CORE_SOURCEDIR)/src/defs/aftermath/templates/toplevel/defs.py \
		-m $(srcdir)/src/templates/filters.py \
		-m src/templates/array_element_types.py \
		-m src/templates/array_idents.py \
		-t $< \
		-o $@

# Build a template definition file associating trace arrays with their
# element type
src/templates/array_element_types.py: src/tmp/array_typedefs.h
	$(MKDIR_P) src/templates
	echo "def definitions():" > $@.tmp
	echo "	return { \"array_element_types\": {" >> $@.tmp
	grep -o 'typedef\s\+[^;]*element_type;' $< | \
		grep -v '\(am_array_registry\|am_array_collection\|am_event_collection\)' | \
		sed 's/typedef\s\+\(.*\)\s\+\([^;]\+\)_element_type\s\?;/			"\2" : "\1",/g' >> $@.tmp
	echo "		}" >> $@.tmp
	echo "	}" >> $@.tmp
	mv $@.tmp $@

# Build a template definition file associating trace arrays with their
# array identifiers
src/templates/array_idents.py: $(AFTERMATH_CORE_SOURCEDIR)/src/default_trace_array_registry.c
	$(MKDIR_P) src/templates
	echo "def definitions():" > $@.tmp
	echo "	return { \"array_idents\": {" >> $@.tmp
	cat $< | \
		tr '\r\n' ' ' | \
		grep -o 'AM_DEFAULT_ARRAY_REGISTRY_REGISTER([a-zA-Z0-9_]\+\s*,\s*[a-zA-Z0-9_]\+\s*,\s*"[^"]\+"\s*)' | \
		sed 's/AM_DEFAULT_ARRAY_REGISTRY_REGISTER([a-zA-Z0-9_]\+\s*,\s*\([a-zA-Z0-9_]\+\)\s*,\s*"\([^"]\+\)"\s*)/			"\1": "\2",/g' \
		>> $@.tmp
	echo "		}" >> $@.tmp
	echo "	}" >> $@.tmp
	mv $@.tmp $@

# Build a file that includes all header files from
# $(AFTERMATH_CORE_SOURCEDIR)/default_trace_array_registry.c that contain the definition of an
# array type, preprocess it and extract all typedefs for array
# elements.
src/tmp/array_typedefs.h: $(AFTERMATH_CORE_SOURCEDIR)/src/default_trace_array_registry.c
	@$(MKDIR_P) src/tmp
	grep '^#include <aftermath/core/.*_array.h>$$' $< > $@.tmp.includes.c
	$(CPP) $(AM_CPPFLAGS) -I$(AFTERMATH_CORE_BUILDDIR)/headers -I$(AFTERMATH_CORE_SOURCEDIR)/headers -o $@.tmp.preprocessed.h $@.tmp.includes.c
	grep -o 'typedef\s\+[^;]*element_type;' $@.tmp.preprocessed.h > $@.tmp.typedefs.h
	mv $@.tmp.typedefs.h $@

# Extract all type definitions for base types
src/tmp/base_types.purified.h: $(AFTERMATH_CORE_BUILDDIR)/src/base_types.h
	@$(MKDIR_P) src/tmp
	grep '^typedef[^;]*;$$' $< > $@

# Produce purified version of in_memory.h that contains neither
# preprocessor directives nor forward declarations
src/tmp/in_memory.purified.h: $(AFTERMATH_CORE_BUILDDIR)/src/in_memory.h
	@$(MKDIR_P) src/tmp
	sed -e 's/\s*#.*$$//g' -e 's/^struct\s\+[a-zA-Z0-9_]\+;$$//g' $< > $@.tmp.c
	$(CPP) -o $@.tmp $@.tmp.c
	sed -e 's/\s*#.*$$//g' -i $@.tmp
	mv $@.tmp $@

BUILT_SOURCES=src/cffi_cdefs.h \
	src/cffi_source.c \
	package/aftermath/core/array.py \
	package/aftermath/core/types.py

clean-local:
	rm -f $(BUILT_SOURCES)
	rm -rf src/tmp \
		src/templates/array_element_types.py \
		src/templates/array_idents.py \
		src/templates/__pycache__ \
		package \
		aftermath.core.whl

COPY_PACKAGE_FILES= \
	package/setup.py \
	package/build.py \
	package/aftermath/__init__.py \
	package/aftermath/core/__init__.py \
	package/README.md

all: $(WHEEL_FILE_NAME)

$(WHEEL_FILE_NAME): $(COPY_PACKAGE_FILES) $(BUILT_SOURCES)
	cd package ; $(PYTHON) build.py
	cd package ; $(PYTHON) setup.py bdist_wheel
	@if test `ls -alh package/dist/aftermath.core*.whl 2>/dev/null | wc -l` -ne 1 ; \
	then \
		echo "More than one wheel generated." ; \
		exit 1 ; \
	fi
	ln -sfr package/dist/aftermath.core*.whl $@

# Contains a workaround for a bug systems that require --system to be
# set explicitly: try first with --system and if that fails without
# the option
install: $(WHEEL_FILE_NAME)
	$(PIP) install --system --prefix="$(prefix)" `readlink $<` || \
	   $(PIP) install --prefix="$(prefix)" `readlink $<`

.PHONY: install

package/setup.py: $(builddir)/src/package/setup.py
package/build.py: $(builddir)/src/package/build.py
package/aftermath/__init__.py: $(srcdir)/src/package/aftermath/__init__.py
package/aftermath/core/__init__.py: $(srcdir)/src/package/aftermath/core/__init__.py
package/README.md: $(srcdir)/src/package/README.md

$(COPY_PACKAGE_FILES):
	$(MKDIR_P) `dirname $@`
	cp $< $@
