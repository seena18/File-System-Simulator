Seena Abed, Aladdin Ismael

Our TinyFS works decently well. We are able to mount and unmount file systems, open files, write and read, seek, and close files.  
We chose the linked list approach for our inodes and for our free blocks. Using a singly linked list means its less efficient
to adjust the the previous block in a list

We chose to add the abilities to rename files, list the files in the file system, and to have timestamps for each file.

We are  facing corruption issues when we try to free in some 
areas so there are some memory leaks in our program as we did't have time to pinpoint the source of the corruption.
If we spam the demo executable, we also find the corruption leaks into our disk file.