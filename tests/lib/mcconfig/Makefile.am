PACKAGE_STRING = "/lib/mcconfig"

AM_CPPFLAGS = \
	-DWORKDIR=\"$(abs_builddir)\" \
	$(GLIB_CFLAGS) \
	-I$(top_srcdir) \
	@CHECK_CFLAGS@

LIBS = @CHECK_LIBS@ \
	$(top_builddir)/lib/libmc.la

if ENABLE_MCLIB
LIBS += $(GLIB_LIBS) \
	@E2P_LIBS@
endif

TESTS = \
	config_string \
	user_configs_path

check_PROGRAMS = $(TESTS)

config_string_SOURCES = \
	config_string.c

user_configs_path_SOURCES = \
	user_configs_path.c
