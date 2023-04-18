simple TicToc
{
    gates:
        input in;
        output out;
}

//
// Two instances (tic and toc) of Txc1 connected both ways.
// Tic and toc will pass messages to one another.
//
network TicTocNetwork
{
    submodules:
        tic: TicToc;
        toc: TicToc;
    connections:
        tic.out --> toc.in;
        tic.in <-- toc.out;
        
}