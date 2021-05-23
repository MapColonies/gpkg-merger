all: gpkg-merger

gpkg-merger:
	gcc src/main.c src/gpkg.h src/gpkg.c src/tileBatch.h src/tileBatch.c src/tile.h src/tile.c src/statement.h src/statement.c src/ImageWand/merge.h src/ImageWand/merge.c -lsqlite3 $(pkg-config --cflags --libs MagickWand) -o gpkg-merger

clean: 
	rm gpkg-merger

rebuild: clean all
