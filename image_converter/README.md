# Multi format supported disk image converter.  

```
image_converter input_file.[mfm|raw|hfe|d77] output_file.[mfm|d77]
```

- Supported image:
    - Input
        - HFE : HxC disk emulator format
        - MFM : My original MFM bit stream format
        - RAW : My original MFM bit stream format captured by [floppy_disk_shield_2d for Arduino](https://github.com/yas-sim/floppy_disk_shield_2d)
        - D77 : D77/D88 disk image format
    - Output
        - MFM
        - D77

Note: HFE and RAW don't support the write function, so they can't be specified as output format.  

