<h1 align="center">minit</h3>

<div align="center">

[![Status](https://img.shields.io/badge/status-active-success.svg)]()
[![GitHub Issues](https://img.shields.io/github/issues/wwmoraes/minit.svg)](https://github.com/wwmoraes/minit/issues)
[![GitHub Pull Requests](https://img.shields.io/github/issues-pr/wwmoraes/minit.svg)](https://github.com/wwmoraes/minit/pulls)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](/LICENSE)

[![Docker Image Size (latest semver)](https://img.shields.io/docker/image-size/wwmoraes/minit)](https://hub.docker.com/r/wwmoraes/minit)
[![Docker Image Version (latest semver)](https://img.shields.io/docker/v/wwmoraes/minit?label=image%20version)](https://hub.docker.com/r/wwmoraes/minit)
[![Docker Pulls](https://img.shields.io/docker/pulls/wwmoraes/minit)](https://hub.docker.com/r/wwmoraes/minit)

</div>

---

<p align="center"> minimal subreaper init for containers
  <br>
</p>

## 📝 Table of Contents

- [About](#about)
- [Getting Started](#getting_started)
- [Running the tests](#tests)
- [Usage](#usage)
- [TODO](./TODO.md)
- [Contributing](./CONTRIBUTING.md)

## 🧐 About <a name = "about"></a>

Minit acts as a init process for your application. It does:

- spawn a child process to run your application
- forward signals to child processes
- adopt orphaned processes
- reaps terminated child processes to prevent zombie processes

### What's the difference between `minit` and `tini`?

| Functionality            | tini | minit |
|--------------------------|------|-------|
| forward signals          |  ✅  |   ✅   |
| adopt orphaned processes |  ✅  |   ✅   |
| reap zombie processes    |  ✅  |   ✅   |
| wait for all processes   |  ❌  |   ✅   |

Even if you start `tini` with the `-s` flag or the `TINI_SUBREAPER` environment
variable set, it won't wait for the all adopted child processes _if the spawned
application exits_. That means if your application A forks/spawns process B and
C, then exits before they finish, `tini` will also finish and still leave
process B and C as zombie processes behind.

### Why would I ever want to adopt all orphaned processes and prevent zombies?

Let's skip the lecture about not running daemons within containers, best
practices of fork/wait pairing or even signaling child processes when the parent
dies. At the end of the day, an application wants a clean exit. This includes
its child processes. An abrupt termination of them can result in unwanted
behaviour, such as state corruption or misleading status.

## 🏁 Getting Started <a name = "getting_started"></a>

You can build the application using `make minit`. You can also build the sample
application using `make sample`, which simply forks a `sleep` system command and
exits, allowing a quick test.

### Prerequisites

The repository contains a developer container that is usable with the VSCode
[`Remote - Containers`][ms-vscode-remote.remote-containers] extension. Due to
this extension lack of support for buildkit, the container build must be done
through a task, which is also provided.

If you're on Linux and want to develop directly, then install:

- GNU Make
- GCC
- Linux headers
- libc-dev or equivalent package
- procps-dev or equivalent package

### Installing

Release versions are built statically, which means you can use `minit` on a
scratch container. In fact the release container is based on scratch, with only
the binary. You can base off it, or just copy the binary from the image:

```dockerfile
FROM minit:latest AS minit

FROM scratch

COPY --from=minit /usr/local/bin/minit /usr/local/bin/minit

# your awesome directives go here

ENTRYPOINT ["minit", "<your-app>"]
CMD ["minit", "<your-app>"]
```

## 🔧 Running the tests <a name = "tests"></a>

The binary is only tested for memory leaks, using valgrind. This is available as
the `test` target on the main Dockerfile, and can be executed using `make test`.

The final container image be tested for its structure using `make image-test`.

## 🎈 Usage <a name="usage"></a>

Prefix any executable with `minit`. It takes any arguments, which will all be
used as-is to execute from the spawned process. For instance, if you want to run
`sleep 10` with `minit`, use:

```shell
minit sleep 10
```

That's it.
