//Provide type definitions (like int, char)
#include "kernel/types.h"

//Define the structure stat, which stores metadata about files
#include "kernel/stat.h"

//Provide system calls like open(), read().....
#include "user/user.h"

//DIRSIZ and struct dirent
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  //Declears a static character buffer to store the file name
  static char buf[DIRSIZ+1];

  //Declears a pointer which will point to character in the path string
  char *p;

  // Find first character after last slash.
  // p=path+strlen(path), p will point to the null terminator at the end
  // If path = "usr/bin/ls", it will stop at the last '/'
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  //Then move one character forward, making p point to 'l'
  p++;

  // Return blank-padded name.
  // strlen(p) -> strlen(p) returns the length of the null-terminated string starting from the character p points to.
  if(strlen(p) >= DIRSIZ)
    return p;

  //Copies the extracted name from p to buf
  memmove(buf, p, strlen(p));
  //Fill the remaining space in buf with sapce to align it to DIRSIZ
  memset(buf+strlen(p), 0, DIRSIZ-strlen(p));
  return buf;
}

int noRecurse(char* path){
    char *buf = fmtname(path);
    if(buf[0] == '.' && buf[1] == 0){
        return 1;
    }

    if(buf[0] == '.' && buf[1] == '.' && buf[2] == 0){
        return 1;
    }
    return 0;
}


void
find(char *path, char *target)
{
  char buf[512], *p;
  int fd;

  //Directory entry
  struct dirent de;

  //Metadata about a file
  struct stat st;

  //Open the file or directory specified by path in read-only (0) mode
  if((fd = open(path, 0)) < 0){

    //2 -> Standard Error
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  //Gets the metadata of the file associated with the file descriptor fd
  //store the metadata into pointer -> st
  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

    //if string path == string target, strcmp(path, target) will return 0
    if(strcmp(fmtname(path), target) == 0){
        printf("%s\n",path);
    }

  switch(st.type){
    //Regular file
  case T_FILE:
    break;

    //Represents a directory
  case T_DIR:
    //Ensures that the path length doesn’t exceed the buffer size.
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }

    //Copies the current path into buf.
    strcpy(buf, path);

    //Move the pointer p to end of the string
    p = buf+strlen(buf);

    //*p = '/'
    //p = p + 1
    //homeuser -> homeuser/
    *p++ = '/';

    //Reads directory entries one by one into de
    while(read(fd, &de, sizeof(de)) == sizeof(de)){

      //ship entries with an inode number of 0 (empty or unused)
      if(de.inum == 0)
        continue;
      
      //Copies the directory entry name to buf
      memmove(p, de.name, DIRSIZ);

      //Ensure the string is null-terminated
      p[DIRSIZ] = 0;

      //Fetch the metadata for each entry using 
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }

        if(noRecurse(buf) == 0){
            find(buf, target);
        }

    }
    break;
  }
  close(fd);
}


int main(int argc, char *argv[]){
    if(argc == 1){
        printf("usage: find [path] [target]\n");
        exit(0);
    }
    
    //current path
    if(argc == 2){

        //'.' -> current path; argv[1] -> target;
        find(".", argv[1]);
        exit(0);
    }
    
    //specific path
    if(argc == 3){
        //argv[1] -> path; argv[2] -> target;
        find(argv[1], argv[2]);
    }
    exit(0);
}

// Assume you have the following directory structure:
// .
// ├── file1.txt
// ├── console
// ├── dir1
// │   ├── file2.txt
// │   └── console
// └── dir2
//     ├── file3.txt
//     └── subdir
//         └── console

// $ find . console

// Step 1: Initial Call to find()

// 	•	path = "." (current directory)
// 	•	target = "console"
// Step 2: Open the Directory

// 	•	The program opens the current directory ".".
// 	•	It retrieves metadata using fstat(), which identifies it as a directory (T_DIR).
// 	•	It then prepares to read the directory entries using read().

// Step 3: Iterating Over Directory Entries

// 	•	It reads entries in the current directory one by one.

// Iteration 1: Entry “file1.txt”

// 	•	de.name = "file1.txt"
// 	•	buf is initially "./", and p points to the end (buf + 2).
// 	•	After copying de.name to buf, buf becomes "./file1.txt".
// 	•	It calls stat(buf) to get the metadata, identifies it as a file (T_FILE).
// 	•	Since the name "file1.txt" does not match "console", it does not print anything.

// Iteration 2: Entry “console”

// 	•	de.name = "console"
// 	•	buf becomes "./console".
// 	•	It calls stat(buf) and identifies it as a file (T_FILE).
// 	•	Comparison:
//         if (strcmp(fmtname(buf), target) == 0) {
//             printf("%s\n", buf);
//         }
// 	•	fmtname(buf) extracts "console" from "./console", which matches the target.
// 	•	It prints:
//         ./console

// Iteration 3: Entry “dir1”

// 	•	de.name = "dir1"
// 	•	buf becomes "./dir1".
// 	•	It calls stat(buf) and identifies it as a directory (T_DIR).
// 	•	Since it’s a directory, it recursively calls find(buf, target):
//         find("./dir1", "console");
// Recursive Call: Searching in “dir1”

// 	1.	path = "./dir1"
// 	2.	The program opens "./dir1", retrieves metadata, and iterates over its entries.

// Sub-Iteration 1: Entry “file2.txt”

// 	•	de.name = "file2.txt"
// 	•	buf becomes "./dir1/file2.txt".
// 	•	It calls stat(buf), identifies it as a file, but does not print anything since it doesn’t match "console".

// Sub-Iteration 2: Entry “console”

// 	•	de.name = "console"
// 	•	buf becomes "./dir1/console".
// 	•	It calls stat(buf), identifies it as a file, and matches the target:
// 	•	fmtname(buf) extracts "console", which matches the target.
// 	•	It prints:
//         ./dir1/console

// Back to the Main Directory, Iteration 4: Entry “dir2”

// 	•	de.name = "dir2"
// 	•	buf becomes "./dir2".
// 	•	It calls stat(buf), identifies it as a directory, and recursively calls:
//         find("./dir2", "console");

// Recursive Call: Searching in “dir2”

// 	1.	path = "./dir2"
// 	2.	Opens "./dir2" and iterates over its entries.

// Sub-Iteration 1: Entry “file3.txt”

// 	•	de.name = "file3.txt"
// 	•	buf becomes "./dir2/file3.txt".
// 	•	It calls stat(buf) and identifies it as a file, but it does not match the target.

// Sub-Iteration 2: Entry “subdir”

// 	•	de.name = "subdir"
// 	•	buf becomes "./dir2/subdir".
// 	•	It calls stat(buf), identifies it as a directory, and recursively calls:
//         find("./dir2/subdir", "console");
// Recursive Call: Searching in “subdir”

// 	1.	path = "./dir2/subdir"
// 	2.	Opens "./dir2/subdir" and iterates over its entries.

// Sub-Iteration: Entry “console”

// 	•	de.name = "console"
// 	•	buf becomes "./dir2/subdir/console".
// 	•	It calls stat(buf), identifies it as a file, and matches the target:
// 	•	fmtname(buf) extracts "console", which matches the target.
// 	•	It prints:
//         ./dir2/subdir/console

// Final Output

// The output of your find command will be:
// ./console
// ./dir1/console
// ./dir2/subdir/console