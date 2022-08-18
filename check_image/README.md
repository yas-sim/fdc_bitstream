# check_image - Disk image file check tool  

A tool to check disk image data integrity.  

|Command Options|Description|
|-|-|
|`-i` `input_file`|Input file to check|
|`-td`|Perform track dump|
|`-gain` `gain_l` `gain_h`|VFO gain (real numbers). Try 0.1~12.0.|
|`-ts` `track_number`|Start track #|
|`-te` `track_number`|End track #|

- `gain_l` will be used for normal data reading (other than SYNC field).
- `gain_h` will be used when SYNC filed is being read.
