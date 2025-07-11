#!/usr/bin/env python3
import os
import argparse
import json
import re
from subprocess import check_call as call
from rom_diff import create_diff
from ntype import BigStream
from crc import calculate_crc

from Rom import *
from Patches import patch_rom

parser = argparse.ArgumentParser()
parser.add_argument('--pj64sym', help="Output path for PJ64 debugging symbols")
parser.add_argument('--compile-c', action='store_true', help="Recompile C modules")
parser.add_argument('--dump-obj', action='store_true', help="Dumps extra object info for debugging purposes. Does nothing without --compile-c")
parser.add_argument('--diff-only', action='store_true', help="Creates diff output without running armips")
parser.add_argument('--mq', action='store_true', help="Patches Beta Quest with Master Quest dungeons")
parser.add_argument('--bonko', action='store_true', help="Patches Beta Quest with Bonk One Hit KO")
parser.add_argument('--song_speedup', action='store_true', help="Patches Beta Quest with Song Speedups")

args = parser.parse_args()
pj64_sym_path = args.pj64sym
compile_c = args.compile_c
dump_obj = args.dump_obj
diff_only = args.diff_only
mq_enabled = args.mq
bonko = args.bonko
song_speedup = args.song_speedup

scripts_dir = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.join(scripts_dir, '..')
compress_dir = os.path.join(root_dir, 'Compress')
os.environ['PATH'] = scripts_dir + os.pathsep + os.environ['PATH']
os.environ['PATH'] = compress_dir + os.pathsep + os.environ['PATH']

run_dir = root_dir

# Compile code

os.chdir(run_dir)
if compile_c:
    clist = ['make']
    if dump_obj:
        clist.append('RUN_OBJDUMP=1')
    call(clist)

if not diff_only:
    os.chdir(run_dir + '/src')
    call(['armips', '-sym2', '../build/asm_symbols.txt', 'build.asm'])

os.chdir(run_dir)

with open('build/asm_symbols.txt', 'rb') as f:
    asm_symbols_content = f.read()
asm_symbols_content = asm_symbols_content.replace(b'\r\n', b'\n')
asm_symbols_content = asm_symbols_content.replace(b'\x1A', b'')
with open('build/asm_symbols.txt', 'wb') as f:
    f.write(asm_symbols_content)

# Parse symbols

c_sym_types = {}

with open('build/c_symbols.txt', 'r') as f:
    for line in f:
        m = re.match('''
                ^
                [0-9a-fA-F]+
                .*
                \.
                ([^\s]+)
                \s+
                [0-9a-fA-F]+
                \s+
                ([^.$][^\s]+)
                \s+$
            ''', line, re.VERBOSE)
        if m:
            sym_type = m.group(1)
            name = m.group(2)
            c_sym_types[name] = 'code' if sym_type == 'text' else 'data'

symbols = {}

with open('build/asm_symbols.txt', 'r') as f:
    for line in f:
        parts = line.strip().split(' ')
        if len(parts) < 2:
            continue
        address, sym_name = parts
        if address[0] != '8':
            continue
        if sym_name[0] in ['.', '@']:
            continue
        sym_type = c_sym_types.get(sym_name) or ('data' if sym_name.isupper() else 'code')
        symbols[sym_name] = {
            'type': sym_type,
            'address': address,
        }

# Output symbols

os.chdir(run_dir)

data_symbols = {}
for (name, sym) in symbols.items():
    if sym['type'] == 'data':
        addr = int(sym['address'], 16)
        if 0x80400000 <= addr < 0x80410000:
            addr = addr - 0x80400000 + 0x03490000
        else:
            continue
        data_symbols[name] = '{0:08X}'.format(addr)
if (not os.path.exists('data')):
    os.mkdir('data')
with open('data/symbols.json', 'w') as f:
    json.dump(data_symbols, f, indent=4, sort_keys=True)

if pj64_sym_path:
    pj64_sym_path = os.path.realpath(pj64_sym_path)
    with open(pj64_sym_path, 'w') as f:
        key = lambda pair: pair[1]['address']
        for sym_name, sym in sorted(symbols.items(), key=key):
            f.write('{0},{1},{2}\n'.format(sym['address'], sym['type'], sym_name))

# Apply python patches
rom = Rom('roms/port.z64')
patch_rom(rom, { 'mq_enabled': mq_enabled, 'bonko': bonko, 'song_speedup': song_speedup })
rom.write_to_file('roms/port.z64')

with open('roms/port.z64', 'r+b') as stream:
    buffer = bytearray(stream.read(0x101000))
    crc = calculate_crc(BigStream(buffer))
    stream.seek(0x10)
    stream.write(bytearray(crc))

# Recompress
os.chdir(run_dir + '/Compress')
call(['Compress', run_dir + '/roms/port.z64'])

# Diff ROMs
os.chdir(run_dir)
create_diff('roms/base.z64', 'roms/port.z64', 'data/rom_patch.txt')
