//#  AVTAPCAnswer.h: forwards property set answers to the specified task.
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

#ifndef AVTAPCAnswer_H
#define AVTAPCAnswer_H

//# Includes
//# Common Includes
//# GCF Includes
#include <GCF/GCF_Answer.h>
//# VirtualTelescope Includes

// forward declaration

class GCFEvent;
class AVTAPCAnswerHandlerInterface;

class AVTAPCAnswer : public GCFAnswer
{
  public:
    explicit AVTAPCAnswer(AVTAPCAnswerHandlerInterface& handler);
    virtual ~AVTAPCAnswer();

    virtual void handleAnswer(GCFEvent& answer);
    
  protected:
    // protected default constructor
    AVTAPCAnswer();
    // protected copy constructor
    AVTAPCAnswer(const AVTAPCAnswer&);
    // protected assignment operator
    AVTAPCAnswer& operator=(const AVTAPCAnswer&);

  private:    
    AVTAPCAnswerHandlerInterface& m_handler;
};
#endif
