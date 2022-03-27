FROM alpine:3.15 AS build-base

RUN apk update && apk add --quiet --no-cache \
  build-base \
  make \
  ;

################################################################################

FROM build-base AS procps

RUN apk add --quiet --no-cache \
  git \
  automake \
  autoconf \
  libtool \
  gettext \
  pkgconf \
  gettext-dev \
  ;

WORKDIR /src

ARG PROCPS_VERSION
RUN git clone --depth 1 --single-branch --branch v${PROCPS_VERSION} \
  https://gitlab.com/procps-ng/procps.git /src
COPY patches/procps-${PROCPS_VERSION}* /src
RUN git apply *.patch
RUN ./autogen.sh \
  && ./configure --without-ncurses --without-systemd \
  && make install-libLTLIBRARIES \
  ;

################################################################################

FROM build-base AS minit

ARG PROCPS_VERSION
RUN apk update && apk add --quiet --no-cache \
  # linux-headers \
  # musl-dev \
  procps-dev~=${PROCPS_VERSION} \
  ;

WORKDIR /src

ENV LDFLAGS=-static
COPY --from=procps /usr/local/lib/libprocps.* /usr/local/lib
COPY Makefile minit.c /src
RUN make minit

################################################################################

FROM alpine:3.15 AS test

RUN apk update && apk add --quiet --no-cache \
  valgrind \
  ;

COPY --from=minit /src/minit /usr/local/bin/minit

RUN  valgrind -q /usr/local/bin/minit sleep 1

################################################################################

FROM scratch

COPY --from=minit /src/minit /usr/local/bin/minit

ENTRYPOINT ["minit"]
CMD ["minit"]

USER 65534:65534
