all: gpkg-merger

gpkg-merger:
	gcc src/main.c src/gpkg.h src/gpkg.c src/tileBatch.h src/tileBatch.c src/tile.h src/tile.c src/statement.h src/statement.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -o gpkg-merger

upscaling: 
	gcc src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c `pkg-config --cflags --libs MagickWand` -O3 -g -o upscaling

clean: 
	rm gpkg-merger

rebuild: clean all
