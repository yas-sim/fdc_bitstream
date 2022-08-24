# Multi format supported disk image converter.  

**Usage:**
```
image_converter -i input_file.[mfm|raw|hfe|d77] -o output_file.[mfm|d77] -n
```

|Option|Option parameters|Description|
|-|-|-|
|`-i`|`in_file`|Specify input file. HFE, MFM, RAW and D77 are supported.|
|`-o`|`out_file`|Specify output file. MFM and D77 are supported.|
|`-n`||Normalize pulse pitch.<br>The converter will collect the statistics data of pulse pitch (pulse-to-pulse distance) for each track data and perform pitch normalization during the image format conversion process.  With normalized data, the VFO operation will get stable and reading accuracy may improve.|
|`-vfo`|`vfo_type`|Specify VFO type. This option is effective only when D77 is specified as output.<br>0:vfo_simple, 1:vfo_fixed, 2:vfo_pid, 3:vfo_pid2, 9:vfo_experimental|
|`-gain`|`low` `high`|VFO gain setting. Effective only for D77.  e.g. `-gain 1.0 2.0`|
|`-v`||Verbose mode.|

Note: HFE and RAW don't support the write function, so they can't be specified as output format.  

- Supported image format:
    - HFE : HxC disk emulator format (input only)
    - MFM : My original MFM bit stream format (input and output)
    - RAW : My original MFM bit stream format captured by [floppy_disk_shield_2d for Arduino](https://github.com/yas-sim/floppy_disk_shield_2d) (input only)
    - D77 : D77/D88 disk image format (input and output)
