import os
import sys


def count_lines(filename):
    try:
        with open(filename, 'r') as file:
            lines = file.readlines()
            line_count = len(lines)
        return line_count
    except FileNotFoundError:
        print(f"File '{filename}' not found.")
        return 0


def get_filenames_recursively(directory, ignore_directories):
    file_list = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            ignore_this_file = False
            for ignore_dir in ignore_directories:
                if ignore_dir in file_path:
                    ignore_this_file = True
                    break
        
            if file_path.endswith(('.cpp', '.hpp')) or file_path.endswith(('.c', '.h')):
                if not ignore_this_file:
                    file_list.append(os.path.join(root, file))
    return file_list


def right_pad_string(string, length, pad_char):
    padded_string = string.ljust(length, pad_char)
    return padded_string


args = sys.argv
arg1 = args[0]
if len(args) > 1:
    arg1 = args[1]

directory = './'  # Replace with the actual directory path

ignore_dirs = ['Engine\\stb', 'Engine\\UI\\imgui']


if arg1 == '-f':
    ignore_dirs = []

filenames = get_filenames_recursively(directory, ignore_dirs)
total_count = 0
for filename in filenames:
    count = count_lines(filename)
    print(right_pad_string(filename, 50, '_'), count)
    total_count += count
print(right_pad_string('', 55, '#'))
print(right_pad_string("Total lines:", 50, '_'), total_count)
