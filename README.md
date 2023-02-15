# Simulations Plots
Please see the visualization folder for the plots and blockchain tree corresponding to the 4 simulations that we've done
# Compilation 
### If you're on Linux, just type `make` in the root dir  
### Otherwise, compile with the following command  
`g++ -std=c++17 ./src/*.cpp -I ./include -o simulator`

# Running the simulator

Give any of the following optional arguments to run simulator

    Usage: Blockchain-Simulator [--help] [--version] [-n VAR] [-z0 VAR] [-z1 VAR] [-Ttx VAR] [-blks VAR] [-txns VAR] [-stime VAR] [-I VAR] [--blockchain_file VAR] [--graph_file VAR] [--stats_file VAR]  

    Optional arguments:
    -h, --help                    shows help message and exits  
    -v, --version                 prints version information and exits
    -n                            number of peers in the network [default: 20]
    -z0                           fraction of "slow" peers in the network [default: 0.4]
    -z1                           fraction of "low CPU" peers in the network [default: 0.35]
    -Ttx                          mean of interarrival time between txns [default: 20]
    -blks                         max blocks in the simulation [default: 100]
    -txns                         max txns in the simulation [default: 2147483647]
    -stime                        simulation time in secs [default: 1.79769e+308]
    -I                            block interarrival time in secs [default: 10]
    -blkchain, --blockchain_file  name of file to write blockchain info to [default: "trace"]
    -graph, --graph_file          name of file to write blockchain edges to [default: "edges"]
    -stats, --stats_file          name of file to write simulation stats to [default: "stats"]
    -v, --visualize               produce visualization data

# Visualization
The simulator produces three files: a trace file which contains information about blockchain of every peer, a statistics file which shows the stats of the simulation and an edges file which can be used to generate a graph.  You can also use the file `visualize.py` to generate the plot of your simulation statistics provided you've generated the visualization data file by enabling `-v` flag, use the following command

    python visualize.py <edges_file> <data_file> <jpeg_name>
    
This also generated a file named `graph.dot` which you can use to generate a graph as follows

    dot -Tpdf graph.dot > graph.pdf

