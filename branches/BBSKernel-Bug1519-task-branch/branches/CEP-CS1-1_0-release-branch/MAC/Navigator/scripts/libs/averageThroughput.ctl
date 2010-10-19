float averageThroughput(
  int    bytes, // The new value
  string dp     // The dp where we do something
)
{
  time timeNew;
  int  bytesOld;
  time timeOld;
  float throughput;

  dpGet(dp + ":_original.._stime",timeNew);
  dpGet(dp + "Old",bytesOld);
  dpGet(dp + "Old:_original.._stime",timeOld);
  // write the new values
  dpSetTimed(timeNew, dp + "Old",bytes);
  DebugTN( "averageThroughput(): bytesOld,timeOld,bytesNew,timeNew",bytesOld,timeOld,bytes,timeNew );
  
  int diffBytes = bytes - bytesOld;
  float diffTime = timeNew - timeOld;
  throughput = diffBytes / diffTime;
  DebugTN( "averageThroughput(): diffBytes,diffTime,throughput:",diffBytes,diffTime,throughput );

  return throughput;
}
