
// after writing this code, I find myself really not good at string processing.
 
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#define MAXCHAR 1024 

// file operation related
struct stat st;
struct dirent de;

char *pattern;

//the ultimate output
char*
strncpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}


void 
find(char *path, char * pattern){
  
  
  char buf[MAXCHAR+1]; 
  char* p = buf;

  int fd;
  strncpy(buf, path, MAXCHAR);
//  memmove(buf, (const void *)path, MAXCHAR);
  buf[MAXCHAR] =  '\0';
  p= buf + strlen(path);
  *p = 0;
  //to put p in the position of the file name 
  for(char* i = buf; (*i)!=0; i++){
    if(*i == '/'){
      p = i;
    }
  }
  p++;  
  
  if((fd = open(buf, O_RDONLY)) < 0){
  //  fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  
//  printf("fd: %d , path: %s\n",fd, path); 

  if(fstat(fd, &st)){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  } 
 
  switch (st.type) {
    case T_DEVICE:
    case T_FILE:
      if(strcmp(p, pattern) == 0)
        printf("%s\n",path);
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
        strncpy(p,de.name,MAXCHAR-(p-buf)); 
//        memmove(p,de.name, sizeof(de.name));
//        p= p + strlen(de.name);
//        *p = 0;
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

