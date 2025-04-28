/*
  Copyright © 2025 SpyrjaGaldr (spyrja@proton.me)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

const char* version_info = "path v25.4.29 Copyright © 2025 SpyrjaGaldr";

#include <fnmatch.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#define stat _stat
#ifdef NO_MSYS
#define fnmatch(gist, text, ignored) PathMatchSpec(text, gist)
#endif
const char* root_directory = "C:\\";
char path_delimiter = '\\';
#else
#include <sys/stat.h>
const char* root_directory = "/";
char path_delimiter = '/';
#endif

size_t max_results = 0;
size_t result_counter = 0;
bool file_search = false;
bool case_sensitive = false;
bool exact_mode = false;
bool fuzzy_mode = false;
bool prefix_mode = false;
bool suffix_mode = false;
bool quote_embedded = false;
bool xargs_format = false;
bool warnings_on = false;
const char** needle_list = NULL;
size_t needle_counter = 0;
const char** directory_list = NULL;
size_t directory_counter = 0;
const char* first_directory = NULL;

bool wildcard_match(const char* text, const char* gist) {
  int flg = 0;
  if (!case_sensitive)
    flg = FNM_CASEFOLD;
  return fnmatch(gist, text, flg) == 0;
}

bool fuzzy_match(const char* text, const char* gist) {
  while (true) {
    char rhs = *gist;
    if (rhs == 0)
      return true;
    if (!case_sensitive)
      rhs = tolower(rhs);
    char lhs = *text++;
    if (lhs == 0)
      return false;
    if (!case_sensitive)
      lhs = tolower(lhs);
    if (lhs != rhs)
      continue;
    ++gist;
  }
  return false;
}

bool prefixed_match(const char* text, const char* prefix) {
  size_t cnt = strlen(prefix);
  int res;
  if (case_sensitive)
    res = strncmp(text, prefix, cnt);
  else
    res = strncasecmp(text, prefix, cnt);
  return res == 0;
}

bool suffixed_match(const char* text, const char* suffix) {
  size_t cnt = strlen(suffix);
  size_t len = strlen(text);
  if (len < cnt)
    return false;
  const char* pos = text + len - cnt;
  int res;
  if (case_sensitive)
    res = strncmp(pos, suffix, cnt);
  else
    res = strncasecmp(pos, suffix, cnt);
  return res == 0;
}

void process_next(const char* directory, const char* path) {
  bool fnd = false;
  size_t ndx = needle_counter;
  while (ndx--) {
    const char* ned = needle_list[ndx];
    if (exact_mode) {
      if (strcmp(path, ned) == 0) {
        fnd = true;
        break;
      }
    } else if (prefix_mode) {
      if (prefixed_match(path, ned)) {
        fnd = true;
        break;
      }
    } else if (suffix_mode) {
      if (suffixed_match(path, ned)) {
        fnd = true;
        break;
      }
    } else if (fuzzy_mode) {
      if (fuzzy_match(path, ned)) {
        fnd = true;
        break;
      }
    } else if (case_sensitive) {
      if (strcasecmp(path, ned) == 0) {
        fnd = true;
        break;
      }
    } else {
      if (wildcard_match(path, ned)) {
        fnd = true;
        break;
      }
    }
  }
  if (fnd) {
    if (max_results != 0 && result_counter++ >= max_results)
      exit(EXIT_SUCCESS);
    char sep = '\n';
    const char* sur = "";
    if (xargs_format) {
      sep = ' ';
      if (strchr(directory, ' ') || strchr(path, ' '))
        sur = "\"";
    } else if (quote_embedded) {
      if (strchr(directory, ' ') || strchr(path, ' '))
        sur = "\"";
    }
    printf("%s%s%c%s%s%c", sur, directory, path_delimiter, path, sur, sep);
  }
}

char* current_directory(void) {
  static size_t max;
  static char* cur = NULL;
  if (cur == NULL)
    cur = malloc(max = PATH_MAX);
  while (getcwd(cur, max) == NULL)
    cur = realloc(cur, max *= 2);
  return strdup(cur);
}

void quiet_chdir(const char* path) {
  int ret = chdir(path);
  (void)ret;
}

void traverse_directory(const char* directory) {
  DIR* dir = opendir(directory);
  if (dir == NULL) {
    if (warnings_on)
      fprintf(stderr, "Error: cannot open directory '%s' (access denied)\n",
              directory);
    return;
  }
  quiet_chdir(directory);
  char* cur = current_directory();
  if (first_directory == NULL || strcmp(cur, first_directory) != 0)
    while (true) {
      struct dirent* nxt = readdir(dir);
      if (nxt == NULL)
        break;
      char* pth = nxt->d_name;
      if (strcmp(pth, ".") == 0 || strcmp(pth, "..") == 0)
        continue;
      struct stat inf;
      if (lstat(pth, &inf) != 0) {
        if (warnings_on)
          fprintf(
              stderr,
              "Error: cannot stat file/directory '%s%c%s' (access denied)\n",
              directory, path_delimiter, pth);
        continue;
      }
      if (S_ISDIR(inf.st_mode)) {
        if (!file_search)
          process_next(cur, pth);
        traverse_directory(pth);
        quiet_chdir(cur);
      } else if (S_ISREG(inf.st_mode) && file_search)
        process_next(cur, pth);
    }
  closedir(dir);
  free(cur);
}

int usage(const char* argv0) {
  fprintf(stderr, "%s\n", version_info);
  fprintf(stderr,
          "Usage: %s [-d DIRECTORY] [-f] [-n MAX] [-c] [-e] [-z] [-p] [-s] "
          "[-q] [-x] "
          "[-w] [-v] "
          "[NEEDLE(S)...]\n",
          argv0);
  fputs("  -d  Include DIRECTORY\n", stderr);
  fputs("  -f  Look for files\n", stderr);
  fputs("  -n  First MAX results only\n", stderr);
  fputs("  -c  Case-sensitive\n", stderr);
  fputs("  -e  Exact-match mode\n", stderr);
  fputs("  -z  Fuzzy-search mode\n", stderr);
  fputs("  -p  Prefix-search mode\n", stderr);
  fputs("  -s  Suffix-search mode\n", stderr);
  fputs("  -q  Quote embedded spaces\n", stderr);
  fputs("  -x  Print on a single line\n", stderr);
  fputs("  -w  Warnings on\n", stderr);
  fputs("  -v  Print version\n", stderr);
  fputs("Note:\n", stderr);
  fputs(
      "* Default behaviour is to look for directories (specify -f for files)\n",
      stderr);
  fputs("* Fuzzy-search matches a string such as 'aoa' with 'anaconda'\n",
        stderr);
  fputs(
      "* If neither -e, -z, -p, or -s are specified, wildcards can be used for "
      "matching\n",
      stderr);
  fputs(
      "* Each additional search directory must be specified with a separate -d "
      "flag\n  (multiple directories can be included by separating them with "
      "a ':' or ';')\n",
      stderr);
  fputs(
      "* Warnings (enabled with the -w flag) may occur with insufficient "
      "permissions \n",
      stderr);
  return EXIT_FAILURE;
}

void add_directory(const char* directory) {
  size_t mor = directory_counter + 1;
  directory_list = realloc(directory_list, mor * sizeof(char*));
  directory_list[directory_counter] = directory;
  directory_counter = mor;
}

int main(int argc, char** argv) {
  if (argc == 1)
    return usage(argv[0]);
  while (true) {
    int opt = tolower(getopt(argc, argv, "d:fn:cezpsqxwvh"));
    if (opt < 0)
      break;
    if (opt == 'd') {
      char* fld = optarg;
      while (true) {
        char* end = strchr(fld, ':');
        if (!end)
          end = strchr(fld, ';');
        if (end == NULL) {
          add_directory(fld);
          break;
        }
        *end = 0;
        add_directory(fld);
        fld = ++end;
      }
    } else if (opt == 'f')
      file_search = true;
    else if (opt == 'n') {
      max_results = atoi(optarg);
      if (max_results <= 0) {
        fprintf(stderr,
                "Error: MAX value must be greater than zero (got '%s')\n",
                optarg);
        return EXIT_FAILURE;
      }
    } else if (opt == 'c')
      case_sensitive = true;
    else if (opt == 'e')
      exact_mode = true;
    else if (opt == 'z')
      fuzzy_mode = true;
    else if (opt == 'p')
      prefix_mode = true;
    else if (opt == 's')
      suffix_mode = true;
    else if (opt == 'q')
      quote_embedded = true;
    else if (opt == 'x')
      xargs_format = true;
    else if (opt == 'w')
      warnings_on = true;
    else if (opt == 'v')
      fprintf(stderr, "%s\n", version_info);
    else
      return usage(argv[0]);
  }
  if ((exact_mode + fuzzy_mode + prefix_mode + suffix_mode) > 1) {
    fputs(
        "Error: 'exact', 'fuzzy_match', 'prefix', and 'suffix' modes are "
        "mutually "
        "exclusive\n",
        stderr);
    return usage(argv[0]);
  }
  size_t num = 0;
  for (int index = optind; index < argc; index++) {
    size_t mor = needle_counter + 1;
    needle_list = realloc(needle_list, mor * sizeof(char*));
    needle_list[needle_counter] = argv[index];
    needle_counter = mor;
    ++num;
  }
  if (directory_list == NULL) {
    traverse_directory(".");
    first_directory = current_directory();
    traverse_directory(root_directory);
  } else
    for (size_t ddx = 0; ddx < directory_counter; ++ddx) {
      const char* dpt = directory_list[ddx];
      struct stat inf;
      if (lstat(dpt, &inf) != 0 || !S_ISDIR(inf.st_mode)) {
        fprintf(stderr, "Error: cannot stat directory '%s'\n", dpt);
        continue;
      }
      if (num != 0)
        traverse_directory(dpt);
    }
  if (num == 0) {
    if (warnings_on)
      fputs("Warning: nothing was processed\n", stderr);
  }
  quiet_chdir("/");
  return EXIT_SUCCESS;
}
