# ccard-lib

## Description

A C library for estimating cardinality in streams for 
which it is infeasible to store all events in memory.

This library implements a series of cardinality estimate algorithms 
such as Linear Counting, LogLog Counting, HyperLogLog Counting and Adaptive Counting.
For more information of this algorithms please read the [Reference](#reference) section.

## Building

(INCOMPLETE)

## Examples

(INCOMPLETE)

## Reference

### LogLog Counting and Adaptive Counting

 * Min Cai, Jianping Pan, Yu K. Kwok, and Kai Hwang. Fast and accurate
 traffic matrix measurement using adaptive cardinality counting. In
 MineNet ’05: Proceedings of the 2005 ACM SIGCOMM workshop on
 Mining network data, pages 205–206, New York, NY, USA, 2005. ACM.

 * Marianne Durand and Philippe Flajolet. LogLog counting of large
 cardinalities. In ESA03, volume 2832 of LNCS, pages 605–617, 2003.

### Linear Counting

 * K.-Y. Whang, B. T. Vander-Zanden, and H. M. Taylor. A Linear-Time 
 Probabilistic Counting Algorithm for Database Applications. ACM 
 Transactions on Database Systems, 15(2):208-229, 1990.

### HyperLogLog Counting

 * P. Flajolet, E. Fusy, O. Gandouet, and F. Meunier.
 Hyperloglog: The analysis of a near-optimal cardinality
 estimation algorithm. Disc. Math. and Theor. Comp. Sci., AH:127-146, 2007.

The implemention refers [stream-lib](https://raw.github.com/clearspring/stream-lib).

