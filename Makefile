# contrib/pg_sulog/Makefile

MODULE_big = pg_block_systemcatalog
OBJS = pg_block_systemcatalog.o $(WIN32RES)

EXTENSION = pg_block_systemcatalog
DATA = pg_block_systemcatalog--1.0.sql
PGFILEDESC = "pg_block_systemcatalog - Block reference for system catalog."

REGRESS = pg_block_systemcatalog
REGRESS_OPTS = --temp-config=$(top_srcdir)/contrib/pg_block_systemcatalog/pg_block_systemcatalog.conf

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_block_systemcatalog
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
