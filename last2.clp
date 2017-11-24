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

(construct-lastN outputs/qnums11 
                 2)
(construct-lastN outputs/qnums12 
                 2)
(construct-lastN outputs/qnums13 
                 2)
(construct-lastN outputs/qnums14 
                 2)
(construct-lastN outputs/qnums15 
                 2)
(construct-lastN outputs/qnums16 
                 2)
(construct-lastN outputs/qnums17 
                 2)

(construct-lastN outputs/qnums11 
                 3)
(construct-lastN outputs/qnums12 
                 3)
(construct-lastN outputs/qnums13 
                 3)
(construct-lastN outputs/qnums14 
                 3)
(construct-lastN outputs/qnums15 
                 3)
(construct-lastN outputs/qnums16 
                 3)
(construct-lastN outputs/qnums17 
                 3)
