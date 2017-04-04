#!/bin/csh

foreach f (*.cc)
 echo 'Compiling '$f' to ../'$f:r
 g++ -g -o ../$f:r $f
end
