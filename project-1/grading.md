# Bootblock
- Is ds=0, what we want? What is the DS-register  used for?
- Where is data  for kernel suppose to go?

OK!

# Createimage
- Missing implementation for extended option
- pheader memsize  and filesize always the same? Buffer allocated  using memsize, filesize used to read into that buffer
- There is a lot of hardcoded "magic" numbers, the program seem to lack possibility for handling multiple files(even though it iterates through 'n'-files). A change in the size of the kernel code should be enough to cause truobles here..

# Pass