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
|`vfo`|Display current VFO parameters|
|`vv` `trk` [`vfo_type`]|VFO visualizer. Read 5,000 pulses from the top of a track using specified type of VFO.|
|`sv` `vfo_type`|Select VFO type.|
|`rv`|(soft) reset VFO|
|`histogram` `trk`|Display histogram of data pulse distances in a track|
|`q`|Quit analyzer|
* Note1: The number starting with '$' will be handled as hexadecimal value (e.g. **$f7** == **247**)  
* Note2: **VFO type** = 0:vfo_fixed, 1:vfo_simple, 2:vfo_pid, 3:vfo_pid2, 9=experimental.  

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
```
* Histogram  
	* Shows distribution of the pulse-to-pulse distance in a track.  
  * You can check the image quality. The more clear separation of pulse distribution, the better quality.  
  * Also, you can check the **actual** bit rate of the pulses. The actual bit rate can vary due to spindle motor speed fluctuation or incorrect spindle motor speed calibration.  

```sh
CMD(5) > histogram 0
#clocks  #pulses
   0 :        0 :
   1 :        3 :
   2 :        1 :
   3 :        1 :
   4 :        0 :
   5 :        0 :
   6 :        0 :
   7 :        0 :
   8 :        1 :
   9 :        2 :
  10 :        3 :
  11 :        2 :
  12 :        2 :
  13 :        4 :
  14 :        0 :
  15 :      236 : *
  16 :    14395 : ****************************************************************************************************
  17 :    10734 : **************************************************************************
  18 :      192 : *
  19 :        4 :
  20 :        1 :
  21 :        2 :
  22 :       21 :
  23 :     1668 : ***********
  24 :     6770 : ***********************************************
  25 :     3705 : *************************
  26 :      208 : *
  27 :        0 :
  28 :        2 :
  29 :        0 :
  30 :        2 :
  31 :      228 : *
  32 :     1378 : *********
  33 :      692 : ****
  34 :       22 :
  35 :        0 :
  36 :        0 :
  37 :        0 :
  38 :        0 :
  39 :        3 :
  40 :        1 :
  41 :        0 :

Peaks:
1 : 16 [CLKs]
2 : 24 [CLKs]
3 : 32 [CLKs]

Estimated bit cell width : 8 [CLKs] (CLK=4 MHz)
Data bit rate : 500 [Kbits/sec]
```
* VFO Visualizer
	* Visualises the operation of software VFO.   
  * You can try different type of VFOs with `sv` command.  

|Symbol|Meaning|
|--|--|
|`>`, `<`|Bit cell width.|
|`-`|Data window period. The data pulse must be in this period. The data window width should be 50% of the bit cell width in MFM/2D standard, but this library uses 75% for easy data reading. You can tweak the data window ratio with an API. |  
|`P`|Data pulse|

```sh
CMD(6) > vv 0
     0 >P     -------------------------------------       <
     1 > P    -------------------------------------      <
     2 >  P   -------------------------------------      <
     3 >    P -------------------------------------      <
     4 >      ------P-----------------------------       <
     5 >      ----------P-------------------------      <
     6 >      --------------P---------------------      <
     7 >      -----------------P------------------      <
     8 >      ---------------------P--------------      <
     9 >      -----------------------P------------      <
    10 >      --------------------------------P---      <
    11 >      ----------------------------------P-       <
    12 >      ------------------------------------P      <
    13 >      ------------------------------------- P    <
    14 >      -------------------------------------  P   <
    15 >      -------------------------------------  P   <
    16 > P    -------------------------------------       <
    17 >  P   -------------------------------------      <
    18 >    P -------------------------------------      <
    19 >      P------------------------------------      <
    20 >      ---P---------------------------------      <
    21 >      -----P------------------------------       <
    22 >      --------------P---------------------      <
    23 >      ------------------P-----------------      <
    24 >      ---------------------P--------------      <
    25 >      -------------------------P----------      <
    26 >      ----------------------------P-------      <
    27 >      ------------------------------P-----       <
    28 >      -------------------------------------P     <
    29 >      -------------------------------------  P   <
     :                          :                        :
  4977 >      ---------------------P----------------       <
  4978 >      -------------------P------------------       <
  4979 >      ----------------------P---------------       <
  4980 >      --------------------P-----------------       <
  4981 >      ------------------P-------------------       <
  4982 >      ---------------P----------------------       <
  4983 >      -------------------P------------------       <
  4984 >      -----------------P--------------------       <
  4985 >      ---------------------P----------------       <
  4986 >      -------------------P------------------       <
  4987 >      ----------------------P---------------       <
  4988 >      -------------------P------------------       <
  4989 >      ----------------------P---------------       <
  4990 >      -------------------P------------------       <
  4991 >      -----------------------P--------------       <
  4992 >      --------------------P-----------------       <
  4993 >      ------------------------P-------------       <
  4994 >      -------------------P------------------       <
  4995 >      -----------------------P--------------       <
  4996 >      --------------------P-----------------       <
  4997 >      -----------------------P--------------       <
  4998 >      -------------------P------------------       <
  4999 >      ----------------------P---------------       <
-- vfo_base --
Cell_size : 8.22547
Cell_size_ref : 8
Window ratio  : 0.75
Window size   : 6.1691
Windoe offset : 1.02818
Gain (Low)    : 1
Gain (High)   : 2
Current gain  : 1
-- vfo_simple --
Freq integral  : -2.25465
CMD(7) > q
```
