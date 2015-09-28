MODULE_big = skyband
OBJS = skyband.o sky_bucket_list.o sky_point_list.o sky_hashtable.o sky_domi_list.o
DATA_built = skyband.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

sky_bucket_list.o: sky_bucket_list.c sky_bucket_list.h
sky_point_list.o: sky_point_list.c sky_point_list.h
sky_hashtable.o: sky_hashtable.c sky_hashtable.h
sky_domi_list.o: sky_domi_list.c sky_domi_list.h
