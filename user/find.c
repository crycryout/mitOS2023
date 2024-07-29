
// after writing this code, I find myself really not good at string processing.
 
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#define MAXCHAR 64

// file operation related
struct stat st;
struct dirent de;

char *pattern;

//the ultimate output
void 
find(char *path, char * pattern){
  
  char buf[MAXCHAR+1]; 
  char* p = buf;

  int fd;
  memmove(buf, (const void *)path, sizeof(path));
  //to put p in the position of the file name 
  for(char* i = buf; (*i)!=0; i++){
    if(*i == '/'){
      p = i;
    }
  }
  p++;  
  
  if((fd = open(buf, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }
  
  if(fstat(fd, &st)){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    exit(1);
  } 
  
  switch (st.type) {
    case T_DEVICE:
    case T_FILE:
      if(!strcmp(p,pattern)){
        printf("%s\n",buf);
      }
      break;
    case T_DIR:
      p = buf + strlen(buf);
      *p++ = '/';
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
          continue;
        if(strcmp(de.name,".") == 0 || strcmp(de.name, "..") == 0){
          continue;
        }
        memmove(p,de.name, DIRSIZ);
        find(buf,pattern);
      }
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  if(argc <= 1){
    fprintf(2, "usage: find [file ...] pattern\n");
  }
  
  if(argc <= 2){
    exit(0);
  }
  
  pattern = argv[2];
  find(argv[1],pattern);
  
  exit(0);
}

