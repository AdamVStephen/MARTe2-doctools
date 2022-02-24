# MARTe Demo : FileWriter-4X

**This example has intentional errors to test your MARTe2 debugging and reasoning skills.**

The X suffix on the example name is to emphasise an intentionally broken example.

The fourth FileWriter example builds on the previous demonstration of a single edge trigger
driving a double pre/post trigger acquisition to a csv file with the file contents being
overwritten and updated on each trigger.

The [FileWriter-3](../FileWriter-3/README.md) example ended with recommended exercises to study the
behaviour.  The eagle-eyed will have spotted that not all of the expected data is seen when the file updates.

This example was developed to try and probe this issue, which revolves around how streams are flushed.
The [FileWriter documentation](https://vcis-jenkins.f4e.europa.eu/job/MARTe2-Components-docs-master/doxygen/classMARTe_1_1FileWriter.html) documents that the DataSourceI interface implements `FlushFile` as an RPC (message interface).

The example *is supposed to* work as follows

1. Time in microseconds comes from a standard LinuxTimer data source.
2. The thread is paced -t 200Hz.
3. A trigger signal is computed as `(t modulo interval) = 0`
4. A sine waveform is evaluated based on the current time.
5. The sine value is displayed on the console via the LoggerDataSource (unconditionally).
6. The sine value is sent to the SignalsWriter.
7. The SignalsWriter is configured with pre/post trigger buffers holding 2 samples each.
8. While the trigger is asserted, the sine values are recorded to file.

This is identical to FileWriter-3 but the difference is in how the FileWriter is configured.

Since the purpose of this exercise is to practice dealing with issues, there is no automated helper script
since it is important to look at the MARTe log output to diagnose what happens.

## Interactive Format

Start the docker and set up the environment to launch MARTe.

```
docker run -it -p 8084:8084 avstephen/marte2-demos-sigtools-ubuntu1804:ayr
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# source setenv.sh
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# source m2-completion.bash
```
Two terminals are required to see the behaviour matches expectations.

In the first terminal, set up to watch for the changing contents of a file, which by default is
to be created in the MARTe2-demos-sigtools/temp directory.

```
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# cd ../MARTe2-demos-sigtools/temp
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/MARTe2-demos-sigtools/temp# rm -f *.csv
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/MARTe2-demos-sigtools/temp# watch -n 1 cat *.csv
```

In the other terminal, run the example

```
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# ./m2 STapp-0007-FileWriter-4X.cfg -s Run
```

Note that on the console, after the initial flurry of setup messages, the DisplayGAM output will show the time (updating in increments of 5000us = 5ms consistent with running at 200Hz).  It also shows the value of the trigger signal (comparison with 0 modulo interval)

Wait until you see the trigger signal passing 1 (this can take more than one second, because startup of the MARTe application takes a few seconds). The watch command running `cat *.csv` every second should show the snapshot data updating every second.

## Mandatory Exercises

If you get stuck, try the following hints to work out what is wrong.

1. [Getting the example to run at all](Hint-1.md)
1. [Understanding the RPC mechanism](Hint-2.md)
1. [How to access the RPC mechanism](Hint-3.md)
