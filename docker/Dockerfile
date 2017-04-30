FROM base/archlinux

ENV TUERLOCATION /tuerkrams
ENV PCSC_READER_PATTERN Contactless

COPY libgcrypt-1.7.0-2.pacman $TUERLOCATION/libgcrypt-1.7.0-2.pacman

RUN pacman --noconfirm -Syu
RUN pacman --noconfirm -S pcsclite ccid base-devel cmake git
RUN pacman --noconfirm -U $TUERLOCATION/libgcrypt-1.7.0-2.pacman

RUN git clone https://github.com/florolf/libmf.git $TUERLOCATION/libmf
RUN git clone https://github.com/entropia/tuerd.git $TUERLOCATION/tuerd

# build libmf
RUN mkdir $TUERLOCATION/libmf/build
WORKDIR $TUERLOCATION/libmf/build
RUN cmake $TUERLOCATION/libmf
RUN make

# build deploy_club
ENV LD_LIBRARY_PATH $TUERLOCATION/libmf/build/src
WORKDIR $TUERLOCATION/tuerd/manage
RUN gcc -o deploy_club \
    $TUERLOCATION/libmf/example/pcsc_common.c \
    deploy_club.c \
    -I ../ \
    -I $TUERLOCATION/libmf/include \
    -L$TUERLOCATION/libmf/build/src \
    -lmf \
    -pthread \
    -I /usr/include/PCSC \
    -std=gnu99 \
    -lgcrypt \
    -lpcsclite

ENTRYPOINT "pcscd" && /bin/bash
