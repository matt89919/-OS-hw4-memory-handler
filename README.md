## hw4-Memory-Manager

# LRU,FIFO,global
Process A, Effective Access Time = 164.758
Process A, Page Fault Rate = 0.723
Process B, Effective Access Time = 163.709
Process B, Page Fault Rate = 0.665

# Random,FIFO,global
Process A, Effective Access Time = 164.309
Process A, Page Fault Rate = 0.723
Process B, Effective Access Time = 163.144
Process B, Page Fault Rate = 0.665


# Random,Clock,gloabl
Process A, Effective Access Time = 163.621
Process A, Page Fault Rate = 0.759
Process B, Effective Access Time = 162.953
Process B, Page Fault Rate = 0.741

# LRU,Clock,global
Process A, Effective Access Time = 164.758
Process A, Page Fault Rate = 0.796
Process B, Effective Access Time = 163.522
Process B, Page Fault Rate = 0.747

# LRU,Clock,local
Process A, Effective Access Time = 164.980
Process A, Page Fault Rate = 0.774
Process B, Effective Access Time = 163.522
Process B, Page Fault Rate = 0.753

# Random,Clock,local
Process A, Effective Access Time = 164.980
Process A, Page Fault Rate = 0.774
Process B, Effective Access Time = 162.953
Process B, Page Fault Rate = 0.741

# Random,FIFO,local
Process A, Effective Access Time = 164.980
Process A, Page Fault Rate = 0.774
Process B, Effective Access Time = 162.761
Process B, Page Fault Rate = 0.700

# LRU,FIFO,local
Process A, Effective Access Time = 164.980
Process A, Page Fault Rate = 0.774
Process B, Effective Access Time = 163.144
Process B, Page Fault Rate = 0.700

The second chance algo have a better page fault rate than FIFO when using global repacement, because its is a improved version of FIFO and LRU.
However, when using local replacement, the page fault rate is sometime larger when the reference of program is not that uniform.

For TLB replacement, I think that LRU may have a better EAT and page fault rate than Random if the testcase is uniform (also if we test many times) enough, because the Random algorithm is not a static method, so the result may be different everytime.

For gloabal or local replacement, i think it should base on the frame size and the process reference. If assume its uniform, i think global will be better because it has more page frame can be selected.
