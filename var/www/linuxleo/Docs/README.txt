README.TXT
April 2020

The Law Enforcement and Forensic Examiner's Introduction to
Linux, A Comprehensive Beginner's Guide to Linux as a Digital
Forensic Platform.

This guide is targeted at computer forensic investigators
interested in learning more about the GNU/Linux operating system.
It assumes no prior experience with Linux. 

This guide does not aim to be a "how-to" for conducting forensic
examinations. It is designed to introduce the tools available for
investigators using Linux. The tools are introduced through a
series of practical exercises, allowing for a "hands on"
approach. Many new Linux users will install the OS and then
wonder what to do next. This guide was written to address that by
providing a guided path that allows investigators to focus their
Linux learning experience on what is of interest to them - Linux
as a forensic tool.

The guide has been updated to recent versions of Linux
(specifically Slackware). And has several more hands on exercises
than the previous versions. Many of these were added as a result
of experiences in classrooms - where the same questions kept
popping up. More and more exercises are added as requests are
made for more info and hands on stuff. Future versions of the
guide will have a comprehensive approach, with more realistic
exercises targeted and a complete examination.

The following files are part of the Introduction to Linux for Law
Enforcement and Forensic Examiners:

1)  linuxintro-LEFE-4.XX.pdf:  The guide itself in PDF format.

2)  fat_fs.raw:  A dd image of a FAT file system for the initial 
exercises. Disk created with a Win9x system. 

3)  image_carve_2017.raw:  A "raw" chunk of data used in a dd
carving exercise.

4)  logs.v3.tar.gz:  A gzip compressed tar archive containing a
set of messages logs from a Unix system for use with a command
line exercise for data parsing and organization.

5)  able2.tar.gz:  A gzip compressed tar archive containing a
forensic (dd) image of a 330Mb Linux system with an ext2 file
system.

6) able_3.tar.gz: A gzip compressed tar archive containing a
forensic image of a 512Mb Linux system with an ext4 file system.
The purpose of this image is to highlight the analytical
difference between examination of ext2 and ext4 file systems, and
the behavior of certain tools.

7) NTFS_Pract_2017_E01.tar.gz:  A gzip compressed image of an
NTFS file system for use in forensic exercises.

8) gtpimage.raw.gz:  A simple disk image with a GPT partition
layout for illustration purposes.

9) sha256.txt:  sha256 hash values for all the files.

Any questions, comments or critique are welcome. 

Barry J. Grundy
bgrundy@LinuxLEO.com
