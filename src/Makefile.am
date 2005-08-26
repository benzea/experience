## arch-tag: 2f85a6f2-8413-4f20-955d-8dfdfb9969aa

INCLUDES = $(EXPERIENCE_CFLAGS)

enginedir = $(libdir)/gtk-2.0/$(GTK_VERSION)/engines

engine_LTLIBRARIES = libexperience.la

libexperience_la_SOURCES = 	\
		drawing_functions.c \
		drawing_functions.h \
		filter.c \
		filter.h \
		group.c \
		group.h \
		image.c \
		image.h \
		fill.c \
		fill.h \
		group_drawable.c \
		group_drawable.h \
		image_loading.c \
		image_loading.h \
		main.c \
		match.c \
		match.h \
		drawable.c \
		drawable.h \
		parser.h \
		parser.y \
		parsetree.c \
		parsetree.h \
		experience.h \
		rcstyle.c \
		rcstyle.h \
		render.c \
		render.h \
		style.c \
		style.h \
		symbols.h \
		utils.c \
		utils.h \
		dynamic_color.c \
		dynamic_color.h \
		raw_image.c \
		raw_image.h \
		data.h

libexperience_la_LDFLAGS = -avoid-version -module -no-undefined
libexperience_la_LIBADD = $(EXPERIENCE_LIBS)
AM_YFLAGS = -p experience_yy
