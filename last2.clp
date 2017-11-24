(deffunction get-lastN
             (?router ?size)
             (bind ?result
                   (readline ?router))
             (if (eq ?result
                     EOF) then
               FALSE
               else
               (sub-string (- (str-length ?result)
                              (- ?size
                                 1))
                           (str-length ?result)
                           ?result)))
(deffunction process-lastN
             (?input ?output ?size)
             (while (bind ?result
                          (get-lastN ?input
                                     ?size)) do
                    (printout ?output
                              ?result
                              crlf)))
(deffunction construct-lastN
             (?path ?offset)
             (bind ?input
                   (gensym*))
             (open ?path
                   ?input
                   "r")
             (bind ?output
                   (gensym*))
             (open (str-cat ?path
                            _last 
                            ?offset)
                   ?output
                   "w")
             (process-lastN ?input
                            ?output
                            ?offset)
             (close ?input)
             (close ?output))
(loop-for-count (?i 2 9) do
                (loop-for-count (?w 11 17) do
                                (construct-lastN (sym-cat outputs/qnums 
                                                          ?w)
                                                 ?i)))
