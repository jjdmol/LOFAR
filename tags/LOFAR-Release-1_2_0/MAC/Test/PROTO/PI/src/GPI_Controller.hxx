/***************************************************************************
                          GPI_Controller.hxx  -  description
                             -------------------
    begin                : Fri Jul 11 2003
    copyright            : (C) 2003 by pvss
    email                : pvss@sun.solarsystem
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GPI_CONTROLLER_HXX
#define GPI_CONTROLLER_HXX

#include <TASK/GCF_Task.hxx>
#include <set>

/**
  *@author pvss
  */
class GCFPortManager;
class GCFEvent;
class GCFPortInterface;
class DpVCItem;

class GPIController : public GCFTask
{
  public: 
	  GPIController();
	  ~GPIController();

    // callback from signal handler
    static void signalHandler(int sig);
    // our exit flag. The signal handler will set it to PVSS_TRUE
  	static PVSSboolean doExit;

  protected:
    void init();
    void workProc();
    int initial_state(GCFEvent& e, GCFPortInterface& p);
    int operational_state(GCFEvent& e, GCFPortInterface& p);

    unsigned short _curSeqNr;
    GCFPortManager *_pPortManager;
  private: // Private methods
    void subscribe(char* arg, bool onOff);
    void valueChanged(GCFEvent& e, GCFPortInterface& p);
    void setValue(DpVCItem* pVCItem);

  private: // Private sequence handling

    typedef struct
    {
      unsigned short seqNr;
      char* seqArg;
    } TSequenceEntry;

    struct TSeqEntryCompare
    {
      bool operator()(const TSequenceEntry& a, const TSequenceEntry& b) const
      {
        return a.seqNr < b.seqNr;
      }
    };

    unsigned short registerSequence(const char *arg);
    char* getSequenceArg(unsigned short seqNr);
    void deregisterSequence(unsigned short seqNr);

    set<TSequenceEntry, TSeqEntryCompare> _sequenceQueue;
    
    typedef set<TSequenceEntry, TSeqEntryCompare>::iterator CLI;

    typedef set<TSequenceEntry, TSeqEntryCompare>::const_iterator CCLI;
};

#endif
