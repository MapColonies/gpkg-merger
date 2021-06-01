all: gpkg-merger

gpkg-merger: clean-gpkg-merger prepare-gpkg-merger build-gpkg-merger run-gpkg-merger

gpkg-merger-debug: clean-gpkg-merger prepare-gpkg-merger build-debug-gpkg-merger run-gpkg-merger-debug

build-gpkg-merger: 
	gcc src/main.c src/threadPool/threadPool.c src/threadPool/threadPool.h src/ImageWand/upscaling.h src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c src/statement.h src/statement.c src/gpkg.c src/gpkg.h src/tileBatch.c src/tileBatch.h src/tile.h src/tile.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -O3 -g -o gpkg-merger

build-debug-gpkg-merger: 
	gcc src/main.c src/threadPool/threadPool.c src/threadPool/threadPool.h src/ImageWand/upscaling.h src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c src/statement.h src/statement.c src/gpkg.c src/gpkg.h src/tileBatch.c src/tileBatch.h src/tile.h src/tile.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -Og -g -o gpkg-merger-debug

clean-gpkg-merger:
	rm -f gpkg-merger ./artzi_fixed.gpkg

prepare-gpkg-merger:
	cp /home/roees/Documents/gpkgs/artzi_fixed_DO_NOT_TOUCH.gpkg /home/roees/Documents/gpkgs/artzi_fixed.gpkg

run-gpkg-merger:
	./gpkg-merger /home/roees/Documents/gpkgs/artzi_fixed.gpkg /home/roees/Documents/gpkgs/area1.gpkg 1000

run-gpkg-merger-debug:
	gdb -ex=r ./gpkg-merger-debug /home/roees/Documents/gpkgs/artzi_fixed.gpkg /home/roees/Documents/gpkgs/area1.gpkg 1000

clean: 
	rm gpkg-merger

rebuild: clean all
