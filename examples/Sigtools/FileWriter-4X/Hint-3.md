# MARTe Demo : FileWriter-4X

## Hint-3 : Getting the example to Run Properly

Congratulations.  If you got this far, you hopefully learned something about FileWriter/MARTe.

## Know When To Ask for Assistance

Getting FileWriter to run the flush file operation via the RPC message interface is clearly the intent
of the author of example 4X.

However, they got stuck at this point, because knowing how to cause this to happen was not clear.

Looking at the commented out lines at the top of the file shows some intent of instantiating a TCP intermediary.
That's a partial clue.

## Find Similar Patterns

The solution is to consider previous training exposure to the message concept.  There is a hook in the startup command
which allows a message to be sent via the "-m" option.  There are also examples of accessing the HTTP interface of the
StateMachine and then getting the WebServer component to send messages to the StateMachine.

So - we need to create an interface component which is preconfigured to send file flush requests to the SignalsWriter based
on external cues from a suitable client application (HTTP request or EPICS Channel Access put request).

To find out how this works, move on to the [next example](../FileWriter-5/README.md)
