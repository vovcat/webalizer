execve("/usr/bin/webalizer", ["webalizer", "--help"], [/* 27 vars */]) = 0
uname({sys="Linux", node="werkbak", ...}) = 0
brk(0)                                  = 0x807dc84
open("/etc/ld.so.preload", O_RDONLY)    = -1 ENOENT (No such file or directory)
open("/etc/ld.so.cache", O_RDONLY)      = 3
fstat64(3, {st_mode=S_IFREG|0644, st_size=68359, ...}) = 0
old_mmap(NULL, 68359, PROT_READ, MAP_PRIVATE, 3, 0) = 0x40012000
close(3)                                = 0
open("/usr/lib/libgd.so.2", O_RDONLY)   = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0\330@\0"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=200976, ...}) = 0
old_mmap(NULL, 220644, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x40023000
mprotect(0x40036000, 142820, PROT_NONE) = 0
old_mmap(0x40036000, 126976, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x12000) = 0x40036000
old_mmap(0x40055000, 15844, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x40055000
close(3)                                = 0
open("/usr/lib/libpng12.so.0", O_RDONLY) = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0([\0\000"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=134384, ...}) = 0
old_mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x40059000
old_mmap(NULL, 133292, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x4005a000
mprotect(0x4007a000, 2220, PROT_NONE)   = 0
old_mmap(0x4007a000, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x20000) = 0x4007a000
close(3)                                = 0
open("/usr/lib/libz.so.1", O_RDONLY)    = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0D\30\0\000"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=51352, ...}) = 0
old_mmap(NULL, 50316, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x4007b000
mprotect(0x40086000, 5260, PROT_NONE)   = 0
old_mmap(0x40086000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0xb000) = 0x40086000
close(3)                                = 0
open("/lib/libm.so.6", O_RDONLY)        = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0\2605\0"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=131156, ...}) = 0
old_mmap(NULL, 133712, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x40088000
mprotect(0x400a8000, 2640, PROT_NONE)   = 0
old_mmap(0x400a8000, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x1f000) = 0x400a8000
close(3)                                = 0
open("/lib/libnsl.so.1", O_RDONLY)      = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0d;\0\000"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=69132, ...}) = 0
old_mmap(NULL, 76448, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x400a9000
mprotect(0x400b9000, 10912, PROT_NONE)  = 0
old_mmap(0x400b9000, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x10000) = 0x400b9000
old_mmap(0x400ba000, 6816, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x400ba000
close(3)                                = 0
open("/usr/lib/libdb3.so.3", O_RDONLY)  = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0h\311\0"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=642472, ...}) = 0
old_mmap(NULL, 641964, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x400bc000
mprotect(0x40158000, 2988, PROT_NONE)   = 0
old_mmap(0x40158000, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x9c000) = 0x40158000
close(3)                                = 0
open("/lib/libc.so.6", O_RDONLY)        = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0\275Z\1"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0755, st_size=1103880, ...}) = 0
old_mmap(NULL, 1113636, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x40159000
mprotect(0x40261000, 32292, PROT_NONE)  = 0
old_mmap(0x40261000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x107000) = 0x40261000
old_mmap(0x40267000, 7716, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x40267000
close(3)                                = 0
open("/usr/lib/libjpeg.so.62", O_RDONLY) = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0\270$\0"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=113400, ...}) = 0
old_mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x40269000
old_mmap(NULL, 116460, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x4026a000
mprotect(0x40286000, 1772, PROT_NONE)   = 0
old_mmap(0x40286000, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x1b000) = 0x40286000
close(3)                                = 0
open("/usr/lib/libfreetype.so.6", O_RDONLY) = 3
read(3, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0\3\0\3\0\1\0\0\0\244|\0"..., 1024) = 1024
fstat64(3, {st_mode=S_IFREG|0644, st_size=316344, ...}) = 0
old_mmap(NULL, 319408, PROT_READ|PROT_EXEC, MAP_PRIVATE, 3, 0) = 0x40287000
mprotect(0x402d1000, 16304, PROT_NONE)  = 0
old_mmap(0x402d1000, 16384, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED, 3, 0x49000) = 0x402d1000
close(3)                                = 0
munmap(0x40012000, 68359)               = 0
open("/usr/lib/locale/locale-archive", O_RDONLY|O_LARGEFILE) = 3
fstat64(3, {st_mode=S_IFREG|0644, st_size=290416, ...}) = 0
mmap2(NULL, 290416, PROT_READ, MAP_PRIVATE, 3, 0) = 0x402d5000
close(3)                                = 0
brk(0)                                  = 0x807dc84
brk(0x807ec84)                          = 0x807ec84
brk(0)                                  = 0x807ec84
brk(0x807f000)                          = 0x807f000
open("/usr/share/locale/locale.alias", O_RDONLY) = 3
fstat64(3, {st_mode=S_IFREG|0644, st_size=2597, ...}) = 0
old_mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x40012000
read(3, "# Locale name alias data base.\n#"..., 4096) = 2597
read(3, "", 4096)                       = 0
close(3)                                = 0
munmap(0x40012000, 4096)                = 0
open("/usr/lib/locale/en_US/LC_IDENTIFICATION", O_RDONLY) = -1 ENOENT (No such file or directory)
open("/usr/lib/locale/en/LC_IDENTIFICATION", O_RDONLY) = -1 ENOENT (No such file or directory)
access("webalizer.conf", F_OK)          = -1 ENOENT (No such file or directory)
access("/etc/webalizer.conf", F_OK)     = 0
open("/etc/webalizer.conf", O_RDONLY|O_LARGEFILE) = 3
fstat64(3, {st_mode=S_IFREG|0644, st_size=13866, ...}) = 0
old_mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x40012000
read(3, "# Sample Webalizer configuration"..., 4096) = 4096
read(3, " for\n\n# HostName defines the hos"..., 4096) = 4096
read(3, ".  (Note: warning and error mess"..., 4096) = 4096
read(3, "I hessitated in adding these,\n# "..., 4096) = 1578
read(3, "", 4096)                       = 0
close(3)                                = 0
munmap(0x40012000, 4096)                = 0
fstat64(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(136, 2), ...}) = 0
old_mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x40012000
write(1, "Usage: webalizer [options] [log "..., 38) = 38
write(1, "-h        = print this help mess"..., 36) = 36
write(1, "-v -V     = print version inform"..., 38) = 38
write(1, "-d        = print additional deb"..., 40) = 40
write(1, "-F type   = Log type.  type= (cl"..., 49) = 49
write(1, "-f        = Fold sequence errors"..., 33) = 33
write(1, "-i        = ignore history file\n", 32) = 32
write(1, "-p        = preserve state (incr"..., 41) = 41
write(1, "-q        = supress informationa"..., 43) = 43
write(1, "-Q        = supress _ALL_ messag"..., 35) = 35
write(1, "-Y        = supress country grap"..., 34) = 34
write(1, "-G        = supress hourly graph"..., 33) = 33
write(1, "-H        = supress hourly stats"..., 33) = 33
write(1, "-L        = supress color coded "..., 46) = 46
write(1, "-l num    = use num background l"..., 46) = 46
write(1, "-m num    = Visit timout value ("..., 41) = 41
write(1, "-T        = print timing informa"..., 37) = 37
write(1, "-c file   = use configuration fi"..., 42) = 42
write(1, "-n name   = hostname to use\n", 28) = 28
write(1, "-o dir    = output directory to "..., 36) = 36
write(1, "-t name   = report title \'name\'\n", 32) = 32
write(1, "-a name   = hide user agent \'nam"..., 35) = 35
write(1, "-r name   = hide referrer \'name\'"..., 33) = 33
write(1, "-s name   = hide site \'name\'\n", 29) = 29
write(1, "-u name   = hide URL \'name\'\n", 28) = 28
write(1, "-x name   = Use filename extensi"..., 42) = 42
write(1, "-P name   = Page type extension "..., 39) = 39
write(1, "-I name   = Index alias \'name\'\n", 31) = 31
write(1, "-A num    = Display num top agen"..., 35) = 35
write(1, "-C num    = Display num top coun"..., 38) = 38
write(1, "-R num    = Display num top refe"..., 38) = 38
write(1, "-S num    = Display num top site"..., 34) = 34
write(1, "-U num    = Display num top URLs"..., 33) = 33
write(1, "-e num    = Display num top Entr"..., 40) = 40
write(1, "-E num    = Display num top Exit"..., 39) = 39
write(1, "-g num    = Group Domains to \'nu"..., 42) = 42
write(1, "-X        = Hide individual site"..., 34) = 34
write(1, "-D name   = Use DNS Cache file \'"..., 38) = 38
write(1, "-N num    = Number of DNS proces"..., 48) = 48
munmap(0x40012000, 4096)                = 0
exit_group(1)                           = ?
