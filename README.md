# JPEG Encoder (C++)

A minimal baseline JPEG encoder implemented in C++.

This project implements a complete baseline sequential DCT JPEG pipeline including:
- PPM loading
- Color space conversion
- Block-based DCT and quantization
- Zig-zag reordering
- Run-length encoding (RLE)
- Huffman entropy coding
- Bit-level output and JPEG marker/writer

Notes: 
- The table for the quantization was the Annex K Table.
- The Huffman Table used the standard DC and AC values.
- The chroma subsampling is 4:4:4, meaning there is no subsampling.

## Build

Requirements:
- A C++ compiler supporting C++20 (e.g., `g++`)
- `make`

Build with:

```bash
make
```

The build produces the encoder binary at `bin/jpeg`.

## Usage

Basic usage (example):

```bash
./bin/jpeg images/image.ppm output.jpg
```

Check `Makefile` for exact targets and output paths.

## Project layout

- `src/` — encoder source files
  - `main.cpp` — program entry
  - `ppm.*` — PPM image loader/utility
  - `dct.*` — discrete cosine transform implementation
  - `zigzag.*` — zig-zag reorder functions
  - `rle.*` — run-length encoding for AC coefficients
  - `huffman_encoding.*` — Huffman table generation and encoding
  - `bitwriter.*` — bit-level output helper for writing JPEG streams
  - `jpeg.*` / `encode.*` — high-level JPEG encoding and marker output

- `images/` — example input PPMs
- `bin/` — build output (binaries)
- `obj/` — object files and intermediate build artifacts

## Notes

- This is a learning/educational implementation and is not intended as a production-grade JPEG library.
