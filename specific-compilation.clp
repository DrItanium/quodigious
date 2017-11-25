; Performs execution specific compilation and invocation

(deffunction build-and-execute-program
             (?width ?sum ?product ?index)
             (system (format nil
                             "gcc -DSUM=%d -DPRODUCT=%d -DINDEX=%d -O3 -flto -fwhole-program -std=c++14 quodigious_compilation_target.cc -o quodigious%d_%d_%d_%d"
                             ?sum
                             ?product
                             ?index
                             ?width
                             ?sum
                             ?product
                             ?index)))


