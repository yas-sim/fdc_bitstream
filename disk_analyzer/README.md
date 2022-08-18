# Simple disk image analyzer tool  

|Command|Description|
|-|-|
|`o`  `file_name`|Open an image file.(HFE/MFM/RAW/D77)|
|`rt` `trk`|Read track|
|`ri` `trk`|Read all sector IDs|
|`rs` `trk` `sid` `sct`|Read sector|
|`ef` `nume` `denom`|Enable fluctuator (VFO stops at rate of nume/denom)|
|`ef`|Disable fluctuator|
|`gain` `gl` `gh`|Set VFO gain (low=gl, high=gh)|
|`q`|Quit analyzer|

## Example  
```sh
>analyzer
*** Command list
o  file_name    Open an image file.
rt trk          Read track
ri trk          Read all sector IDs
rs trk sid sct  Read sector
ef nume denom   Enable fluctuator (VFO stops at rate of nume/denom)
ef              Disable fluctuator
gain gl gh      Set VFO gain (low=gl, high=gh)

CMD(1) > o disk.raw
CMD(2) > ri 0
 0 00 00 01 01 fa0c OK
 1 00 00 02 01 af5f OK
 2 00 00 03 01 9c6e OK
 3 00 00 04 01 05f9 OK
 4 00 00 05 01 36c8 OK
 5 00 00 06 01 639b OK
 6 00 00 07 01 50aa OK
 7 00 00 08 01 4094 OK
 8 00 00 09 01 73a5 OK
 9 00 00 0a 01 26f6 OK
10 00 00 0b 01 15c7 OK
11 00 00 0c 01 8c50 OK
12 00 00 0d 01 bf61 OK
13 00 00 0e 01 ea32 OK
14 00 00 0f 01 d903 OK
15 00 00 10 01 ca4e OK
CMD(3) > rs 0 0 1
1a 50 86 fd 1f 8b 0d 0f 8e 01 1a c6 0f ad 9f fb fa 6c 02 6c 05 5a 26 f5 20 08 0a 00 02 00 00 02 00 00 1a 50 0c 0f 17 00 cc 17 00 e1 10 8e 02 ef 8e 0b df ce c0 00 17 04 99 8e c0 00 17 05 60 17
03 5f 86 05 17 01 22 17 01 01 10 8e 01 d5 8e 10 00 17 03 62 8e 10 00 17 01 23 10 8e 01 e1 8e 10 00 17 03 52 8e 10 00 17 01 13 ce 40 44 17 01 40 10 8e 01 db 8e 10 00 17 03 3c 8e 10 00 17 00 fd
ce 40 00 17 01 2a 86 04 17 00 de 86 06 17 00 d9 86 04 b7 fd 02 30 8d 05 61 bf ff f8 4f b7 06 f8 17 06 b2 1c ef 17 01 6b 17 06 58 10 8e 01 ed 8e 80 00 17 03 01 10 8e 0c 00 8e 80 00 ce c0 00 17
01 14 10 8e 01 e7 8e 80 00 17 02 ea b6 01 f3 f6 02 4a 7e 80 00 41 54 54 4c 45 31 41 54 54 4c 45 32 41 54 54 4c 45 33 41 50 52 47 20 20 41 53 42 50 20 20 ff 00 8e fd 15 86 04 a7 84 a6 01 6f 84

CRC DAM  RNF --ID_POS-- --DAM_POS- SIZE
OK  DAM  OK  0000013682 0000019508 256
CMD(4) > rt 0
9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c
9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 9c 00 00 00 00 00 00 00 00 00 00 00 01 c2 a1 a1 a1 fe 00 00 01 01 fa 0c 4e 4e 4e 4e 4e 4e 4e 4e 4e 4e 4e 4e 4e 4e
4e 4e 4e 4e 4e 4e 4e 4e 00 00 00 00 00 00 00 00 00 00 00 00 02 c2 a1 a1 a1 fb 1a 50 86 fd 1f 8b 0d 0f 8e 01 1a c6 0f ad 9f fb fa 6c 02 6c 05 5a 26 f5 20 08 0a 00 02 00 00 02 00 00 1a 50 0c 0f
  :     :     :     :
00 00 00 00 00 00 00 e1 22 4e 08 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 48 7f ff ff

CMD(5) > q
```
