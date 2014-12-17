#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>

enum {
   RELEASED,
   PRESSED, 
   REPEATED
};
enum {
   RESULT_OK,
   RESULT_FAILURE_OPEN_DEVICE
};
enum {
   VERBOSE_NONE,
   VERBOSE_WARNING,
   VERBOSE_INFO,
   VERBOSE_DEBUG,
};


int verbose = VERBOSE_DEBUG; 
 


void echo(int level, const char *fmt, ...) {
    if(verbose >= level){
      va_list args;
      int ret = 0;
      va_start(args, fmt);
    	ret = vfprintf(stdout, fmt, args);
      va_end(args);
    }
}




/* External command execution */
static int ext_exec(char *cmd) {

  int result;
  
  
  
  echo(VERBOSE_INFO ,"Executing: %s\n", cmd);
  
  pid_t pid;
  
  if (!(pid = fork())) {
    setsid();
    switch (fork()) {
      case 0: 
        result = execlp ("sh", "sh", "-c", cmd, (char *) NULL);
        // @todo doesen`t work
        if ( result == -1 ) {
          echo(VERBOSE_WARNING,"[WARNING] Command failure: %s \n", cmd);
        } else {
          echo(VERBOSE_DEBUG ,"[SUCCESS] %s \n", cmd);
        }
        
      break;
      default: 
        _exit(0);
      break;
    }
  }
  
  if (pid > 0){
    wait(NULL);
  }
  
  return RESULT_OK;
}



int main(void) {
  FILE *file;
  struct input_event ev;
  char *command = "";
  char *commands[256] = {};
  
  verbose = VERBOSE_DEBUG;
  // custom commands
  
  commands[125] = "term-command 'source ~/.ssh/environment && a-git-pull && sleep 10'";
  commands[29] = "term-command 'source ~/.ssh/environment && a-g-sync && sleep 10'";
    
  // vk
  commands[67] = "terminator --hidden --config=fast -e vk.prevTrack";         // f9
  commands[68] = "terminator --hidden --config=fast -e vk.nextTrack";         // f10
  commands[87] = "terminator --hidden --config=fast -e vk.toggle";            // f11
  commands[88] = "terminator --hidden --config=fast -e vk.addCurrentTrack";   // f12
    
  
           
  file = fopen("/dev/input/event5", "r");
  if (!file) {
    perror("fopen");
    exit(RESULT_FAILURE_OPEN_DEVICE);
  }

  int keyRelease = NULL;
  int showKey = 0;
  
  while(1){
    if(1 != fread(&ev, sizeof(struct input_event), 1, file)){
      continue;
    }

    keyRelease = (ev.type == EV_KEY && ev.value==RELEASED);

    
    if(!keyRelease){
      continue;
    }
    

    if(showKey){
      echo(VERBOSE_INFO, "KEY fire %d\n", (int)ev.code);
      continue;
    }
    
      command = commands[ev.code];

      if (command && command[0] != '\0'){
        echo(VERBOSE_INFO, "KEY %d\n", (int)ev.code);
        ext_exec(command);
      } else{
        echo(VERBOSE_DEBUG, "Empty Command %s\n", command);
      }
    


  }
  

  return RESULT_OK;
}


