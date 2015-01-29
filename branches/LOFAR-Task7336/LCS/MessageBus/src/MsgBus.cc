#include "MsgBus.h"

using namespace qpid::messaging;
using namespace qpid::types;

using std::stringstream;
using std::string;

#define S_OPEN 1
#define S_SESSION 2
#define S_SENDER 4
#define S_RECEIVER 8

  void FromBus::cleanup(void)
  {
//        if (state&S_SENDER) { sender = 0; state &= ~S_SENDER;}
//       if (state&S_SESSION) { session = 0 ; state &= ~S_SESSION;}
//        if (state&S_OPEN) { this.close(); state &= ~S_OPEN;}
  }


  FromBus::FromBus(const std::string &address, const std::string &options, const std::string &broker)
  :Connection(broker)
  {
     try {
            open();
            session = createSession();
            receiver = session.createReceiver(address);
     } catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
        close();
    }
  }
  std::string FromBus::GetStr(double dtimeout) // timeout 0.0 means blocking
  {
        Duration timeout=Duration::FOREVER;
        if (dtimeout>0.0) timeout = (Duration) (1000.0 * dtimeout);
        Message incoming = receiver.fetch(timeout);
        DiffNumAck ++;
        return incoming.getContent();
  }

  Message FromBus::GetMsg(double dtimeout) // timeout 0.0 means blocking
  {
        Duration timeout=Duration::FOREVER;
        if (dtimeout>0.0) timeout = (Duration) (1000.0 * dtimeout);
        Message incoming = receiver.fetch(timeout);
        DiffNumAck ++;
        return incoming;
  }

  void FromBus::Ack(void)
  {
     session.acknowledge();
     DiffNumAck --;
  }
 
  FromBus::~FromBus(void)
  {
    if (DiffNumAck) { std::cout << "Queue " << queuename << " on broker " << brokername << " has " << DiffNumAck << " messages not ack'ed " << std::endl;};
    close();
  }

 void ToBus::cleanup(void)
  {
//        if (state&S_SENDER) { sender = 0; state &= ~S_SENDER;}
//        if (state&S_SESSION) { session = 0 ; state &= ~S_SESSION;}
//        if (state&S_OPEN) { this.close(); state &= ~S_OPEN;}
  }
  
 ToBus::ToBus(const std::string &address, const std::string &options, const std::string &broker) 
  : Connection(broker)
  {
     queuename = string(address);
     brokername = string( broker);
     state = 0;
     DiffNumAck= 0;
     try {
            open();
            state |= S_OPEN;
            session = createSession();
            state |= S_SESSION;
            sender = session.createSender(address);
            state |= S_SENDER;
     } catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
        cleanup();
    }
  }

  void ToBus::Send( std::string & m)
  {
        Message tosend(m);
        sender.send(tosend,true);
        DiffNumAck ++;
  }
 ToBus::~ToBus(void)
  {
    cleanup();
  }
 void MultiBus::cleanup(void)
  {
//        if (state&S_SESSION) { session = 0 ; state &= ~S_SESSION;}
//        if (state&S_OPEN) { this.close(); state &= ~S_OPEN;}
  }

  MultiBus::MultiBus(MsgHandler handler, const std::string &address, const std::string &options, const std::string &broker) 
  : Connection(broker)
  {
     string queuename = string(address);
     brokername = string(broker);
     state = 0;
     DiffNumAck= 0;
     try {
            open();
            state |= S_OPEN;
            session = createSession();
            state |= S_SESSION;
            Receiver receiver = session.createReceiver(address);
            receiver.setCapacity(1);
            MsgWorker *worker=new MsgWorker;
            worker->handler=handler;
            worker->queuename=queuename;
            handlers[receiver]=worker;
            state |= S_RECEIVER;
     } catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
        cleanup();
    }
  }

  void MultiBus::add(MsgHandler handler, const std::string &address, const std::string &options)
  {
            Receiver receiver = session.createReceiver(address);
            receiver.setCapacity(1);
            MsgWorker * worker = new MsgWorker;
            worker->handler=handler;
            worker->queuename=string(address);
            handlers[receiver]=worker;
            state |= S_RECEIVER;
  }

  void MultiBus::HandleMessages(void)
  {
     while (1)
     {
        Receiver nrec = session.nextReceiver();
        MsgWorker *worker = handlers[nrec];
        Message msg;
        nrec.get(msg);
        std::string tmp= msg.getContent();
        if (worker)
        {
           if (worker->handler)
              if (worker->handler(tmp,worker->queuename)) session.acknowledge(msg);
              else // todo: define a proper fail over mechanism with proper handling
                       session.reject(msg); // broker can be configured for this
           else
              std::cout << "Error: undefined handler" << std::endl;
        } else {
           std::cout << "Error: incoming message but receiver not properly configured" << std::endl;
        }

     }
  }

  Message MultiBus::Get(double dtimeout)
  {

   Duration timeout=Duration::FOREVER;
   if (dtimeout>0.0) timeout = (Duration) (1000.0 * dtimeout);
   Receiver nrec = session.nextReceiver();
   Message msg = nrec.get(timeout);
   return msg;
  }

  MultiBus::~MultiBus()
  {
    cleanup();
    // fixme: memory leak for workers. Also infinite loop needs a fix.
  }

