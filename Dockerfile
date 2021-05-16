FROM ubuntu:20.04 as builder

ENV CGO_ENABLED=0

RUN apt-get update -y && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends tzdata

RUN apt-get update -y && apt-get upgrade -y && apt-get install -y build-essential sqlite3 libsqlite3-dev git-all pkg-config

WORKDIR /opt/gpkg-merger

COPY ./src /opt/gpkg-merger

RUN git clone https://github.com/ImageMagick/ImageMagick.git ImageMagick-7.0.11 \
    && cd ImageMagick-7.0.11 \
    && ./configure \
    && make \
    && make install \
    && ldconfig /usr/local/lib \
    && make check

RUN gcc main.c gpkg.h gpkg.c tileBatch.h tileBatch.c tile.h tile.c statement.h statement.c ImageWand/merge.h ImageWand/merge.c -lsqlite3 -lpthread $(pkg-config --cflags --libs MagickWand)

FROM python:3.6.13-alpine3.13

WORKDIR /app

RUN apk update \
    && apk add --virtual build-deps gcc python3-dev musl-dev \
    && apk add jpeg-dev zlib-dev libjpeg \
    && pip install Pillow \
    && apk del build-deps

COPY --from=builder /opt/gpkg-merger/a.out /app

COPY ./src/merge.py .

RUN ln -s /app/a.out /bin/a.out

RUN pip install Pillow

CMD ["a.out"]