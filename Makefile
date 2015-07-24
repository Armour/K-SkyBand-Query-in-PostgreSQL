MODULE_big = skyline
OBJS = skyline.o BucketVector.o PointVector.o HashTable.o
DATA_built = skyline.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

BucketVector.o: BucketVector.c BucketVector.h
PointVector.o: PointVector.c PointVector.h
HashTable.o: HashTable.c HashTable.h
