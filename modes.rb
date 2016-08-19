S_IFMT = 0170000
S_ARCH2 = 0400000
S_ARCH1 = 0200000
S_IFWHT = 0160000
S_IFSOCK = 0140000
S_ISVTX = 01000
S_IFLNK = 0120000
S_IFREG = 0100000
S_IFBLK = 060000
S_IFDIR = 040000
S_IFCHR = 020000
S_IFIFO = 010000

S_ISUID = 04000
S_ISGID = 02000
S_ISTXT = 01000

S_IRWXU = 0700
S_IRUSR = 0400
S_IWUSR = 0200
S_IXUSR = 0100

S_IRWXG = 070
S_IRGRP = 040
S_IWGRP = 020
S_IXGRP = 010

S_IRWXO = 07
S_IROTH = 04
S_IWOTH = 02
S_IXOTH = 01

def is_lnk(mode) return ((mode & S_IFLNK) == S_IFLNK) end
def is_dir(mode) return ((mode & S_IFDIR) == S_IFDIR) end

def is_suid(mode) return ((mode & S_ISUID) == S_ISUID) end
def is_sgid(mode) return ((mode & S_ISGID) == S_ISGID) end
def is_stxt(mode) return ((mode & S_ISTXT) == S_ISTXT) end

def is_rusr(mode) return ((mode & S_IRUSR) == S_IRUSR) end
def is_wusr(mode) return ((mode & S_IWUSR) == S_IWUSR) end
def is_xusr(mode) return ((mode & S_IXUSR) == S_IXUSR) end

def is_rgrp(mode) return ((mode & S_IRGRP) == S_IRGRP) end
def is_wgrp(mode) return ((mode & S_IWGRP) == S_IWGRP) end
def is_xgrp(mode) return ((mode & S_IXGRP) == S_IXGRP) end

def is_roth(mode) return ((mode & S_IROTH) == S_IROTH) end
def is_woth(mode) return ((mode & S_IWOTH) == S_IWOTH) end
def is_xoth(mode) return ((mode & S_IXOTH) == S_IXOTH) end

def mode_string(mode)
    mode_string = '----------'

    if is_rusr(mode) then mode_string[1] = 'r' end
    if is_wusr(mode) then mode_string[2] = 'w' end
    if is_xusr(mode) then mode_string[3] = 'x' end

    if is_rgrp(mode) then mode_string[4] = 'r' end
    if is_wgrp(mode) then mode_string[5] = 'w' end
    if is_xgrp(mode) then mode_string[6] = 'x' end

    if is_roth(mode) then mode_string[7] = 'r' end
    if is_woth(mode) then mode_string[8] = 'w' end
    if is_xoth(mode) then mode_string[9] = 'x' end

    if is_suid(mode) then mode_string[3] = 's' end
    if is_sgid(mode) then mode_string[6] = 's' end
    if is_stxt(mode) then mode_string[9] = 't' end

    if is_lnk(mode) then mode_string[0] = 'l' end
    if is_dir(mode) then mode_string[0] = 'd' end

    return mode_string
end
