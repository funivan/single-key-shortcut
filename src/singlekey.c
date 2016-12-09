#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>
#include <regex.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
#include <ctype.h>


enum {
  RELEASED
};
enum {
  RESULT_OK,
  RESULT_FAILURE_OPEN_CONFIG,
  RESULT_FAILURE_OPEN_KEYBOARD_DEVICE,
  RESULT_INVALID_OPTIONS,
  RESULT_FAILURE_OPEN_DISPLAY,
  RESULT_INVALID_CONFIG_DEVICE_ID
};

enum {
  VERBOSE_NONE,
  VERBOSE_WARNING,
  VERBOSE_INFO,
  VERBOSE_DEBUG,
};

enum {
  ACTION_RUN,
  ACTION_LIST_DEVICES,
  ACTION_SHOW_HELP,
};


int verbose = VERBOSE_DEBUG;
int current_action = ACTION_RUN;

int DEVICE_ID = 0;
char *CONFIG_PATH = NULL;

char *commands[256] = {};


struct SupportedDeviceInfo {
  int id;
  char *name;
  char *path;
};

typedef struct SupportedDeviceInfo SupportedDeviceInfo;


void echo(int level, const char *fmt, ...) {
  //if (verbose >= level) {
    va_list args;
    int ret = 0;
    va_start(args, fmt);
    ret = vfprintf(stdout, fmt, args);
    va_end(args);
  //}
}


char *rtrim(const char *s) {
  while (isspace(*s) || !isprint(*s)) ++s;
  return strdup(s);
}

char *ltrim(const char *s) {
  char *r = strdup(s);
  if (r != NULL) {
    char *fr = r + strlen(s) - 1;
    while ((isspace(*fr) || !isprint(*fr) || *fr == 0) && fr >= r) --fr;
    *++fr = 0;
  }
  return r;
}

char *trim(const char *s) {
  char *r = rtrim(s);
  char *f = ltrim(r);
  free(r);
  return f;
}


/* External command execution */
static int ext_exec(char *cmd) {

  int result;

  echo(VERBOSE_INFO, "Executing: %s\n", cmd);

  pid_t pid;

  if (!(pid = fork())) {
    setsid();
    switch (fork()) {
      case 0:
        execlp("sh", "sh", "-c", cmd, (char *) NULL);
        break;
      default:
        _exit(0);
        break;
    }
  }

  if (pid > 0) {
    wait(NULL);
  }

  return RESULT_OK;
}

char *substring(char *s, int start, int len) {
  char *subString = (char *) malloc((len + 1) * sizeof(char));
  int n;
  for (n = 0; n < len; n++) {
    subString[n] = s[start + n];
  }
  return subString;
}

static void usage() {
  fprintf(stdout, "    Options are as follows:\n"
    "        -l            List available devices\n"
    "        -c <file>     Specify the configuration file to use\n"
    "        -d <id>       Specify the device id. Keyboard id in the configuration file will be overwritten\n"
    "        -h            Show this help\n");
}

void init_devices(Display *display, XDeviceInfo *devices, struct SupportedDeviceInfo **supported_devices, int num_devices) {
  int idx;
  int i;

  XInputClassInfo *ip;

  Atom xi_keyboard = XInternAtom(display, XI_KEYBOARD, 0);

  Atom *props;
  Atom prop;
  int ndevs = 0;
  int nprops = 0;

  XDeviceInfo *info = XListInputDevices(display, &ndevs);
  XDevice *device;

  Atom act_type;
  int act_format;
  unsigned long nitems, bytes_after;
  char *data;
  char *ret;

  prop = XInternAtom(display, "Device Node", False);

  unsigned char *df = "/tmp/1";
        
  for (idx = 0; idx < num_devices; ++idx) {
    supported_devices[idx] = malloc(sizeof(struct SupportedDeviceInfo));
    supported_devices[idx]->id = 0;
    supported_devices[idx]->name = NULL;
    supported_devices[idx]->path = NULL;

    if (xi_keyboard != devices[idx].type) {
      continue;
    }

    device = XOpenDevice(display, devices[idx].id);
    if (!device) {
      fprintf(stderr, "[ERROR] Cannot open device '%s'\n", devices[idx].name);
      XFreeDeviceList(info);
      continue;
    }


    if (!prop) {
      fprintf(stderr, "Cant detect device node\n");
      continue;
    }


    // get property of device
    XIGetProperty(display, devices[idx].id, prop, 0, 1000, False, AnyPropertyType, &act_type, &act_format, &nitems, &bytes_after, &data);
    // store all device info to memory

    supported_devices[idx]->id = (int) devices[idx].id;
    int id = (int) devices[idx].id;
    
    supported_devices[idx]->name = (char *) devices[idx].name;
    supported_devices[idx]->path = (char *) data;

    XCloseDevice(display, device);
  }

  return;
}


static void parse_options(int argc, char **argv) {
  int c;

  while ((c = getopt(argc, argv, "c:d:hl")) != -1) {
    switch (c) {
      case 'l':
        current_action = ACTION_LIST_DEVICES;
        break;
      case 'd':
        DEVICE_ID = atoi(strdup(optarg));

        if (DEVICE_ID == 0) {
          echo(VERBOSE_WARNING, "Invalid device id");
          exit(RESULT_INVALID_OPTIONS);
        }
      break;
      case 'c':
        if (!optarg) {
          usage();
          exit(RESULT_INVALID_OPTIONS);
        }
        CONFIG_PATH = strdup(optarg);
        break;
      case 'h':
        usage();
        exit(RESULT_OK);
        break;
      default:
        usage();
        exit(RESULT_INVALID_OPTIONS);
    }
  }

}


void read_config() {

  FILE *file;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i = 0;

  file = fopen(CONFIG_PATH, "r");
  if (file == NULL) {
    fprintf(stderr, "[ERROR] Can`t open config file\n");
    exit(RESULT_FAILURE_OPEN_CONFIG);
  }
  
  while ((read = getline(&line, &len, file)) != -1) {
    int lineLen;
    lineLen = strlen(line);
    
    if (line[0] == '#') {
      // line start with comment
      continue;
    }

    if (line[0] == '\n' || line[0] == '\0') {
      continue;
    }
    
    // first line must contain device ID
    if (DEVICE_ID == 0) {
       DEVICE_ID = atoi(line);
       if(DEVICE_ID == 0){
         fprintf(stderr, "[ERROR] Invalid device id\n");
         exit(RESULT_INVALID_CONFIG_DEVICE_ID);
       }
       continue;
    }



    if (lineLen < 3) {
      continue;
    }

    char symbol;
    int useKey = 1;


    int key;
    char *command;

    for (i = 0; i <= lineLen; ++i) {
      symbol = line[i];
      if (symbol == ':') {
        useKey = 0;
      }

      if (!useKey) {

        key = atoi(substring(line, 0, i));
        command = substring(line, i + 1, (lineLen));
        
        command = trim(command);
        if (key >= 0 && key < 256 && strlen(command) > 0) {
          commands[key] = command;
        }

        break;
      }

    }
  }

  fclose(file);
  if (line) {
    free(line);
  }
  
  echo(VERBOSE_INFO,"Loaded device ID: %d\n", DEVICE_ID); 
  echo(VERBOSE_INFO,"Loaded commands:\n");
  
  for (i = 0; i < 256; i++) {
    if (commands[i]) {
      echo(VERBOSE_INFO, "%d : %s\n", i, commands[i]); 
    }
  }


}

int main(int argc, char **argv) {
  FILE *file;
  struct input_event ev;
  char *command = "";
                  
  parse_options(argc, argv);

  char *display_name = XDisplayName (NULL);
  fprintf(stdout, "[display_name] %s \n", display_name);

  Display *display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "[ERROR] Could not connect to X server\n");
    exit(RESULT_FAILURE_OPEN_DISPLAY);
  }

  int num_devices;
  XDeviceInfo *devices = XListInputDevices(display, &num_devices);
  
  struct SupportedDeviceInfo **supported_devices;
  
  supported_devices = malloc(sizeof(struct SupportedDeviceInfo *) * num_devices);
  init_devices(display, devices, supported_devices, num_devices);
  
  if (current_action == ACTION_LIST_DEVICES) {
    int index;
    for (index = 0; index < num_devices; ++index) {
      if (supported_devices[index] && supported_devices[index]->name) {
        echo(VERBOSE_INFO, "%d\t%s\t\n", supported_devices[index]->id, supported_devices[index]->name);
      }
    }
    
    XCloseDisplay(display);
    return RESULT_OK;
  }


  read_config();

  char *DEVICE_PATH = NULL;

  int index;
  for (index = 0; index < num_devices; ++index) {
    if (supported_devices[index]->name == NULL) {
      continue;
    }

    int id = supported_devices[index]->id;
    if (DEVICE_ID == id) {
      DEVICE_PATH = supported_devices[index]->path;
      echo(VERBOSE_INFO, "Detect device input file path: %s\n", DEVICE_PATH);
      break;
    }
  }

  echo(VERBOSE_INFO,"DEVICE_PATH: %s\n", DEVICE_PATH); 

  if (DEVICE_PATH == NULL) {
    fprintf(stderr, "[ERROR] Can`t detect device input file path. Possible invalid device id\n");
    exit(RESULT_INVALID_OPTIONS);
  }


  echo(VERBOSE_INFO,"start open file\n"); 


  //verbose = VERBOSE_DEBUG;
  
  // custom commands

  file = fopen(DEVICE_PATH, "r");
  if ( file == NULL ) {
    fprintf(stderr, "[ERROR] Can`t open device input file\n");
    exit(RESULT_FAILURE_OPEN_KEYBOARD_DEVICE);
  }
    
    
 
  char buf[256];
  snprintf(buf, sizeof buf, "xinput disable %d", DEVICE_ID);
  echo(VERBOSE_INFO,"Disable device with command: %s\n", buf);
  ext_exec(buf);
  

  int keyRelease = NULL;
  int showKey = 1;

  while (1) {
    if (1 != fread(&ev, sizeof(struct input_event), 1, file)) {
      continue;
    }

    keyRelease = (ev.type == EV_KEY && ev.value == RELEASED);


    if (!keyRelease) {
      continue;
    }


    if (showKey) {
      echo(VERBOSE_INFO,"KEY fire %d\n", (int) ev.code); 
    }

    command = commands[ev.code];

    if (command && command[0] != '\0') {
      echo(VERBOSE_INFO,"KEY %d\n", (int) ev.code); 
      ext_exec(command);
    } else {
      echo(VERBOSE_INFO,"Empty Command %s\n", command); 
    }


  }


  return RESULT_OK;
}


