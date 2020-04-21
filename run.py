#! /usr/bin/env python3

import os, subprocess, concurrent.futures

def run(cmd):
    return subprocess.run(cmd.split()).returncode == 0

def get_cpus():
    result = subprocess.run('wmic cpu get name'.split(), stdout=subprocess.PIPE)
    if result.returncode != 0:
        return ['<Unknown CPU>']
    lines = result.stdout.decode('utf-8').splitlines()
    return [line.strip() for line in lines[1:] if len(line.strip()) > 0]

def get_gpus():
    result = subprocess.run('wmic path win32_VideoController get name'.split(), stdout=subprocess.PIPE)
    if result.returncode != 0:
        return ['<Unknown GPU>']
    lines = result.stdout.decode('utf-8').splitlines()
    return [line.strip() for line in lines[1:] if len(line.strip()) > 0]

def generate_texture_set(target_dir, max_size, min_size):
    os.makedirs(target_dir, exist_ok=True)

    if len(os.listdir(target_dir)) > 0:
        return

    def run_generator(fname, size):
        if not run(f'.bin/Release/png_generator --type noise --size {size} --output {fname}'):
            exit('Failed to generate test content')

    fnames = []
    sizes = []

    size = max_size
    count = 1
    while size >= min_size:
        for i in range(count):
            fname = f'{target_dir}/{size}x{size}_{i}.png'
            fnames.append(fname)
            sizes.append(size)
        size = int(size / 2)
        count *= 4

    with concurrent.futures.ThreadPoolExecutor() as executor:
        executor.map(run_generator, fnames, sizes)

def report_clear():
    with open('report.txt', 'w'):
        pass

def report_append(msg):
    print(msg)
    with open('report.txt', 'a') as f:
        f.write(msg)
        f.write('\n')

def run_benchmark(arguments):
    report_append(arguments)
    cmd = f".bin/Release/benchmark {arguments}"
    result = subprocess.run(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode != 0:
        report_append(f'exited with error code {result.returncode}')
    else:
        errors = result.stderr.decode('utf-8').strip()
        output = result.stdout.decode('utf-8').strip()
        if len(errors) > 0:
            report_append(errors)
        report_append(output)

if not run('cmake -S . -B .build -A x64'):
    exit('Failed to configure CMake')

if not run('cmake --build .build --config Release'):
    exit('Failed to build project')

print("Generating test content...")
generate_texture_set('.content/small', 2048, 64)
generate_texture_set('.content/large', 8192, 128)

print("Running benchmark...")

report_clear()

for cpu in get_cpus():
    report_append(cpu)
for gpu in get_gpus():
    report_append(gpu)

report_append('')
report_append('========= BC1 ==================================================================')
run_benchmark('--input .content/large --format bc1 --codec compressonator --quality low')
run_benchmark('--input .content/large --format bc1 --codec compressonator --quality medium')
run_benchmark('--input .content/large --format bc1 --codec compressonator --quality high')
run_benchmark('--input .content/large --format bc1 --codec nvtt --quality low')
run_benchmark('--input .content/large --format bc1 --codec nvtt --quality medium')
run_benchmark('--input .content/large --format bc1 --codec nvtt --quality high')
run_benchmark('--input .content/large --format bc1 --codec nvtt --quality low --gpu')
run_benchmark('--input .content/large --format bc1 --codec nvtt --quality medium --gpu')
run_benchmark('--input .content/large --format bc1 --codec nvtt --quality high --gpu')
run_benchmark('--input .content/large --format bc1 --codec directxtex')

report_append('')
report_append('========= BC3 ==================================================================')
run_benchmark('--input .content/large --format bc3 --codec compressonator --quality low')
run_benchmark('--input .content/large --format bc3 --codec compressonator --quality medium')
run_benchmark('--input .content/large --format bc3 --codec compressonator --quality high')
run_benchmark('--input .content/large --format bc3 --codec nvtt --quality low')
run_benchmark('--input .content/large --format bc3 --codec nvtt --quality medium')
run_benchmark('--input .content/large --format bc3 --codec nvtt --quality high')
run_benchmark('--input .content/large --format bc3 --codec directxtex')

report_append('')
report_append('========= BC4 ==================================================================')
run_benchmark('--input .content/large --format bc4 --codec compressonator --quality low')
run_benchmark('--input .content/large --format bc4 --codec compressonator --quality medium')
run_benchmark('--input .content/large --format bc4 --codec compressonator --quality high')
run_benchmark('--input .content/large --format bc4 --codec nvtt --quality low')
run_benchmark('--input .content/large --format bc4 --codec nvtt --quality medium')
run_benchmark('--input .content/small --format bc4 --codec nvtt --quality high')
run_benchmark('--input .content/large --format bc4 --codec directxtex')

report_append('')
report_append('========= BC5 ==================================================================')
run_benchmark('--input .content/large --format bc5 --codec compressonator --quality low')
run_benchmark('--input .content/large --format bc5 --codec compressonator --quality medium')
run_benchmark('--input .content/large --format bc5 --codec compressonator --quality high')
run_benchmark('--input .content/large --format bc5 --codec nvtt --quality low')
run_benchmark('--input .content/large --format bc5 --codec nvtt --quality medium')
run_benchmark('--input .content/small --format bc5 --codec nvtt --quality high')
run_benchmark('--input .content/large --format bc5 --codec directxtex')

report_append('')
report_append('========= BC6 ==================================================================')
run_benchmark('--input .content/large --format bc6 --codec compressonator --quality low')
run_benchmark('--input .content/small --format bc6 --codec compressonator --quality medium')
run_benchmark('--input .content/small --format bc6 --codec compressonator --quality high')
run_benchmark('--input .content/large --format bc6 --codec nvtt --quality low')
run_benchmark('--input .content/large --format bc6 --codec nvtt --quality medium')
run_benchmark('--input .content/large --format bc6 --codec nvtt --quality high')
run_benchmark('--input .content/small --format bc6 --codec directxtex')
run_benchmark('--input .content/large --format bc6 --codec directxtex --gpu')

report_append('')
report_append('========= BC7 ==================================================================')
run_benchmark('--input .content/small --format bc7 --codec compressonator --quality low')
run_benchmark('--input .content/small --format bc7 --codec compressonator --quality medium')
run_benchmark('--input .content/small --format bc7 --codec compressonator --quality high')
run_benchmark('--input .content/small --format bc7 --codec nvtt --quality low')
run_benchmark('--input .content/small --format bc7 --codec nvtt --quality medium')
run_benchmark('--input .content/small --format bc7 --codec nvtt --quality high')
run_benchmark('--input .content/large --format bc7 --codec directxtex --bc7quick')
run_benchmark('--input .content/small --format bc7 --codec directxtex')
run_benchmark('--input .content/small --format bc7 --codec directxtex --bc7use3subsets')
run_benchmark('--input .content/large --format bc7 --codec directxtex --gpu --bc7quick')
run_benchmark('--input .content/large --format bc7 --codec directxtex --gpu')
run_benchmark('--input .content/large --format bc7 --codec directxtex --gpu --bc7use3subsets')
