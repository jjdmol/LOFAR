#ifndef WORKERPROXY_H_HEADER_INCLUDED_C0C42DDA
#define WORKERPROXY_H_HEADER_INCLUDED_C0C42DDA
class BlackBoard;

//##ModelId=3F3BAFC30157
//##Documentation
//## Polls for work to do in the database.(pull, push is more desirable. Can be
//## done when the database supports action paradigm/trigger) It has an
//## abstract function, e.g. "perform" that starts applying the real knowledge.
class WorkerProxy
{
  public:
    //##ModelId=3F3BAFC3015A
    BlackBoard *blackboard;

};



#endif /* WORKERPROXY_H_HEADER_INCLUDED_C0C42DDA */
