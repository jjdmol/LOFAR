#include "lofar_config.h"
#include <MessageBus/MsgBus.h>

#include <iostream>

using namespace qpid::messaging;

using std::string;

namespace LOFAR {

#define S_OPEN 1
#define S_SESSION 2
#define S_SENDER 4
#define S_RECEIVER 8

  static Duration TimeOutSecs(double secs)
  {
    if (secs > 0.0)
      return (Duration)(1000.0 * secs);

    return Duration::FOREVER;
  }


  FromBus::FromBus(const std::string &address, const std::string &options, const std::string &broker)
  :
    connection(broker),
    DiffNumAck(0)
  {
     try {
            connection.open();
            session = connection.createSession();
            Address addr(address+options);
            receiver = session.createReceiver(addr);
     } catch (const std::exception& error) {
        std::cout << error.what() << std::endl;
        connection.close();
    }
  }
 
  FromBus::~FromBus(void)
  {
    if (DiffNumAck) { std::cout << "Queue " << queuename << " on broker " << brokername << " has " << DiffNumAck << " messages not ack'ed " << std::endl;};
    connection.close();
  }

  bool FromBus::getString( std::string & Str, double timeout) // timeout 0.0 means blocking
  {
        Message incoming;
        bool ret = receiver.fetch(incoming,TimeOutSecs(timeout));
        if (ret) {
            DiffNumAck ++;
            Str= incoming.getContent();
        }
        return ret;
  }

  bool FromBus::getMessage(Message & msg, double timeout) // timeout 0.0 means blocking
  {
        bool ret= receiver.fetch(msg,TimeOutSecs(timeout));
        if (ret) DiffNumAck++;
        return ret;
  }

  void FromBus::ack(void)
  {
     session.acknowledge();
     DiffNumAck --;
  }
  
 ToBus::ToBus(const std::string &address, const std::string &options, const std::string &broker) 
  :
    connection(broker),
    DiffNumAck(0)
  {
     queuename = string(address);
     brokername = string( broker);
     state = 0;
     try {
       connection.open();
       state |= S_OPEN;
       session = connection.createSession();
       state |= S_SESSION;
       Address addr(address+options);
       sender = session.createSender(addr);
       state |= S_SENDER;
     } catch (const std::exception& error) {
       std::cout << error.what() << std::endl;
    }
  }

  void ToBus::send(const std::string &msg)
  {
    Message tosend(msg);
    sender.send(tosend,true);
    DiffNumAck ++;
  }

  ToBus::~ToBus(void)
  {
  }

  MultiBus::MultiBus(MsgHandler handler, const std::string &address, const std::string &options, const std::string &broker) 
  : 
    connection(broker),
    DiffNumAck(0)
  {
     string queuename = string(address);
     brokername = string(broker);
     state = 0;
     try {
       connection.open();
       state |= S_OPEN;
       session = connection.createSession();
       state |= S_SESSION;
       Address addr(address+options);
       Receiver receiver = session.createReceiver(addr);
       receiver.setCapacity(1);
       MsgWorker *worker=new MsgWorker;
       worker->handler=handler;
       worker->queuename=queuename;
       handlers[receiver]=worker;
       state |= S_RECEIVER;
     } catch (const std::exception& error) {
       std::cout << error.what() << std::endl;
    }
  }

  void MultiBus::add(MsgHandler handler, const std::string &address, const std::string &options)
  {
    Address addr(address+options);
    Receiver receiver = session.createReceiver(addr);
    receiver.setCapacity(1);
    MsgWorker * worker = new MsgWorker;
    worker->handler=handler;
    worker->queuename=string(address);
    handlers[receiver]=worker;
    state |= S_RECEIVER;
  }

  void MultiBus::handleMessages(void)
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

  bool MultiBus::getMessage(Message &msg, double timeout)
  {
    Receiver nrec = session.nextReceiver();
    return nrec.get(msg,TimeOutSecs(timeout));
  }

  bool MultiBus::getString(std::string &str, double timeout)
  {
    Message msg;
    Receiver nrec = session.nextReceiver();
    bool ret = nrec.get(msg,TimeOutSecs(timeout));

    if (ret) 
      str = msg.getContent();
    return ret;
  }

  MultiBus::~MultiBus()
  {
    // fixme: memory leak for workers. Also infinite loop needs a fix.
  }

} // namespace LOFAR

