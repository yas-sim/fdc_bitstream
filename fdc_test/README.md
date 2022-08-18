# fdc_bitstream test program

This is a test program to check the function and API integrity of the fdc_bitstream class and disk_image_??? classes.  

|test func|test contents|
|-|-|
|`test1()`|Read a HFM file / Read track / Dump sector IDs|
|`test2()`|Format a track (1 sector) / Read track / Write sector / Read track / Read sector|
|`test3()`|Format a track (16 sectors) / Dump sector IDs / Read sector / Write 16 sectors with data / Read Track / Read sector data |
|`test4()`|'Corocoro (unstable data)' protect test. Format a track and then place random pulses in the desired region to make the data in the region unstable. Performs read sector operation multiple times with fluctuator enabled to check whether the fdc_bitstream can generate random data on the unstable region.|
