; Performs execution specific compilation and invocation

(deffunction build-program
             (?width ?sum ?product ?index)
             (system (format nil
                             "g++ -lpthread -DSUM=%d -DPRODUCT=%d -DINDEX=%d -O3 -flto -fwhole-program -std=c++14 quodigious_compilation_target.cc -o /tmp/quodigious%d_%d"
                             ?sum
                             ?product
                             ?index
                             ?width
                             ?index))
             (system (format nil
                             "/tmp/quodigious%d_%d > q%d_%d &"
                             ?width
                             ?index
                             ?width
                             ?index)))

(deffunction body
             (?owidth ?width ?os ?op ?ok)
             (if (= ?width 
                    12) then
               (build-program ?owidth
                              ?os
                              ?op
                              ?ok)
               else
               (loop-for-count (?x 2 9) do
                               (if (<> ?x 5) then
                                 (body ?owidth
                                       (- ?width 
                                          1)
                                       (+ ?os
                                          ?x)
                                       (* ?op
                                          ?x)
                                       (+ ?ok
                                          (integer (* ?x
                                                      (** 10 
                                                          (- ?width
                                                             1))))))))))
(deffunction do-it
             (?width)
             (body ?width
                   ?width
                   0
                   1
                   0))

(do-it 19)
;(loop-for-count (?i 13 19) do
;                (do-it ?i))
(exit)
