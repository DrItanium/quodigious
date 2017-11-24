(deffacts unique-numbers
          (unique-numbers 224
                          232
                          248
                          264
                          272
                          288
                          296
                          328
                          336
                          344
                          368
                          376
                          384
                          392
                          424
                          432
                          448
                          464
                          472
                          488
                          496
                          624
                          632
                          648
                          664
                          672
                          688
                          696
                          728
                          736
                          744
                          768
                          776
                          784
                          792
                          824
                          832
                          848
                          864
                          872
                          888
                          896
                          928
                          936
                          944
                          968
                          976
                          984
                          992))
(deftemplate analyzed-number
             (slot original-value
                   (default ?NONE))
             (multislot digits))
(defrule convert-unique-number
         ?f <- (unique-numbers ?number
                               $?rest)
         =>
         (retract ?f)
         (assert (analyzed-number (original-value ?number)
                                  (digits (mod ?number 
                                               10)
                                          (mod (div ?number
                                                    10)
                                               10)
                                          (div ?number
                                               100)))
                 (unique-numbers $?rest)))

(defrule generate-body-line
         (declare (salience 1))
         ?k <- (analyzed-number (original-value ?number)
                                (digits ?a ?b ?c))
         =>
         (retract ?k)
         (format t 
                 "innerBody<0>(stream, sum + %d + %d + %d, product * %d * %d * %d, index + %d, depth);%n"
                 ?c ?b ?a
                 ?c ?b ?a
                 ?number))

(reset)
(run)
(exit)

