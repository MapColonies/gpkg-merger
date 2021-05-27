all: gpkg-merger

gpkg-merger:
	gcc src/main.c src/gpkg.h src/gpkg.c src/tileBatch.h src/tileBatch.c src/tile.h src/tile.c src/statement.h src/statement.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -o gpkg-merger

upscaling: clean-upscaling prepare-upscaling build-upscaling run-upscaling

build-upscaling: 
	gcc src/ImageWand/main.c src/threadPool/threadPool.c src/threadPool/threadPool.h src/ImageWand/upscaling.h src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c src/statement.h src/statement.c src/gpkg.c src/gpkg.h src/tileBatch.c src/tileBatch.h src/tile.h src/tile.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -O3 -g -o upscaling

clean-upscaling:
	rm -f upscaling /home/roees/Documents/gpkgs/artzi_fixed.gpkg

prepare-upscaling:
	cp /home/roees/Documents/gpkgs/artzi_fixed_DO_NOT_TOUCH.gpkg /home/roees/Documents/gpkgs/artzi_fixed.gpkg

run-upscaling:
	./upscaling

debug-upscaling: 
		rm -f upscaling /home/roees/Documents/gpkgs/artzi_fixed.gpkg && \
		cp /home/roees/Documents/gpkgs/artzi_fixed_DO_NOT_TOUCH.gpkg /home/roees/Documents/gpkgs/artzi_fixed.gpkg && \
		rm -f valgrind-out.txt && \
		gcc src/ImageWand/main.c src/threadPool/threadPool.c src/threadPool/threadPool.h src/ImageWand/upscaling.h src/ImageWand/upscaling.c src/ImageWand/wandUtil.h src/ImageWand/wandUtil.c src/statement.h src/statement.c src/gpkg.c src/gpkg.h src/tileBatch.c src/tileBatch.h src/tile.h src/tile.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 `pkg-config --cflags --libs MagickWand` -O3 -g -o upscaling && \
		valgrind --log-file=valgrind-out.txt ./upscaling	


clean: 
	rm gpkg-merger

rebuild: clean all
