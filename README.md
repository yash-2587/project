# CUDA at Scale Independent Project

Assignment for the Coursera course [CUDA at Scale for the Enterprise](https://www.coursera.org/learn/cuda-at-scale-for-the-enterprise).

## Project description

Apply box filters to all images in a folder, using NPP's `nppiFilterBoxBorder_8u_C1R` routine.
It's heavily based on nvidia's sample code https://github.com/NVIDIA/cuda-samples/tree/master/Samples/4_CUDA_Libraries/boxFilterNPP

## Code organization

`boxFilterNPP.cpp` is the main implementation, it loops through all files in the input folder, reads the image,
copies to device, applies the filter, and writes the result to the output folder.

`Common` contains the helper code for error checking and image file handling.

`data` contains example input images.

`output` contains output for the example inputs.

`Makefile` for compiling the code.

`run.sh` contains scripts for building the code and running on example inputs.

## How to run

Run `run.sh`.

Use the `-input` command line flag to specify the input folder to use.

The code is tested on ubuntu 20.04 and nvcc 11.6.
