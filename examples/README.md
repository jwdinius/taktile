# taktile/examples

## Prerequisites

To run the TLS/SSL examples, you will need to create certificates. The [`run_taky`](./taky/run_taky)
script creates certificates - both for client and server - that you can use when running these examples.
The script checks if these certificates exist before creating them. If the certificates already exist,
they are not recreated.

## Examples

Launch arguments and example commands are provided as docstrings within each example. Below are brief
summaries for each example:

* [`send`](./send.cpp) - modeled after the Python script of the same name from [`pytak`](https://pytak.readthedocs.io/en/latest/examples/#send-tak-data).
You will need TAK server running to check that this example is working correctly. You can
execute a [`taky`](https://github.com/tkuester/taky) server by running the [`run_taky`](./taky/run_taky) script.
* [`send_receive`](./send_receive.cpp) - modeled after the Python script of the same name from [`pytak`](https://pytak.readthedocs.io/en/latest/examples/#send-receive-tak-data).
This example is self-contained and needs no other processes running to verify the example is working
correctly.
