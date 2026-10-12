"""Compile the small starter check with the current CMake audit flags/libraries."""
import pathlib
import shlex
import subprocess
import sys

repo, build, output = [pathlib.Path(p).resolve(strict=True) for p in sys.argv[1:]]
target = build / 'CMakeFiles/openemperor-audit-original-occupancy.dir'
flags = {}
for line in (target / 'flags.make').read_text().splitlines():
    for key in ('CXX_DEFINES', 'CXX_INCLUDES', 'CXX_FLAGS'):
        if line.startswith(key + ' = '):
            flags[key] = shlex.split(line.split(' = ', 1)[1])
link = shlex.split((target / 'link.txt').read_text())
obj = output / 'empty-map-preflight.o'
subprocess.run([link[0], *flags['CXX_DEFINES'], *flags['CXX_INCLUDES'],
                *flags['CXX_FLAGS'], '-c', str(repo / 'tools/CheckEmptyOriginalMap.cpp'),
                '-o', str(obj)], cwd=build, check=True)
original = 'CMakeFiles/openemperor-audit-original-occupancy.dir/tools/AuditOriginalOccupancy.cpp.o'
if link.count(original) != 1 or link.count('-o') != 1:
    raise SystemExit('Unexpected current CMake audit link; no app launched.')
link[link.index(original)] = str(obj)
link[link.index('-o') + 1] = str(output / 'empty-map-preflight')
subprocess.run(link, cwd=build, check=True)
