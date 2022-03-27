FROM alpine:3.15

RUN adduser -h /home/vscode -g "" -s /bin/sh -G users -D -u 10001 vscode

RUN echo @testing http://dl-cdn.alpinelinux.org/alpine/edge/testing >> /etc/apk/repositories
RUN apk update && apk add --quiet --no-cache \
  moreutils \
  git \
  make \
  build-base \
  # minit
  linux-headers \
  musl-dev \
  procps-dev~=3.3.17 \
  ;
