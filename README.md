# Rectilinear Polygon Extractor

## Description

This is a tool that converts an image with a transparent background to a recilinear polygone. It outputs the points as
JSON or SVG if the program is ran with `--output-as-svg`.

## Catch

This program can't handle:

- Single pixel images
	```
	+---+---+---+
	|   |   |   |
	+---+---+---+
	|   | * |   |
	+---+---+---+
	|   |   |   |
	+---+---+---+
	```
- Images with a protruding single pixel
	```
	+---+---+---+
	|   | * |   |
	+---+---+---+
	| * | * | * |
	+---+---+---+
	|   | * |   |
	+---+---+---+
	```
- Images that contain a multiple sections
	```
	+---+---+---+---+---+---+---+---+---+
	|   | * | * | * | * |   |   |   |   |
	+---+---+---+---+---+---+---+---+---+
	|   | * | * | * | * |   | * | * | * |
	+---+---+---+---+---+---+---+---+---+
	| * | * | * | * | * |   | * | * | * |
	+---+---+---+---+---+---+---+---+---+
	| * | * | * | * | * |   | * | * | * |
	+---+---+---+---+---+---+---+---+---+
	| * | * | * | * | * |   | * | * | * |
	+---+---+---+---+---+---+---+---+---+
	|   |   |   |   |   |   | * | * | * |
	+---+---+---+---+---+---+---+---+---+
	```

## Build

```bash
gcc -o nobuild nobuild.c
./nobuild
```
