# Kryoflux raw stream data to FD-Shield bit stream data converter
import sys
import os
import re
import glob
import argparse

class KFX_stream:
    def __init__(self):
        self.pos = 0
        pass

    def open(self, file):
        with open(file, 'rb') as f:
            self.streamdata = f.read()
    
    def get_byte(self, pos=-1):
        if pos == -1:
            if self.pos < len(self.streamdata):
                res = self.streamdata[self.pos]
                self.pos += 1
                return res
            return -1   # end of file
        else:
            if pos < len(self.streamdata):
                res = self.streamdata[pos]
                return res
            return -1
            
    def get_word(self, pos=-1):
        res = 0
        if pos == -1:
            dt1 = self.get_byte()
            if dt1 == -1:
                return -1            
            dt2 = self.get_byte()
            if dt2 == -1:
                return -1
            res = (dt2<<8) | dt1
            return res
        else:
            dt1 = self.get_byte(pos)
            if dt1 == -1:
                return -1            
            dt2 = self.get_byte(pos+1)
            if dt2 == -1:
                return -1
            res = (dt2<<8) | dt1
            return res

    def get_dword(self, pos=-1):
        res = 0
        if pos == -1:
            dt1 = self.get_byte()
            if dt1 == -1:
                return -1            
            dt2 = self.get_byte()
            if dt2 == -1:
                return -1
            dt3 = self.get_byte()
            if dt3 == -1:
                return -1
            dt4 = self.get_byte()
            if dt4 == -1:
                return -1
            res = (dt4<<24) | (dt3<<16) | (dt2<<8) | dt1
            return res
        else:
            dt1 = self.get_byte(pos)
            if dt1 == -1:
                return -1            
            dt2 = self.get_byte(pos+1)
            if dt2 == -1:
                return -1
            dt3 = self.get_byte(pos+3)
            if dt3 == -1:
                return -1
            dt4 = self.get_byte(pos+4)
            if dt4 == -1:
                return -1
            res = (dt4<<24) | (dt3<<16) | (dt2<<8) | dt1
            return res

    def get_variable_len(self, len, pos=-1):
        res = []
        if pos == -1:
            for i in range(len):
                dt = self.get_byte()
                if dt == -1:
                    return -1
                res.append(dt)
            return res
        else:
            for i in range(len):
                dt = self.get_byte(pos+i)
                if dt == -1:
                    return -1
                res.append(dt)
            return res


def decode_track(file):
    """
    Decode a KryoFlux RAW file (1 track)  
    Args:  
      file       : KryoFlux RAW bitstream file name
    Returns:  
      stream     : Floppy pulse interval (count in sck)
      stream_pos : Bit stream position (byte position of corresponding stream data in the stream buffer)
      index_data : Index hole data [ (stream_position0, sample_counter0, index_counter0), (), ...]
      sck        : KryoFlux sampling clock rate (Hz)
    """
    bs = KFX_stream()
    bs.open(file)

    ovf = 0
    dur = 0
    stream = []
    pos = 0
    stream_pos = []

    sck = 0
    ick = 0

    stream_position = 0
    sample_counter  = 0
    index_counter   = 0
    index_data      = []
    prev_stream_position = 0
    prev_sample_counter  = 0
    prev_index_counter   = 0

    while True:
        dt = bs.get_byte()
        if dt == -1:
            break

        stream_len = 0
        if dt >= 0x0e and dt <= 0xff:
            dur = dt + ovf
            stream_len = 1
        elif dt <= 0x07:  # FLUX2
            val = bs.get_byte()
            dur = (dt<<8) + val + ovf
            stream_len = 2
        elif dt == 0x08: # NOP1
            stream_len = 1
        elif dt == 0x09: # NOP2
            bs.get_byte()
            stream_len = 2
        elif dt == 0x0a: # NOP3
            bs.get_byte()
            bs.get_byte()
            stream_len = 3
        elif dt == 0x0b: # OVL16
            ovf += 0x10000
            stream_len = 1
        elif dt == 0x0c: # FLUX3
            dur  = bs.get_byte() << 8
            dur += bs.get_byte() + ovf
            stream_len = 3
        elif dt == 0x0d: # OOB
            dt   = bs.get_byte()
            siz  = bs.get_word()
            if dt == 0x00: # Invalid
                data = bs.get_variable_len(siz) 
            if dt == 0x01: # StreamInfo
                data = bs.get_variable_len(siz) 
            if dt == 0x02: # Index
                stream_position = bs.get_dword()
                sample_counter  = bs.get_dword()
                index_counter   = bs.get_dword()
                index_data.append((stream_position, sample_counter, index_counter))
                """
                print(index_data[-1])
                if len(index_data)>1:
                    print(index_data[-1][0]-index_data[-2][0],
                          index_data[-1][1]-index_data[-2][1],
                          index_data[-1][2]-index_data[-2][2]) 
                """
                if len(index_data)==2:
                    try:
                        spin_time = (index_data[-1][2]-index_data[-2][2]) * (1/ick)
                        rpm = (1/spin_time) * 60
                    except ZeroDivisionError:
                        spin_time = 0
                        rpm = 0
                    print(', {:.2f} RPM'.format(rpm), end='')
                prev_stream_position = stream_position
                prev_sample_counter  = sample_counter
                prev_index_counter   = index_counter
            if dt == 0x03: # StreamEnd
                data = bs.get_variable_len(siz) 
            if dt == 0x04: #KFInfo
                data = bs.get_variable_len(siz) 
                string = bytearray(data).decode('UTF-8')
                string = string.translate(str.maketrans({' ':'', '\0':'', '\n':''}))
                params = string.split(',')
                for param in params:
                    if ('sck' in param):
                        sck = eval(param.split('=')[1])
                    if ('ick' in param):
                        ick = eval(param.split('=')[1])
                if sck!=0 and ick!=0:
                    print(', SCK={:.2f}MHz, ICK={:.2f}MHz'.format(sck/1e6, ick/1e6), end='')
            if dt == 0x0d: # EOF
                data = bs.get_variable_len(siz) 
                break

        if dur != 0:
            stream.append(dur)
            stream_pos.append(pos)
            pos += stream_len
            dur = 0
            ovf = 0

    print()
    return stream, stream_pos, index_data, sck, rpm


def get_track(stream, stream_pos, index):
    result = []
    if len(index)<2:
        return []
    index1 = index[-2][0] # stream_position
    index2 = index[-1][0]
    for s, pos in zip(stream, stream_pos):
        if pos>index1 and pos<index2:
            result.append(s)
    return result


# pulse interval buffer -> bit stream
def encode(stream):
    flat_bitstream = []
    for interval in stream:
        flat_bitstream += [0] * interval
        flat_bitstream += [1]
    bitstream = bytearray()
    bit_pos = 0x80
    dt = 0
    # Condense into bytearray data
    for bit in flat_bitstream:
        if bit == 1:
            dt |= bit_pos
        bit_pos >>= 1
        if bit_pos==0:
            bitstream.append(dt)
            dt = 0
            bit_pos = 0x80
    if bit_pos != 0x80:
        bitstream.append(dt)
    return bitstream    

def append_uint64_t_le(array:bytearray, val):
    for pos in range(8):
        array.append((int(val)>>int(pos*8)) & 0x0ff)
    return array

def align(val, boundary):
    ret = ((val // boundary) + (1 if val % boundary != 0 else 0)) * boundary
    return ret

def write_mfm(bitstreams, sampling_clk, bit_rate, spindle_rpm, f):
    spindle_speed = 60/spindle_rpm   # 1 rotation / sec
    spindle_time_ns = spindle_speed * 1e9
    header = bytearray('MFM_IMG '.encode())
    track_offset_table_offset = 0x200
    append_uint64_t_le(header, track_offset_table_offset)      # track table offset
    append_uint64_t_le(header, len(bitstreams))      # number of tracks
    append_uint64_t_le(header, spindle_time_ns)   # spindle time [ns]
    append_uint64_t_le(header, 500e3)               # data bit rate
    append_uint64_t_le(header, 4e6)             # sampling rate
    f.write(header)

    track_ofst_table = bytearray()
    track_pos = align(len(header) + 16*len(bitstreams), 0x200)  # 1 record in track offset table is 16 bytes (8x2)
    for bitstream in bitstreams:
        f.seek(track_pos)
        f.write(bitstream)
        append_uint64_t_le(track_ofst_table, track_pos)
        append_uint64_t_le(track_ofst_table, len(bitstream) * 8)    # length in bit
        track_pos = align(track_pos + len(bitstream), 0x100)
    f.seek(track_offset_table_offset)
    f.write(track_ofst_table)


def main(args):
    # Get all 'raw' files and sort them
    dir_name = args.input
    files = glob.glob(dir_name+'/*.raw')
    files.sort()
    dirs = os.path.split(dir_name)
    base_dir = dirs[-1]

    fdshield_clk = args.clk_spd   # Clock speed of FD-Shield (default: 4MHz)

    out_file = base_dir + '.mfm'
    print(os.path.join(args.input, '*.raw'), '->', out_file)
    track_bitstream = []
    with open(out_file, 'wb') as f:
        count = 0
        # Process all raw files in the directory
        for file in files:
            m = [ int(s) for s in re.findall(r'track(\d+)\.(\d)\.raw', file)[0] ]
            print('track {} {}  '.format(*m), end='', flush=True)
            stream, stream_pos, index, kfx_sck, rpm = decode_track(file)     # parse KryoFlux raw stream data
            track = get_track(stream, stream_pos, index)                 # Kryoflux stream buffer may contain track data for multiple spins. Extract the data of exact 1 track
            scale = kfx_sck / fdshield_clk
            track = [ int(s/scale) for s in track ]                      # Downsample
            bitstream = encode(track)                                    # Convert pulse interval data into bit stream
            track_bitstream.append(bitstream)
            count += 1
        write_mfm(track_bitstream, fdshield_clk, 500e3, rpm, f)

if __name__ == '__main__':
    print('** "FryoFlux RAW stream data" to "fdc_bitstream MFM format" file converter (https://www.kryoflux.com/)')
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True, help='Directory name which contains input KryoFlux RAW bitstream files')
    parser.add_argument('--clk_spd', type=float, required=False, default=4e6, help='clock speed for down-sampling (default=4MHz=4000000)')
    args = parser.parse_args()
    main(args)
