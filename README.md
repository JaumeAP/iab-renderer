# Introduction

IABLib is a C++ library for creating, parsing and rendering IAB essence, as
specified in SMPTE ST 2098-2.

# License

See [LICENSE](./LICENSE).

# Documentation

Documentation is provided inline using [Doxygen](https://www.doxygen.nl/)
conventions. The build system can generate an HTML rendering.

# Examples

The [unit tests](./tests) and the [render-to-files application](./src/apps/)
illustrate the use of the library.

# Objects (this fork)

[objects/](./objects) is not part of IABLib: it is object pan, movement and automation code on top of IABLib
(`PanEvent`, `ObjectTrack`, the pan reader and writer, the pan profiles, ADM-style block sampling), shared by
New-Renderer and Cinema-MXF, which add it with `add_subdirectory` after IABLib. Nothing in IABLib depends on it.
State in [objects/docs/state.md](./objects/docs/state.md).

# Dependencies

The library has no runtime dependencies.

The following development dependencies exist:

* [GoogleTest](https://github.com/google/googletest) (unit tests)
* [Doxygen](https://www.doxygen.nl/) (documentation)
