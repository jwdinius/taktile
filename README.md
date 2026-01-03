# `taktile`

A C++ client library for sharing data with Team Awareness Kit (TAK) network participants.

This library is inspired by the excellent [`pytak`](https://pytak.readthedocs.io/en/latest/)
project and reproduces much of the functionality for C++ projects.

For details about TAK protocols, see the description [here](https://github.com/deptofdefense/AndroidTacticalAssaultKit-CIV/blob/4.6.0.5/takproto/README.md).
This library implements the following protocols:

* [V0] - _Cursor-on-Target XML messages_
* [V1 (Mesh/Stream)] - _protocol buffers for minimal serialization footprint_

Protocols V0 and V1 (Mesh) are supported for UDP, TCP, and TLS network protocols.
Protocol V1 (Stream) is supported for TCP and TLS protocols _only_;
ordered delivery is not guaranteed for UDP message fragments.

Get started by checking out the [unit tests](./tests) or the [examples](examples/README.md).

## Contributing to `taktile`

There's a VS Code [devcontainer](https://code.visualstudio.com/docs/devcontainers/tutorial)
provided to ease setup of the developer environment. Familiarize yourself with the dependencies
by looking at the [Dockerfile](./.devcontainer/Dockerfile).
