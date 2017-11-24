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
(deffunction get-last2
             (?router)
             (get-lastN ?router
                        2))
(deffunction get-last3
             (?router)
             (get-lastN ?router
                        3))
(deffunction process-lastN
             (?input ?output ?fn)
             (while (bind ?result
                          (funcall ?fn
                                   ?input)) do
                    (printout ?output
                              ?result
                              crlf)))
(deffunction construct-lastN
             (?path ?fn)
             (bind ?input
                   (gensym*))
             (open ?path
                   ?input
                   "r")
             (bind ?output
                   (gensym*))
             (open (str-cat ?path
                            _last2)
                   ?output
                   "w")
             (process-lastN ?input
                            ?output
                            ?fn)
             (close ?input)
             (close ?output))

(construct-lastN outputs/qnums11 
                 get-last2)
(construct-lastN outputs/qnums12 
                 get-last2)
(construct-lastN outputs/qnums13 
                 get-last2)
(construct-lastN outputs/qnums14 
                 get-last2)
(construct-lastN outputs/qnums15 
                 get-last2)
(construct-lastN outputs/qnums16 
                 get-last2)
(construct-lastN outputs/qnums17 
                 get-last2)

(construct-lastN outputs/qnums11 
                 get-last3)
(construct-lastN outputs/qnums12 
                 get-last3)
(construct-lastN outputs/qnums13 
                 get-last3)
(construct-lastN outputs/qnums14 
                 get-last3)
(construct-lastN outputs/qnums15 
                 get-last3)
(construct-lastN outputs/qnums16 
                 get-last3)
(construct-lastN outputs/qnums17 
                 get-last3)
