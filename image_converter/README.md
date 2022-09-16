# Multi format supported disk image converter.  

**Usage:**  
1. Simple conversion
```
image_converter -i input_file.[mfm|raw|d77|hfe|fdx] -o output_file.[mfm|d77|hfe|fdx] -v
```
2. Merge images to generate a chimera image

```
image_converter -i input_file1.[mfm|raw|d77|hfe|fdx] [-i input_file2.[mfm|raw|d77|hfe|fdx]] [-i ...] -o output_file.[mfm|d77|hfe|fdx] -v
```

|Option|Option parameters|Description|
|-|-|-|
|`-i`|`in_file`|Specify input file. MFM, RAW, D77, HFE, and FDX are supported.<br>The image converter can accept **multiple image files** as input. The converter will inspect all tracks in the images, select the best tracks among the images, and generates a merged image (a chimera image).|
|`-o`|`out_file`|Specify output file. MFM, D77, HFE, and FDX are supported.|
|`-n`||Normalize pulse pitch.<br>The converter will collect the statistics data of pulse pitch (pulse-to-pulse distance) for each track data and perform pitch normalization during the image format conversion process.  With normalized data, the VFO operation will get stable and reading accuracy may improve.|
|`-vfo`|`vfo_type`|Specify VFO type. This option is effective only when D77 is specified as output.<br>0:vfo_simple, 1:vfo_fixed, 2:vfo_pid, 3:vfo_pid2, 9:vfo_experimental|
|`-gain`|`low` `high`|VFO gain setting. Effective only for D77.  e.g. `-gain 1.0 2.0`|
|`-v`||Verbose mode.|

Note: HFE and RAW don't support the write function, so they can't be specified as output format.  

- Supported image formats:
    - HFE : HxC disk emulator format (input only)
    - MFM : My original MFM bit stream format (input and output)
    - RAW : My original MFM bit stream format captured by [floppy_disk_shield_2d for Arduino](https://github.com/yas-sim/floppy_disk_shield_2d) (input only)
    - D77 : D77/D88 disk image format (input and output)
    - FDX : Disk image format for [FDX68, FDD emulator](http://retropc.net/gimons/fdx68/) 