#!/usr/bin/env ruby

require 'pp'
require 'bindata'

require_relative 'modes'

class MinixSuperBlock < BinData::Record
    endian :little
    uint16 :s_ninodes
    uint16 :s_nzones
    uint16 :s_imap_blocks
    uint16 :s_zmap_blocks
    uint16 :s_firstdatazone
    uint16 :s_log_zone_size
    uint32 :s_max_size
    uint16 :s_magic
    uint16 :s_state
    uint32 :s_zones
end

class MinixInode < BinData::Record
    endian :little
    uint16 :i_mode
    uint16 :i_uid
    uint32 :i_size
    uint32 :i_time
    uint8  :i_gid
    uint8  :i_nlinks
    array  :i_zone, :type => :uint16, :initial_length => 9
end

class Minix2Inode < BinData::Record
    endian :little
    uint16 :i_mode
    uint16 :i_nlinks
    uint16 :i_uid
    uint16 :i_gid
    uint32 :i_size
    uint32 :i_atime
    uint32 :i_mtime
    uint32 :i_ctime
    array  :i_zone, :type => :uint32, :initial_length => 10
end

class MinixDentry < BinData::Record
    endian :little
    uint16 :inode
    array  :name, :type => :uint8, :initial_length => 14
end

def dentry_name(dentry)
    name = dentry.name.map{ |x| x.chr }.join
    return name.delete("\x00")
end

disk = File.open('/root/disk.raw', 'r')
disk.seek(0x400)
sb = MinixSuperBlock.read(disk)
#pp sb

def find_inode_offset(inode_num)
    base = 0x800 + 3 * 0x400 + 8 * 0x400
    return base + (inode_num-1)*32
end

disk.seek(find_inode_offset(1))
root_inode = MinixInode.read(disk)

def read_dir(disk, inode_num)
    disk.seek(find_inode_offset(inode_num))
    inode = MinixInode.read(disk)
    dentries = []
    for zone in inode.i_zone do
        disk.seek(zone * 1024)

        bytes_read = 0
        loop do
            dentry = MinixDentry.read(disk)
            break if bytes_read >= 1024 or dentry.inode == 0
            dentries << dentry
            bytes_read += 16
        end
    end
    return dentries
end

def find_inode_num(disk, path)
    path = path.split('/').reject(&:empty?)

    inode_num = 1
    while path != [] do
        found = 0
        path_part = path.shift
        dentries = read_dir(disk, inode_num)
        for dentry in dentries do
            if path_part == dentry_name(dentry) then
                found = 1
                inode_num = dentry.inode
            end
        end
        if found == 0
            puts 'not found'
            return
        end
    end
    return inode_num
end

def ls(disk, path)
    inode_num = find_inode_num(disk, path)
    disk.seek(find_inode_offset(inode_num))
    inode = MinixInode.read(disk)
    if not is_dir(inode.i_mode) then
        puts "#{mode_string(inode.i_mode)} #{inode.i_uid} #{inode.i_size} #{path}"
    else
        dentries = read_dir(disk, inode_num)
        for dentry in dentries do
            inode_num = dentry.inode
            disk.seek(find_inode_offset(inode_num))
            inode = MinixInode.read(disk)
            puts "#{mode_string(inode.i_mode)} #{inode.i_uid} #{inode.i_size} #{dentry_name(dentry)}"
        end
    end
end

#ls(disk, '/vtrgb')
ls(disk, '/apparmor.d/abstractions')
