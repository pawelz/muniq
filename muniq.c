#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// A linked list of messages. Only elements such that next != NULL contain data. The last element is just a terminator.
struct msg {
  char *mid;
  char *fn;
  struct msg *next;
};

// Whether the h is a Message-ID header (case insensitive).
int ismid(char *h) {
  if (*h != 'M' && *h != 'm') return 0; h++;
  if (*h != 'E' && *h != 'e') return 0; h++;
  if (*h != 'S' && *h != 's') return 0; h++;
  if (*h != 'S' && *h != 's') return 0; h++;
  if (*h != 'A' && *h != 'a') return 0; h++;
  if (*h != 'G' && *h != 'g') return 0; h++;
  if (*h != 'E' && *h != 'e') return 0; h++;
  if (*h != '-')              return 0; h++;
  if (*h != 'I' && *h != 'i') return 0; h++;
  if (*h != 'D' && *h != 'd') return 0; h++;
  if (*h != ':')              return 0; h++;
  if (*h != ' ')              return 0;
  return 1;
}

// Reads Message-ID header from file named *fn, places the value of the header in *o.
int read_mid(char *fn, char *o) {
  FILE *file = fopen(fn, "r");
  char b[4096];
  do {
    char *rv = fgets(b, 4096, file);
    if (rv == NULL) {
      perror("Unable to read the file");
      return 1;
    }

    // End of headers, no Message ID found.
    if (b[0] == '\n' || b[0] == 0) {
      return 1;
    }
  } while(!ismid(b));
  fclose(file);

  // strcpy is for those of weak mind
  int i;
  for (i = 0; i < 4095 - 12; i++) {
    if (b[i+12] == 0 || b[i+12] == '\n' || b[i+12] == '\r') {
      break;
    }
    o[i] = b[i+12];
  }
  o[i] = 0;

  return 0;
}

// Sorts the linked list of messages by Message-ID.
struct msg *sort(struct msg *msgs) {
  struct msg *sorted = malloc(sizeof(struct msg));
  struct msg *c;
  struct msg *cs;
  sorted->next = NULL;

  for(c = msgs; c->next != NULL; c = c->next) {
    for(cs = sorted; cs->next != NULL; cs = cs->next) {
      if(strcmp(cs->mid, c->mid) > 0) {
        break;
      }
    }
    if (cs->next == NULL) {
      cs->fn = strdup(c->fn);
      cs->mid = strdup(c->mid);
      cs->next = malloc(sizeof(struct msg));
      cs->next->next = NULL;
      continue;
    }

    struct msg *next = malloc(sizeof(struct msg));
    next->fn = cs->fn;
    next->mid = cs->mid;
    next->next = cs->next;

    cs->fn = strdup(c->fn);
    cs->mid = strdup(c->mid);
    cs->next = next;
  }

  return sorted;
}

void remove_dupes(struct msg *msgs, int dryrun) {
  struct msg *c = msgs;
  for (c = msgs; c->next != NULL && c->next->next != NULL; c = c->next) {
    if (strcmp(c->mid, c->next->mid) == 0) {
      printf("rm %s\n", c->fn);
      if (!dryrun) {
        unlink(c->fn);
      }
    }
  }
}

void help() {
  printf("Usage: muniq [flags]\n");
  printf("Flags: --dryrun, -h, --help\n");
}

int main(int argc, char **argv) {
  DIR *dir;
  struct dirent *de;
  char mid[4096];
  struct msg msgs;
  struct msg *c = &msgs;
  int dryrun = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--dryrun") == 0) {
      dryrun = 1;
    } else if (strcmp(argv[i], "-h")*strcmp(argv[i], "--help") == 0 ) { 
      help();
      return 0;
    } else {
      fprintf(stderr, "Unrecognized flag '%s'.\n", argv[i]);
      help();
      return 1;
    }   
    continue;
  }

  msgs.next = NULL;

  dir = opendir(".");
  if (dir == NULL) {
    perror("Unable to read dir");
    return 1;
  }

  while((de = readdir(dir))) {
    if (de->d_type != DT_REG || de->d_name[0] == '.') {
      continue;
    }
    if (read_mid(de->d_name, mid) > 0) {
      fprintf(stderr, "Skipping file %s, failed to read the Message-ID header.\n", de->d_name);
      continue;
    }
    // The purpose of this check is to prevent the deduplication of all
    // messages, for which we really fail to read the message ID. Consider for
    // example multiline header like this:
    //
    // Message-ID:
    //     <xyz1234567890>
    //
    // All message IDs like that will map to an empty string and so muniq would
    // deduplicate them leading to data loss.
    //
    // The misread message IDs issue aside, short message IDs exponentially
    // increase the risk of ID collisions.
    if (strlen(mid) < 10) {
      fprintf(stderr, "Skipping file %s, the Message-ID has fewer than 10 characters.\n", de->d_name);
      continue;
    }

    c->fn = strdup(de->d_name);
    c->mid = strdup(mid);
    c->next = malloc(sizeof(struct msg));
    c = c->next;
    c->next = NULL;
  }
  closedir(dir);

  struct msg *sm = sort(&msgs);
  remove_dupes(sm, dryrun);

  return 0;
}
