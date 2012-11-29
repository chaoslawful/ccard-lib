## Description

C library for estimating cardinality in streams for 
which it is infeasible to store all events in memory.

This library implements a series of cardinality estimate algorithms 
such as Linear Counting, LogLog Counting, HyperLogLog Counting and Adaptive Counting.
For more information about this algorithms please read the [Reference](#reference) section.

## Building

Building ccard-lib needs the [scons](http://www.scons.org/). Please read document 
at [scons homepage](http://www.scons.org/) for more information about it.

Assuming you have scons installed, just build ccard-lib like this:

    scons

All will be done.

## Examples

Please see test cases in [t/](https://github.com/chaoslawful/ccard-lib/tree/master/t) 
to know how to use ccard-lib. 

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

## Experiment

When using 2^16 (64k) bitmap, estimate result as follow:

	Linear Counting with Murmurhash:
	actual: 50000, estimated: 50062, error: 0.12%
	actual: 100000, estimated: 99924, error: 0.08%
	actual: 150000, estimated: 149865, error: 0.09%
	actual: 200000, estimated: 199916, error: 0.04%
	actual: 250000, estimated: 250123, error: 0.05%
	actual: 300000, estimated: 299942, error: 0.02%
	actual: 350000, estimated: 349801, error: 0.06%
	actual: 400000, estimated: 400101, error: 0.03%
	actual: 450000, estimated: 449955, error: 0.01%
	actual: 500000, estimated: 500065, error: 0.01%

	Linear Counting with Lookup3hash:
	actual: 50000, estimated: 49835, error: 0.33%
	actual: 100000, estimated: 99461, error: 0.54%
	actual: 150000, estimated: 149006, error: 0.66%
	actual: 200000, estimated: 198501, error: 0.75%
	actual: 250000, estimated: 248365, error: 0.65%
	actual: 300000, estimated: 298065, error: 0.65%
	actual: 350000, estimated: 347504, error: 0.71%
	actual: 400000, estimated: 397292, error: 0.68%
	actual: 450000, estimated: 446700, error: 0.73%
	actual: 500000, estimated: 495944, error: 0.81%

	Hyperloglog Counting with Murmurhash:
	actual: 50000, estimated: 50015, error: 0.03%
	actual: 100000, estimated: 100048, error: 0.05%
	actual: 150000, estimated: 149709, error: 0.19%
	actual: 200000, estimated: 201595, error: 0.80%
	actual: 250000, estimated: 250168, error: 0.07%
	actual: 300000, estimated: 299864, error: 0.05%
	actual: 350000, estimated: 348571, error: 0.41%
	actual: 400000, estimated: 398583, error: 0.35%
	actual: 450000, estimated: 448632, error: 0.30%
	actual: 500000, estimated: 498330, error: 0.33%

	Hyperloglog Counting with Lookup3hash:
	actual: 50000, estimated: 49628, error: 0.74%
	actual: 100000, estimated: 99357, error: 0.64%
	actual: 150000, estimated: 148880, error: 0.75%
	actual: 200000, estimated: 200475, error: 0.24%
	actual: 250000, estimated: 249362, error: 0.26%
	actual: 300000, estimated: 299119, error: 0.29%
	actual: 350000, estimated: 349225, error: 0.22%
	actual: 400000, estimated: 398805, error: 0.30%
	actual: 450000, estimated: 448373, error: 0.36%
	actual: 500000, estimated: 498183, error: 0.36%

	Adaptive Counting with Murmurhash:
	actual: 50000, estimated: 50015, error: 0.03%
	actual: 100000, estimated: 100048, error: 0.05%
	actual: 150000, estimated: 149709, error: 0.19%
	actual: 200000, estimated: 201059, error: 0.53%
	actual: 250000, estimated: 249991, error: 0.00%
	actual: 300000, estimated: 300067, error: 0.02%
	actual: 350000, estimated: 349610, error: 0.11%
	actual: 400000, estimated: 399875, error: 0.03%
	actual: 450000, estimated: 450348, error: 0.08%
	actual: 500000, estimated: 500977, error: 0.20%

	Adaptive Counting with Lookup3hash:
	actual: 50000, estimated: 49628, error: 0.74%
	actual: 100000, estimated: 99357, error: 0.64%
	actual: 150000, estimated: 148880, error: 0.75%
	actual: 200000, estimated: 199895, error: 0.05%
	actual: 250000, estimated: 249563, error: 0.17%
	actual: 300000, estimated: 299047, error: 0.32%
	actual: 350000, estimated: 348665, error: 0.38%
	actual: 400000, estimated: 399266, error: 0.18%
	actual: 450000, estimated: 450196, error: 0.04%
	actual: 500000, estimated: 499516, error: 0.10%

	Loglog Counting with Murmurhash:
	actual: 50000, estimated: 59857, error: 19.71%
	actual: 100000, estimated: 103108, error: 3.11%
	actual: 150000, estimated: 150917, error: 0.61%
	actual: 200000, estimated: 201059, error: 0.53%
	actual: 250000, estimated: 249991, error: 0.00%
	actual: 300000, estimated: 300067, error: 0.02%
	actual: 350000, estimated: 349610, error: 0.11%
	actual: 400000, estimated: 399875, error: 0.03%
	actual: 450000, estimated: 450348, error: 0.08%
	actual: 500000, estimated: 500977, error: 0.20%

	Loglog Counting with Lookup3hash:
	actual: 50000, estimated: 59870, error: 19.74%
	actual: 100000, estimated: 103044, error: 3.04%
	actual: 150000, estimated: 150435, error: 0.29%
	actual: 200000, estimated: 199895, error: 0.05%
	actual: 250000, estimated: 249563, error: 0.17%
	actual: 300000, estimated: 299047, error: 0.32%
	actual: 350000, estimated: 348665, error: 0.38%
	actual: 400000, estimated: 399266, error: 0.18%
	actual: 450000, estimated: 450196, error: 0.04%
	actual: 500000, estimated: 499516, error: 0.10%

