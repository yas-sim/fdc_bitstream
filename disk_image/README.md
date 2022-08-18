# C++ class library for the disk image file handling  

Library supports `mfm`, `raw` , `d77` and `hfe` file formats. `disk_image_???` classes derived from `disk_image_base` class.

- `mfm` and `d77` classes overridden `read()` and `write()` function. Those image types can read and write a file.  
- `hfe` and `raw` overridden only `read()` functions.  

- All classes read a disk image file and convert it into the MFM bit stream data internally.  
- When the `write()` function is called, the internal MFM bit stream will be translated into a specific file format and be written.  

