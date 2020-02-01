# Implementations

## How to run?
You can just copy the files into your ns-3/scratch and ns-3/src folder respectively . 
Then run the command:  
`$./waf --run “Neat [your .ne file]” `
or
`$./waf --run “evaluation” `
### Prerequisites 

At first [**ns3**](https://www.nsnam.org/) of version 3.27 is needed. <br>
Follow the instructions given on their webpage.

To get the NEAT implementation running in the ns-3.27/scratch folder the [‘-Werror’] flag in the ns-3.27/waf-tools/cflags.py file (line 22) need to be deleted (incommented). <br>

NEAT needs two files to work. The startgenes file and the parameter (.ne) file. These files need to be at the root ns-3.27 folder. Further information about the (.ne) files can be found in the [**NEAT source project**](https://github.com/FernandoTorres/NEAT): 

After that you are ready to go.
