# Contributing to sd_sockets

## Licensing

Any contribution that you make to this repository will
be under the Apache 2 License, as dictated by that
[license](http://www.apache.org/licenses/LICENSE-2.0.html):

~~~
5. Submission of Contributions. Unless You explicitly state otherwise,
   any Contribution intentionally submitted for inclusion in the Work
   by You to the Licensor shall be under the terms and conditions of
   this License, without any additional terms or conditions.
   Notwithstanding the above, nothing herein shall supersede or modify
   the terms of any separate license agreement you may have executed
   with Licensor regarding such Contributions.
~~~

## Use Non-Boost Asio

To minimize dependencies, this library only uses the non-Boost version of Asio.
Changes made that require Boost as a dependency will not be accepted.

## Style Guide

We follow the ROS2 format configuration provided in the `.clang-format` file in
the root directory.
Please make sure all code submitted is formatted using this file.
Beyond formatting, we do our best to adhere to the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and
we ask that you do likewise.
Line endings should be Unix-style (`\n`), so if you are developing on Windows
please make sure this is the case.

## New Issues

If you spot an issue in the library or identify an improvement you'd like to
see, first search the open and closed issues to see if it has been raised
previously.
If it hasn't, please create a new one.
Give it a descriptive title and provide a detailed explanation of the problem or
improvement it is about.
There are issue templates provided that you may use, but following them is not
required so long as the issue is clear and descriptive.

## Pull Requests

All pull requests should be linked to an issue that describes the problem being
solved or the improvement being made, and why it should be.
Please also do a self-review of your pull request before requesting approval.

## Versioning

This project follows [semantic versioning](https://semver.org/).
Each pull request should increment the version number appropriately according to
the changes being made.
If your PR is merged, the merge commit on the main branch will then be tagged
with the new version number.

The version number for the project is found at the top of the [CMakeLists.txt](./CMakeLists.txt):

```cmake
project("sd_sockets"
  VERSION 1.1.1
  DESCRIPTION "A header-only C++ library that provides easy-to-use TCP clients and servers."
  HOMEPAGE_URL "https://github.com/streetdrone-home/sd_sockets"
)
```
