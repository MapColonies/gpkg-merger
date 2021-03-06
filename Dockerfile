FROM ubuntu:18.04 as builder

ENV CGO_ENABLED=0
ENV DEBIAN_FRONTEND=noninteractive
ENV PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
ENV BLOB_MEMORY_ALLOCATION=10000000

RUN apt-get update -y && apt-get upgrade -y && apt-get install -y tzdata build-essential sqlite3 libsqlite3-dev git-all pkg-config libpng-dev libjpeg-dev imagemagick valgrind

RUN git clone https://github.com/ImageMagick/ImageMagick.git ImageMagick-7.0.11 \
    && cd ImageMagick-7.0.11 \
    && ./configure \
    && make \
    && make install \
    && ldconfig /usr/local/lib
# && make check  

WORKDIR /opt/gpkg-merger

COPY ./src /opt/gpkg-merger

RUN make build-gpkg-merger

CMD ["./gpkg-merger"]
