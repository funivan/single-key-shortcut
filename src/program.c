#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>
#include <regex.h>

enum {
   RELEASED,
   PRESSED, 
   REPEATED
};
enum {
   RESULT_OK,
   RESULT_FAILURE_OPEN_CONFIG,
   RESULT_FAILURE_OPEN_DEVICE,
   RESULT_INVALID_OPTIONS
};
enum {
   VERBOSE_NONE,
   VERBOSE_WARNING,
   VERBOSE_INFO,
   VERBOSE_DEBUG,
};


int verbose = VERBOSE_DEBUG;
char *DEVICE_INPUT_PATH = NULL;
char *CONFIG_PATH = NULL;
 
char *commands[256] = {};

void echo(int level, const char *fmt, ...) {
    if (verbose >= level) {
      va_list args;
      int ret = 0;
      va_start(args, fmt);
    	ret = vfprintf(stdout, fmt, args);
      va_end(args);
    }
}


char *rtrim(const char *s)
{
  while( isspace(*s) || !isprint(*s) ) ++s;
  return strdup(s);
}
 
char *ltrim(const char *s)
{
  char *r = strdup(s);
  if (r != NULL)
  {
    char *fr = r + strlen(s) - 1;
    while( (isspace(*fr) || !isprint(*fr) || *fr == 0) && fr >= r) --fr;
    *++fr = 0;
  }
  return r;
}
 
char *trim(const char *s)
{
  char *r = rtrim(s);
  char *f = ltrim(r);
  free(r);
  return f;
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

char* substring(char* s, int start, int len){
    char* subString = (char*)malloc((len + 1) * sizeof(char));
    int n;
    for(n = 0; n < len ; n++){
      subString[n] = s[start + n];
    }
    return subString;
}

static void usage() {
    printf(
	"    Options are as follows:\n"
	"        -c <file>     Specify the configuration file to use\n"
	"        -d <file>     Specify device file path\n"
  "        -h            Show this help\n");
}

static void parse_options(int argc, char **argv) {
    int c;
        
    while ((c = getopt (argc, argv, "d:c:h")) != -1){
      switch (c) {
          case 'h':
            usage();
            exit(RESULT_OK);
            break;
          case 'd':
            if (!optarg) {
              usage();
              exit(RESULT_INVALID_OPTIONS);
            }
            DEVICE_INPUT_PATH = strdup(optarg); 
            break;
          case 'c':
            if (!optarg) {
                usage();
                exit(RESULT_INVALID_OPTIONS);
            }
            CONFIG_PATH = strdup(optarg);
            break;
          default:
            usage();
            exit(RESULT_INVALID_OPTIONS);
            
      }
    }
    
    if (CONFIG_PATH == NULL || DEVICE_INPUT_PATH == NULL){
      usage();
      exit(RESULT_INVALID_OPTIONS);
    }
}


void init_commands(){
  
   FILE *file;
   char * line = NULL;
   size_t len = 0;
   ssize_t read;
    int i=0;
   
   file = fopen(CONFIG_PATH, "r");
   if (file == NULL){
    exit(RESULT_FAILURE_OPEN_CONFIG);
   }

  while ((read = getline(&line, &len, file)) != -1) {
    
     
     int lineLen;
     lineLen = strlen(line);
              
     if(lineLen < 3 || line[0]== '#' || line[0] == '\n' ||  line[0] == '\0' ){
      // line start with comment
      continue;
     }

//      printf("Retrieved line of length %zu :\n", read);
//      printf("%s", line);
     
         char symbol;
         int useKey=1;
         
                  
         int key;         
         char *command;
   
         for (i = 0; i <= lineLen; ++i) {
           symbol = line[i]; 
            if (symbol == ':' ) {
              useKey = 0;
           }
           
           if(!useKey){
           
              key = atoi(substring(line, 0, i));
              command = substring(line, i+1,(lineLen));

//              printf("KEY : %d\n", key);
//              printf("COMMAND : %s\n", command);
              command = trim(command);              
              if ( key >= 0 && key < 256 && strlen(command)>0){
                commands[key] = command;
              }

              break;
           }

     	   }
   }

   fclose(file);
   if (line){ 
    free(line);
   }
   printf("Loaded commands:");
   for(i = 0; i < 256; i++) {
    if(commands[i]){
      printf("%d : %s\n", i, commands[i]);
    }
   }       
          
}
int main(int argc, char **argv) {
  FILE *file;
  struct input_event ev;
  char *command = "";
  
  parse_options(argc, argv);
  
  echo(VERBOSE_DEBUG, "DEVICE_INPUT_PATH: %s\n", DEVICE_INPUT_PATH);
  echo(VERBOSE_DEBUG, "CONFIG_PATH: %s\n", CONFIG_PATH);
  
  init_commands();
  
     
  verbose = VERBOSE_DEBUG;
  // custom commands
  
  file = fopen(DEVICE_INPUT_PATH, "r");
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


