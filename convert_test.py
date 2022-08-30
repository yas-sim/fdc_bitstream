import glob
import os
import sys
import subprocess

bin_dir = '../bin/Release'
cmd_name = 'image_converter'

raw_files = glob.glob(sys.argv[1]+'//**/*.raw', recursive=True)

total_good = 0
total_bad = 0

for raw_file in raw_files:
    cmd = os.path.join(bin_dir, cmd_name)
    cmd += ' -i {} -o dmy.d77 -v'.format(raw_file)
    #print(cmd)
    print(raw_file, ' ', end='', flush=True)
    log = subprocess.run(cmd, capture_output=True, text=True).stdout
    for line in log.splitlines():
        if(line[:2] == '**'):
            msg, data = line.split(':')
            good, bad = data.split(' ')
            print(good, bad, int(bad)/(int(good)+int(bad)))
            total_good += int(good)
            total_bad  += int(bad)

print('total result - good/bad/error rate:', total_good, total_bad, total_good/(total_good+total_bad))
