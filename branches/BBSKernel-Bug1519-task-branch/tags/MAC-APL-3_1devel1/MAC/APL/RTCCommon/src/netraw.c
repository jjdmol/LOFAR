//#
//#  netraw.cc: change capabilities to allow raw ethernet access by non-root user.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <sys/capability.h>

#if defined(ENABLE_NET_RAW)
bool set_capabilities(char *str)
{
  if (!str) return false;            /** RETURN **/
  
  // must be effectively uid root
  if (0 != geteuid()) return false;  /** RETURN **/

  // but not uid root itself
  if (0 == getuid()) return false;   /** RETURN **/

  // convert string to cap_t
  cap_t newcap = cap_from_text(str);
  if (!newcap) return false;         /** RETURN **/
  
  // set new capabilities
  if (cap_set_proc(newcap) < 0)
  {
    cap_free(newcap);
    perror("cap_set_proc");
    return false;                    /** RETURN **/
  }
  cap_free(newcap);

  // setuid to effective user
  setuid(getuid());

  // convert string to cap_t
  newcap = cap_from_text(str);
  if (!newcap) return false;         /** RETURN **/
  
  // set new capabilities
  if (cap_set_proc(newcap) < 0)
  {
    cap_free(newcap);
    perror("cap_set_proc");
    return false;                    /** RETURN **/
  }
  cap_free(newcap);

  return true;
}
#endif

/* This program demonstrates the use of POSIX.1e capabilities in stock Linux
 * distributions, with a view to adding the same functionality to Ethereal.
 * Note that this code requires the kernel headers, but not the libcap library.
 * for the real version we should use libcap rather than calling the kernel directly.
*/

#include <sys/capability.h>

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/prctl.h>
#include <linux/unistd.h>
#include <linux/capability.h>
#include <stdlib.h>


_syscall2(int, capget, cap_user_header_t, header, cap_user_data_t, dataptr);
_syscall2(int, capset, cap_user_header_t, header, cap_user_data_t, dataptr);

typedef struct __user_cap_header_struct capheader_t;
typedef struct __user_cap_data_struct capdata_t;

char * isbit (__u32 word, int bit, char * buffer)
{
  sprintf (buffer, word & (1<< bit) ?"SET": "CLEAR");
  return buffer;
}

char * describe_cap (capdata_t *data, int cap)
{
  static char buffer[80];
  static char buffer1[10], buffer2[10],buffer3[10];

  sprintf (buffer, "effective %s, permitted %s, inheritable %s",
          isbit(data->effective,cap, buffer1),
          isbit(data->permitted,cap, buffer2),
          isbit(data->inheritable,cap, buffer3));
  return buffer;
}

void remove_all(capdata_t *data) {
  data->effective = 0;
  data->permitted = 0;
  data->inheritable = 0;
}

void add_cap(capdata_t *data, int cap) {
//  data->effective |= (1 << cap);
  data->permitted |= (1 << cap);
}

void add_cap_eff(capdata_t *data, int cap) {
  data->effective |= (1 << cap);
}

void add_cap_inh(capdata_t *data, int cap) {
  data->inheritable |= (1 << cap);
}

void cap_get(capheader_t *header, capdata_t *data) {
  if (capget(header, data) == 0) return;
  perror("capget");
  exit(-1);
}

void cap_set(capheader_t *header, capdata_t *data) {
  if (capset(header, data) == 0) return;
  perror("capset");
  exit(-1);
}

#if 0
int main(int argc, char** argv)
{
  cap_t cap;
  
  cap = cap_get_proc();
  
  if (cap)
  {
    printf("cap_to_text:\n%s", cap_to_text(cap, NULL));
  }
  else
  {
    fprintf(stderr, "cap_get_proc: failed\n");
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}
#endif


void set_capabilities(char* str)
{
  cap_t newcap = cap_from_text(str);
  if (newcap)
  {
    if (cap_set_proc(newcap) < 0)
    {
      cap_free(newcap);
      perror("cap_set_proc");
      exit(EXIT_FAILURE);
    }

    cap_free(newcap);
  }
}

void print_capabilities(FILE* file)
{
  cap_t cap = cap_get_proc();
  
  if (cap)
  {
    fprintf(file, "capabilities%s\n", cap_to_text(cap, NULL));
    cap_free(cap);
  }
}

cap_t create_capabilities(char *str)
{
  cap_t newcap = cap_from_text(str);
  if (newcap)
  {
    if (cap_set_proc(newcap) < 0)
    {
      cap_free(newcap);
      return 0;
    }
  }

  return newcap;
}

int main(int argc, char** argv)
{
  print_capabilities(stdout);

  if (geteuid() == 0)
  {
    set_capabilities("= cap_net_raw,cap_net_admin,cap_setpcap+eip");
    print_capabilities(stdout);
  }
#if 0
  else
  {
    set_capabilities("= cap_net_raw,cap_net_admin+ep");
    exit(EXIT_SUCCESS);
  }
  
  print_capabilities(stdout);

  print_capabilities(stdout);

  set_capabilities("cap_net_raw,cap_net_admin+eip");
  print_capabilities(stdout);
#endif
  
  pid_t pid = fork();
  switch (pid)
  {
    case 0:
      printf ("Forking keeps the same capabilities for the child process:\n");

      setuid(getuid());
      print_capabilities(stdout);

      if (execvp(argv[1], &argv[1]) < 0)
      {
	perror("execvp");
	exit(EXIT_FAILURE);
      }
      break;
   
    case -1:
      perror ("fork failed");
      break;
     
    default:
    {
      
      /* enable capabilities for cap_net_raw and cap_net_admin in child process */
      cap_t cap = create_capabilities("cap_net_raw,cap_net_admin+eip");
      if (cap)
      {
	if (capsetp(pid,cap) < 0)
	{
	  perror("capsetp");
	}
	cap_free(cap);
      } else perror("create_capabilities");
      
      wait (NULL);
    }
    break;
  }

  exit(EXIT_SUCCESS);
}
