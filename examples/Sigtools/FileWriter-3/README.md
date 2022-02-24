# MARTe Demo : FileWriter-3

The third FileWriter example also demonstrates how to condition whether or not signal data is appended to a file
based on the current time.  As with [FileWriter-2](../FileWriter-2/README.md) the trigger is raised repeatedly
at a specified `interval` but rather than keeping the trigger high for a `record time` period, a single edge
is generated. The `SignalsWriter` is configured with two pre/post trigger buffers and the `RefreshContent` attribute
is asserted.

The effect is that on every iteration, a snapshot of 5 samples is overwritten into the file.
The default interval is set to 10 seconds.

```mermaid
gantt
    title FileWriter-3
    dateFormat  X
    axisFormat  %L
	section Acquisition 1
	Pre-trigger           : 0, 0.10
	Trigger               : 0.10, 0.15 
	Post-trigger          : 0.15, 0.25
	Delay                 : 0.25, 1.25
	section Acquisition 2
	Pre-trigger           : 1.25, 1.35
	Trigger               : 1.35, 1.40
	Post-trigger          : 1.40, 1.50
```
It works as follows

1. Time in microseconds comes from a standard LinuxTimer data source.
2. The thread is paced -t 200Hz.
3. A trigger signal is computed as `(t modulo interval) = 0`
4. A sine waveform is evaluated based on the current time.
5. The sine value is displayed on the console via the LoggerDataSource (unconditionally).
6. The sine value is sent to the SignalsWriter.
7. The SignalsWriter is configured with pre/post trigger buffers holding 2 samples each.
8. While the trigger is asserted, the sine values are recorded to file.

![Run State](sta_StateRun.png)

There are two options for running the demo.  The simple case is to start the application directly in the Run state thus:

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
root@abcedf:/opt/MARTe/Projects/MARTe2-sigtools/bin# ./m2 STapp-0007-FileWriter-3.cfg -s Run
```

Note that on the console, after the initial flurry of setup messages, the DisplayGAM output will show the time (updating in increments of 5000us = 5ms consistent with running at 200Hz).  It also shows the value of the trigger signal (comparison with 0 modulo interval)

Wait until you see the trigger signal passing 1 (this can take more than one second, because startup of the MARTe application takes a few seconds). The watch command running `cat *.csv` every second should show the snapshot data updating every second.

## Automated Format

Create a simple bash script as follows to do everything in one.

This script is available for the moment [here](https://github.com/AdamVStephen/MARTe2-sigtools/blob/ayr/bin/md-fw-3).

```
!/usr/bin/env bash
rm -f ../MARTe2-demos-sigtools/temp/*.csv
start_time=$(date --rfc-3339='ns')
./m2 STapp-0007-FileWriter-3.cfg -s Run > /tmp/log 2>&1 &
watch -n 1 cat ../MARTe2-demos-sigtools/temp/*.csv
echo "MARTe started at $start_time
```

Since this is a continuous demo, the script then has to be exited and you need to cleanup with `pkill MARTe`.

## Suggested Exercises

1. Study the output and consider whether it behaves exactly as expected.
1. Edit the interval time and the pre/post trigger buffer sizes.
1. Check that the computed value is as expected.

