-include .env
export

CC = gcc
CFLAGS ?= -Wall -Os
LDFLAGS += $(shell pkg-config --libs-only-L libprocps)
LDLIBS += $(shell pkg-config --libs-only-l libprocps)

.PHONY: build
build: minit sample

.PHONY: run
run: minit sample
	$(info running $<...)
	@exec ./minit ./sample 3

.PHONY: test
test:
	@docker build --load --target test \
		--build-arg PROCPS_VERSION=3.3.17 \
		-t minit:test .

.PHONY: image-test
image-test:
	@docker build --load \
		--build-arg PROCPS_VERSION=3.3.17 \
		-t minit:latest .
	@container-structure-test test -c structure-test.yaml -i minit:latest

.PHONY: clean
clean:
	@${RM} minit
	@${RM} sample

minit: minit.c
	$(info building $@...)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

sample: sample.c
	$(info building $@...)
	@$(CC) $(CFLAGS) -o $@ $^

.PHONY: kill
kill: kill-sample kill-minit

.PHONY: kill-%
kill-%:
	@pidof $* | ifne xargs kill -INT

.PHONY: pids
pids:
	@ps -o pgid,ppid,pid,comm,args | awk '$$4 == "minit" || $$4 == "sample" || $$4 == "sleep"'

image:
	@docker build --load \
		--build-arg PROCPS_VERSION=${PROCPS_VERSION} \
		-t minit:latest .
