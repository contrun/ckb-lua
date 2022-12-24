To faciliate the sharing of lua modules, we created a file system called Simple Lua File System that can be mounted by ckb-lua.
Files within the file system may be made available for lua scripts to read and execute,
e.g. running `require('mymodule')` and `io.open('myfile')`.
The file system can be stored in any cell whose data are reachable from lua scripts,
and lua scripts may mount multiple file systems from multiple cells.
A file system is represented as a binary file in the format described below.
We may use the script [fs.lua](../utils/fs.lua) to create a file system from given files, or unpacking the file system into files.

# Mounting the File System

Calling `ckb.mount(source, index)` will mount the file system from the data of the cell with the specified `source` and `index`.
For example, if you want to mount the fs within the first output cell, you can run `ckb.mount(ckb.SOURCE_OUTPUT, 0)`.
The return value of `ckb.mount` will be nil unless some error happened. In that case a integer to represent the error will be returned.
Afterwards, you can read and execute files contained in this file system.
You may call `ckb.mount` multiple times to mount several file systems.
Note that files from later mounts may override files from earlier mounts, i.e. if a file called `a.txt` is contained in two file systems.
The file `a.txt` from a later mount will be preferred over that of a earlier mount when reading.

# Create a File System

To pack all lua files within current directory into `$packed_file`, you may run
`find . -name '*lua' -type f | lua "./utils/fs.lua" pack "$packed_file"`.
Note that all file paths piped into the lua script must be in the relative path format. The absolute path of a file
in current system is usually meaningless in the Simple Lua File System.
You may also run `lua "./utils/fs.lua" pack "$packed_file" *.lua`. Although, the utility of the later command is limited due to the limit of
number of command line arguments in your OS.

# Unpack File System to Files

To unpack the files contained within a fs, you may run
`lua "./utils/fs.lua" unpack "$packed_file" "$directory"`,
where `$packed_file` is the file that contains the file system, `$directory` is the directory to put all the unpacked files.


# Simple Lua File System On-disk Representation

The on-disk represention of a Simple Lua File System consists of three parts,
a number to represent the number of files contained in this file system, an array of metadata to store file metadata
and a array of binary objects (also called blob) to store the acutal file contents.

A metadadum is simply an offset from the start of the
blob array and a datum length. Each file name and file content has a metadatum.
For each file stored in the fs, there is four `uint32_t` number in the metadata, i.e. the offset of the file name in the
blob array, the length of the file name, the offset of the file content in the blob array and the length of the file content.

We represent the above structures using c struct-like syntax as follows.
```c
struct Blob {
	uint32_t offset;
	uint32_t length;
}

struct Metadata {
	struct Blob file_name;
	struct Blob file_content;
}

struct SimpleFileSystem {
	uint32_t file_count;
	struct Metadata metadata[..];
	uint8_t payload[..];
}
```

When serializing the file system into a file, all intergers are encoded as an 32-bit little endian number.
The file names are stored as null-terminated strings.

Below is a binary dump of the file system created from a simple file called `main.lua` with content `print('hello world!')`.

```
00000000: 0100 0000 0000 0000 0900 0000 0900 0000  ................
00000010: 1500 0000 6d61 696e 2e6c 7561 0070 7269  ....main.lua.pri
00000020: 6e74 2827 6865 6c6c 6f20 776f 726c 6421  nt('hello world!
00000030: 2729                                     ')
```

