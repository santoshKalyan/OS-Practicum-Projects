Team members:
        Name: Santoshkalyan Rayadhurgam 
        Name: Atheendra P Tarun 

Project 3: Unreliable Datagram Networking
Overview
Preemptive threads, which you implemented in the previous project, provide a basis for more sophisticated features to be added to minithreads. Your goal for this project is to add unreliable networking on top of the preemptive threads package: this makes it possible for one copy of the minithreads package to send a message to another minithreads instance running on a different computer.

We have provided you with a simple "raw" interface to the network, which behaves like IP: it allows a thread running in a minithreads package on machine A to send a packet to the minithreads package running on machine B. When the packet arrives, it raises a network interrupt. This interface is not very convenient for application programming, since it does not allow the sender to control which thread at machine B will receive the message!

To achieve the goal of this project, you have to implement a "minimsg" layer on top of the raw network interface, which implements "miniports". Each instance of the minithreads system maintains a collection of local miniports. Miniports serve as destination and source identifiers, uniquely identifying the connection (and the set of threads) to which the packet needs to be delivered. Senders name not only the destination machine, say B, but also the port number, say X, to which their packet should be delivered. When the packet arrives at B, it is queued at the appropriate miniport, waiting to be received.

You will also have to construct a header that precedes the data being sent. The header packs in information that your minimsg layer uses to identify communication endpoints and must thus be standard in order for other minithreads implementations to meaningfully communicate with one another. It also has to be in an architecture-independent format so that hardware with different internal representations of integers can correctly interpret your data.

This assignment closely follows the UDP/IP analogy. The raw interface we provide is equivalent to IP. The miniport and packet send and receive operations you write are equivalent to UDP.

The interface you need to implement is in minimsg.h. We have provided you with skeletal routines to start off with. Note that you will need to closely interface with the network.h interface in order to start up the pseudo-network device and the IP-layer, to send messages and to receive them. Below, we first describe what we provide you with in terms of a network device and how you need to interact with it, (as with real networking cards, there are some elaborate constraints on when the card can be initialized), then what you need to implement in the minimsgs interface, and finally describe the miniport usage conventions you need to follow to write applications.

If written correctly, you can actually communicate with your friends' (and the TAs') instances of minithreads!
