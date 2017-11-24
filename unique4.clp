(deffacts numbers
          (unique-numbers 2224
                          2272
                          2288
                          2336
                          2368
                          2384
                          2432
                          2448
                          2464
                          2496
                          2624
                          2672
                          2688
                          2736
                          2768
                          2784
                          2832
                          2848
                          2864
                          2896
                          2928
                          2944
                          2976
                          2992
                          3232
                          3248
                          3264
                          3296
                          3328
                          3344
                          3376
                          3392
                          3424
                          3472
                          3488
                          3632
                          3648
                          3664
                          3696
                          3728
                          3744
                          3776
                          3792
                          3824
                          3872
                          3888
                          3936
                          3968
                          3984
                          4224
                          4272
                          4288
                          4336
                          4368
                          4384
                          4432
                          4448
                          4464
                          4496
                          4624
                          4672
                          4688
                          4736
                          4768
                          4784
                          4832
                          4848
                          4864
                          4896
                          4928
                          4944
                          4976
                          4992
                          6224
                          6272
                          6288
                          6336
                          6368
                          6384
                          6432
                          6448
                          6464
                          6496
                          6624
                          6672
                          6688
                          6736
                          6768
                          6784
                          6832
                          6848
                          6864
                          6896
                          6928
                          6944
                          6976
                          6992
                          7232
                          7248
                          7264
                          7296
                          7328
                          7344
                          7376
                          7392
                          7424
                          7472
                          7488
                          7632
                          7648
                          7664
                          7696
                          7728
                          7744
                          7776
                          7792
                          7824
                          7872
                          7888
                          7936
                          7968
                          7984
                          8224
                          8272
                          8288
                          8336
                          8368
                          8384
                          8432
                          8448
                          8464
                          8496
                          8624
                          8672
                          8688
                          8736
                          8768
                          8784
                          8832
                          8848
                          8864
                          8896
                          8928
                          8944
                          8976
                          8992
                          9232
                          9248
                          9264
                          9296
                          9328
                          9344
                          9376
                          9392
                          9424
                          9472
                          9488
                          9632
                          9648
                          9664
                          9696
                          9728
                          9744
                          9776
                          9792
                          9824
                          9872
                          9888
                          9936
                          9968
                          9984))
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
                                          (mod (div ?number
                                                    100)
                                               10)
                                          (div ?number 
                                               1000)))
                 (unique-numbers $?rest)))

(defrule generate-body-line
         (declare (salience 1))
         ?k <- (analyzed-number (original-value ?number)
                                (digits ?a ?b ?c ?d))
         =>
         (retract ?k)
         (format t 
                 "innerBody<0>(stream, sum + %d + %d + %d + %d, product * %d * %d * %d * %d, index + %d, depth);%n"
                 ?d ?c ?b ?a
                 ?d ?c ?b ?a
                 ?number))

(reset)
(run)
(exit)

