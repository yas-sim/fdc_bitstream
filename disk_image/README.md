# C++ class library for the disk image file handling  

Library supports `mfm`, `raw`, `d77`, `hfe` and `fdx` file formats.  
The `disk_image_???` classes derived from `disk_image_base` class.

- All classes read a disk image file and convert it into the MFM bit stream data internally.  
- When the `write()` function is called, the internal MFM bit stream will be translated into a specific file format and be written.  
