all: gpkg-merger

gpkg-merger: clean-gpkg-merger prepare-gpkg-merger build-gpkg-merger run-gpkg-merger

gpkg-merger-debug: clean-gpkg-merger prepare-gpkg-merger build-debug-gpkg-merger run-gpkg-merger-debug

build-gpkg-merger: 
	gcc main.c threadPool/threadPool.c threadPool/threadPool.h ImageWand/upscaling.h ImageWand/upscaling.c ImageWand/wandUtil.h ImageWand/wandUtil.c statement.h statement.c env.h env.c extent.h gpkg.c gpkg.h tileBatch.c tileBatch.h tile.h tile.c ImageWand/merge.h ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -O3 -g -o gpkg-merger

build-debug-gpkg-merger: 
	gcc main.c threadPool/threadPool.c threadPool/threadPool.h ImageWand/upscaling.h ImageWand/upscaling.c ImageWand/wandUtil.h ImageWand/wandUtil.c statement.h statement.c env.h env.c extent.h gpkg.c gpkg.h tileBatch.c tileBatch.h tile.h tile.c ImageWand/merge.h ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -Og -g -o gpkg-merger-debug

clean-gpkg-merger:
	rm -f gpkg-merger /opt/gpkgs/artzi_fixed.gpkg

prepare-gpkg-merger:
	cp /opt/gpkgs/artzi_fixed_DO_NOT_TOUCH.gpkg /opt/gpkgs/artzi_fixed.gpkg

run-gpkg-merger:
	./gpkg-merger /opt/gpkgs/artzi_fixed.gpkg /opt/gpkgs/area1.gpkg 1000

run-gpkg-merger-debug:
	gdb -ex=r ./gpkg-merger-debug /opt/gpkgs/artzi_fixed.gpkg /opt/gpkgs/area1.gpkg 1000

clean: 
	rm gpkg-merger

rebuild: clean all
