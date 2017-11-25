; Performs execution specific compilation and invocation

(deffunction build-program
	     (?prefix ?width ?sum ?product ?index)
	     (system (format nil
			     "g++ -lpthread -DSUM=%d -DPRODUCT=%d -DINDEX=%d -O3 -flto -fwhole-program -std=c++14 quodigious_compilation_target.cc -o tmp/quodigious%d_%d"
			     ?sum
			     ?product
			     ?index
			     ?width
			     ?index))
	     (system (format nil
			     "tmp/quodigious%d_%d > %s/q%d_%d &"
			     ?width
			     ?index
			     ?prefix
			     ?width
			     ?index)))

(deffunction body
	     (?prefix ?owidth ?width ?os ?op ?ok)
	     (if (= ?width 
		    12) then
	       (build-program ?prefix
			      ?owidth
			      ?os
			      ?op
			      ?ok)
	       else
	       (loop-for-count (?x 2 9) do
			       (if (<> ?x 5) then
				 (body ?prefix
				       ?owidth
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
	     (?prefix ?width)
	     (body ?prefix
		   ?width
		   ?width
		   0
		   1
		   0))

;(do-it 19)
;(loop-for-count (?i 13 19) do
;                (do-it ?i))
;(exit)
