# MARTe Demos Sigtools

There are two sets of example MARTe2 applications building towards an end to
end demonstration of standard signal processing tasks.  This top level documentation
describes what each example shows and how to run and exploit them.

The following tests use docker image avstephen/marte2-demos-sigtools-ubuntu1804:ayr

To run one example from scratch, use the following example

```
docker run -it -p 8084:8084 avstephen/marte2-demos-sigtools-ubuntu1804:ayr
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# source setenv.sh
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# export PS1="# "
root@abcedf:# source m2-completion.bash
root@abcedf:# ./m2 STapp-0007-FileWriter-1.cfg -m StateMachine:START
```

The expected output will begin as follows 
```
[Debug - Bootstrap.cpp:79]: Arguments:
...
...
[Warning - Threads.cpp:175]: Failed to change the thread priority (likely due to insufficient permissions)
[Information - StateMachine.cpp:340]: In state (INITIAL) triggered message (StartNextStateExecutionMsg)
[FatalError - Waveform.cpp:429]: GAMRef0::Input time is not increasing
[Warning - GAMSchedulerI.cpp:431]: ExecutableI GAMRef0 failed
```

Ignore the repeated errors.  Time is not increasing because we have not yet started the Run thread.  
We do this via the web interface. Open a browser pointing at [http://localhost:8084](http://localhost:8084/) 
and you should be able to do this as per the earlier training.

The documentation links below give more detailed instructions about
what each example does and how to put each through its paces.


## FileWriter Examples

1. [FileWriter-1](FileWriter-1/README.md) with a demo video on [YouTube](https://www.youtube.com/watch?v=-GuBPVnREkc) 
1. [FileWriter-2](FileWriter-2/README.md)
1. [FileWriter-3](FileWriter-3/README.md)
1. [FileWriter-4X](FileWriter-4X/README.md) - beware 4X - this is a set of intentional dead ends as a pedagogical technique.

## Waveform Examples
