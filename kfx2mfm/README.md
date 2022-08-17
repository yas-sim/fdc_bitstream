# Simple Kryoflux RAW image file(s) to MFM image converter  

## How to use:  
```sh
python kfx2mfm.py -i <input_raw_directory> 
```
The convert will generate "`<input_raw_directory>.mfm`" file as output.  

Kryoflux RAW image directory may contain RAW track files like below:  
```
2019/02/01  18:01           251,829 track00.0.raw
2019/02/01  18:01           256,814 track00.1.raw
2019/02/01  18:01           245,475 track01.0.raw
2019/02/01  18:01           276,201 track01.1.raw
2019/02/01  18:02           219,762 track02.0.raw
    :         :                :         :
2019/02/01  18:03           193,485 track41.0.raw
2019/02/01  18:03           181,097 track41.1.raw
```
