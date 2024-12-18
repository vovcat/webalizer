# This makefile is not distributed, only the maintainer (you) will use it.
# You will need GNU make for the following line to work correctly.
-include Makefile

ACLOCAL = aclocal -Wall -I tools/m4 --install
AUTOHEADER = autoheader -Wall
AUTOCONF = autoconf -Wall -I tools/autoconf
AUTOMAKE = automake --copy --add-missing

mkversion != mkversion=`git describe --dirty` && echo "$${mkversion\#*[_-]}$(EXTRA_VER)"
autoclean += autom4te.cache aclocal.m4 install-sh
autoclean += configure.ac~ configure configure~ configure.in configure.in~
autoclean += config.status configure.lineno configure.status.lineno
autoclean += config.h config.h.in~ config.log config.cache stamp-h1
autoclean += Makefile Makefile.in tools/autoconf

default-is-configure-and-make:
	[ -s configure ] || $(MAKE) regen
	$(MAKE) conf
	$(MAKE)

.PHONY: ver autoclean regen am reconf conf cleandeps

ver:
	@echo "=== Version is $(mkversion)"
autoclean:
	rm -rf $(autoclean)
	mkdir -p tools/autoconf
regen: ver autoclean
	$(ACLOCAL) --force
	$(AUTOHEADER) --force
	$(AUTOCONF) --force
	$(AUTOMAKE)
am:
	$(ACLOCAL)
	$(AUTOMAKE)
reconf:
	$(ACLOCAL)
	$(AUTOMAKE)
	$(AUTOCONF)
conf:
	sh config.status --recheck && sh config.status || sh configure
cleandeps:
	find . -xdev -type d -name .deps |xargs rm -rfv

.PHONY: subst_vars vars_val vars_exp vars
vars_o = $(subst file,file   ,$(origin $(1)))
# undefined, default, environment, environment override, command line, override, automatic, file
vars_f = $(foreach v, $(filter-out vars_% .VARIABLES, $(.VARIABLES)),				\
	$(if $(filter-out environment,$(origin $(v))),						\
	$(info [$(call vars_o,$(v))] $(v)=$(call $(1),$(v)))))
vars_p = $(foreach v, $(sort $(filter-out vars_% .VARIABLES, $(.VARIABLES))),			\
	$(if $(filter-out environment,$(origin $v)),						\
	$(info	[$(call vars_o,$v)]	$v = $(value $v)$(if	 				\
	$(filter simple,$(flavor $v))$(filter default automatic,$(origin $v)),,	[$($v)]))))
vars_s = $(foreach v, $(sort $(filter-out subst_vars, $(subst_vars))),				\
	$(info	[$(call vars_o,$v)]	$v = $(value $v)$(if	 				\
	$(filter simple,$(flavor $v))$(filter default automatic,$(origin $v)),,	[$($v)])))
subst_vars:; $(vars_s)
vars_val:; $(call vars_f, value)
vars_exp:; $(call vars_f, call)
vars:; $(vars_p)
