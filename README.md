# ws21
Decompilations / ports of Whitesmiths 2.1 tools

The tools

**disrel** - this is a tool I wrote around 2007 that disassembles Whitesmiths' 8080 object files. I have recently updated to make sure it compiles cleanly for x64 builds. There was a corresponding tool to extract object files from archives, but since the port of lib this is no longer needed
**Usage**:
disrel [-8] file
where -8  will generate a-natural format files vs. standard asm files and the file is the file to disassemble

**anat** - this is my decompilation and porting of the Whitesmiths' anat tool to compile under 32/64 bit. The work was originally done in 2008 with recent changes to make sure it compiles cleanly for x64 builds.

**as80, lib, link** - these are my decompilations from June 2020 of the corresponding Whitesmiths' 2.1 tools and ports to make them compile under 32/64 bit.

Note I fixed a couple of bugs in the originals files, namely

- as80 - in define the original code had a variable declared as char rather than char *
- link - the processing of v7 libraries incorrectly read too few bytes from the header

Other than the fixes most of the code is as close to the original decompilation as I could make it, the key changes are as follows

1. File I/O is now done using the C standard FILE * functions. The originals had a mix of raw I/O and Whitesmiths' own buffered file I/O routines
2. Simple modifications to use standard C libraries e.g. lenstr -> strlen, cpybuf -> memcpy.
3. A port of the Whitesmiths' getflags function. This required major modifications as the original assumed all parameters were on the stack and sizeof(int) == sizeof(pointerType).  This necessitated some changes to the callers of getflags as the option to return a list of items also made the same assumption
4. Ports of the getfiles and getbfiles, modified to use FILE * rather than int file descriptors
5. Implementation of a small number of Whitesmiths' functions, modified to use standard C.
6. Adding dummy arguments where the function  call did not match the function prototype.
7. Changes to as80 to use intptr_t as the original assumed sizeof(int) == sizeof(pointerType). Appropriate typecasts were added where the variables were used.

Whilst I have done some testing of the code, e.g. library add/delete, checking link by building the whitesmiths' executables and assembling echo.s, there are likely to be some hidden bugs. If you identify one, please let me  know and I will look to fix it.

Mark Ogden

20-Jun-2020