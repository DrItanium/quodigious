(deffunction grab-1000
	     (?in ?out)
	     (loop-for-count (?k 1 1000) do
			     (bind ?v
				   (readline ?in))
			     (if (neq ?v 
				      EOF) then
			       (printout ?out 
					 ?v crlf)
			       else
			       (return FALSE)))
	     (return TRUE))

(deffunction split-up-files
	     (?prefix ?in)
	     (bind ?i
		   0)
	     (while TRUE do
		    (bind ?k 
			  (gensym*))
		    (open (str-cat ?prefix 
				   _ ?i)
			  ?k
			  "w")
		    (if (not (grab-1000 ?in
				   ?k)) then
		      (close ?k)
		      (return TRUE))
		    (close ?k)
		    (bind ?i
			  (+ ?i 1))))
				   
(deffunction divide-files
	     (?path ?prefix)
	     (bind ?n
		   (gensym*))
	     (open ?path
		   ?n
		   "r")
	     (split-up-files (str-cat tmp/ 
				      ?prefix)
			     ?n)
	     (close ?n))

(divide-files /srv/repo/data/i19_3
	      i19_3)
(divide-files /srv/repo/data/i19_4
	      i19_4)
(divide-files /srv/repo/data/i19_6
	      i19_6)

	     

