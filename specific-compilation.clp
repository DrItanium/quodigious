; Performs execution specific compilation and invocation

(deffunction build-program
             (?width ?sum ?product ?index)
             (system (format nil
                             "g++ -lpthread -DSUM=%d -DPRODUCT=%d -DINDEX=%d -O3 -flto -fwhole-program -std=c++14 quodigious_compilation_target.cc -o quodigious%d_%d_%d_%d"
                             ?sum
                             ?product
                             ?index
                             ?width
                             ?sum
                             ?product
                             ?index)))


(loop-for-count (?i 2 9) do
                (if (<> ?i 5) then
                    (build-program 14
                                   ?i
                                   ?i
                                   (integer (* ?i (** 10 12))))))
