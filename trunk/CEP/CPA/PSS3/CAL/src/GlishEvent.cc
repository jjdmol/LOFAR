#include <tasking/Glish.h>
#include <tasking/Glish/GlishEvent.h>

#include <sys/time.h>
#include <signal.h>

static GlishSysEventSource *expire_s = 0;

void timer_expired(int)
{
  if (expire_s) expire_s->postEvent("timeout", "");
}

Bool eventhandler(GlishSysEvent &evt)
{
  //GlishValue &val = evt.val();
  GlishSysEventSource *s = (GlishSysEventSource *) evt.source();
  
  expire_s = s;

  struct itimerval period;

  if (evt.type() == "start")
  {
    signal(SIGALRM, timer_expired);

    period.it_interval.tv_sec  = 1;
    period.it_interval.tv_usec = 0;
    period.it_value.tv_sec  = 1;
    period.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &period, NULL);
  }
  else if (evt.type() == "stop")
  {
    signal(SIGALRM, NULL);

    period.it_interval.tv_sec  = 0;
    period.it_interval.tv_usec = 0;
    period.it_value.tv_sec  = 0;
    period.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &period, NULL);
  }

#if 0
  String mod("MOD");
  String sval = val.format();

  mod += evt.type();
  
  s->postEvent(mod,sval);
#endif

  return True;
}

int main(int argc, char **argv)
{
  GlishSysEventSource gsrc(argc,argv);
  GlishSysEventTarget gtgt(eventhandler);
  SysEvent event;
  
  while (gsrc.connected()) {               // 1
    if (gsrc.waitingEvent()) {           // 2
      event = gsrc.nextEvent();        // 3
      event.dispatch(gtgt);            // 4
    }
  }
}

