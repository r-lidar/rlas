This is an early release because compilation failed on CRAN with osx and solaris (segfault)

There was indeed a segfault in the code that, for an unknown reason, was not a segfault in any
other platform. I was not able to reproduce the issue whatever the compiler I used. The only 
way I reproduced the segfault was compiling the package via the r-hub.io osx builder.

I fixed the bug and I added several extra tests. The bug is fully fixed on osx according to
osx builder on r-hub.io. However it does not compile for solaris on r-hub.io but the config 
seems different than the one on the CRAN. Previous release compiled properly on solaris. So 
I'm confident and I'm expecting that everything should go fine since the code did not change.

The package should be compiled on solaris and osx first to figure out if everything is good 
on those two platforms.

## Test environments
* Linux Mint 18 (Ubuntu 16.04), R 3.4.4, g++ -std=gnu++11
* Linux Mint 18 (Ubuntu 16.04), R 3.4.4, g++-7 -std=gnu++11
* osx release via r-hub.io builder
* windows via CRAN builder

## R CMD check results
There were no ERRORs or WARNINGs.

There was between 0 or 1 NOTE depending on platforms:

* checking CRAN incoming feasibility ... NOTE

Identical with the previous release

* checking installed package size ... NOTE

Identical with the previous release

## Downstream dependencies
There is currently 1 downstream dependency for this package.

* `lidR` no problem reported with `devtools::revdep_check()`

