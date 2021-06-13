#!/usr/bin/python

import sys

def print_format_error():
    print("Unsupported file format!")
    print("Supported formats are: .ele")
    sys.exit(0)


if len(sys.argv) < 2:
    print("Usage: ./convert2off.py <filename>")
    sys.exit(0)

filename = sys.argv[1]
parts = filename.split(".")

if len(parts) < 2:
    print_format_error()

format = parts[-1]
name = filename[0 : -len(format) - 1]

if format not in ["ele"]:
    print_format_error()

in_file = open(filename, "r")
out_file = open(name + ".off", "w")

if format == "ele":
    num_triangles, nodes_per_tri, att = in_file.readline().split()

    if int(nodes_per_tri) != 3:
        print("Invalid number of nodes per triangle!")
        sys.exit(0)

    node_file = open(name + ".node", "r")
    num_vertices, dim, att, boundary = node_file.readline().split()

    out_file.write("OFF\n")
    out_file.write(num_vertices + " " + num_triangles + " 0\n")

    start_index = 1

    for line in node_file:
        if len(line) == 0 or line.startswith("#"):
            continue

        vertex = line.split()
        out_file.write(vertex[1] + " " + vertex[2] + " 0\n")
        if vertex[0] == "0":
            start_index = 0

    for line in in_file:
        if len(line) == 0 or line.startswith("#"):
            continue

        tri = line.split()
        tri[1] = str(int(tri[1]) - start_index)
        tri[2] = str(int(tri[2]) - start_index)
        tri[3] = str(int(tri[3]) - start_index)
        out_file.write("3 " + tri[1] + " " + tri[2] + " " + tri[3] + "\n")

    node_file.close()

out_file.close()
in_file.close()
