//# Semaphore.h: semaphore implementation on top of pthreads
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$ 

#ifndef  CEPFRAME_SEMAPHORE_H
#define  CEPFRAME_SEMAPHORE_H

#include <pthread.h>
 
class Semaphore
{
  public:
    Semaphore(unsigned level = 0);
    ~Semaphore();

    void up(unsigned count = 1);
    void down(unsigned count = 1);
    
  private:
    pthread_mutex_t mutex;
    pthread_cond_t  condition;
    unsigned	    level;
};
 
#endif
