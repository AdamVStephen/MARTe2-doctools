# MARTe Demo : FileWriter-4X

## Hint-2 : Getting the example to Run Properly

Congratulations.  If you got this far, you found the syntax error in the FileWriter class name.

So - it runs. That don't impress me much. Does it also exhibit the desired behaviour ?

## Understand the Objective

Having fixed the syntax, we now address the semantics.   The objective of the [FileWriter-3](../FileWriter-3/README.md) demo
was to show pre/post buffer samples being written to file.  However, for reasons suspected to be to do with file flushing,
not all of the data is seen.

## Figure out Technical Options

Once the root cause (stream flushing) has been identified, the question is whether the problem can be cured via

1. System configuration
1. Component configuration
1. Component modification

In this case, when a stream flushes is probably not fully under our control.  In contrast, some tuning of TCP/IP network parameters,
or some behaviours about thread/process prioritisation might be resolved by OS tuning.

The component configuration explicitly mentions file flushing as an option, so this sounds promising.

It is generally best to avoid jumping in to changing a component, unless it is a new component under development.  Rather look at the provided test suite to better understand what features of the component have been proven first.  And then, contact some experts ideally including the component author, before hacking a local fix.   Unless your project can't wait, of course.

## Resolving Problem 2

Think about this for long enough to have either learned something about FileWriter/MARTe, or to need to progress to the solution
which can be found in [Hint-3](Hint-3.md).

