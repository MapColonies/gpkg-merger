all: gpkg-merger

gpkg-merger: clean-gpkg-merger prepare-gpkg-merger build-gpkg-merger run-gpkg-merger

build-gpkg-merger: 
	gcc src/ImageWand/main.c src/threadPool/threadPool.c src/threadPool/threadPool.h src/ImageWand/upscaling.h src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c src/statement.h src/statement.c src/gpkg.c src/gpkg.h src/tileBatch.c src/tileBatch.h src/tile.h src/tile.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -O3 -g -o upscaling

build-debug-gpkg-merger: 
	gcc src/ImageWand/main.c src/threadPool/threadPool.c src/threadPool/threadPool.h src/ImageWand/upscaling.h src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c src/statement.h src/statement.c src/gpkg.c src/gpkg.h src/tileBatch.c src/tileBatch.h src/tile.h src/tile.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -Og -g -o upscaling

clean-gpkg-merger:
	rm -f gpkg-merger ./artzi_fixed.gpkg

prepare-gpkg-merger:
	cp ./artzi_fixed_DO_NOT_TOUCH.gpkg ./artzi_fixed.gpkg

run-gpkg-merger:
	./gpkg-merger

clean: 
	rm gpkg-merger

rebuild: clean all
